#include "../common/common.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_PATH "/var/log/emb_mgr.log"
#define PID_PATH "/var/run/filesrv.pid"
#define INBOX_DIR "/data/inbox"

#define FILESRV_PATH "/usr/bin/filesrv"

#define FILESRV_PORT 9000

static void cgi_header(void)
{
  printf("Content-Type: application/json\r\n\r\n");
}

static int ensure_dirs(void)
{
  ensure_dir("/var");
  ensure_dir("/var/log");
  ensure_dir("/var/run");
  ensure_dir("/data");
  ensure_dir(INBOX_DIR);
  return 0;
}

static int read_pid_file(void)
{
  FILE *fp = fopen(PID_PATH, "r");
  if (!fp)
    return -1;
  int pid = -1;
  if (fscanf(fp, "%d", &pid) != 1)
    pid = -1;
  fclose(fp);
  return pid;
}

static int pid_alive(int pid)
{
  if (pid <= 1)
    return 0;
  if (kill(pid, 0) == 0)
    return 1;
  return (errno == EPERM) ? 1 : 0;
}

// 非阻塞 connect + select 检测端口是否开放（本机 127.0.0.1:9000）
static int port_open_local(int port, int timeout_ms)
{
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    return 0;

  int flags = fcntl(s, F_GETFL, 0);
  fcntl(s, F_SETFL, flags | O_NONBLOCK);

  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_port = htons((uint16_t)port);
  inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);

  int r = connect(s, (struct sockaddr *)&a, sizeof(a));
  if (r == 0)
  {
    close(s);
    return 1;
  }

  if (errno != EINPROGRESS)
  {
    close(s);
    return 0;
  }

  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(s, &wfds);

  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  r = select(s + 1, NULL, &wfds, NULL, &tv);
  if (r <= 0)
  {
    close(s);
    return 0;
  }

  int soerr = 0;
  socklen_t slen = sizeof(soerr);
  getsockopt(s, SOL_SOCKET, SO_ERROR, &soerr, &slen);
  close(s);
  return (soerr == 0) ? 1 : 0;
}

// 兜底：若 PID 文件没有，扫描 /proc 找到名为 filesrv 的进程
static int find_filesrv_pid_by_proc(void)
{
  DIR *dp = opendir("/proc");
  if (!dp)
    return -1;

  struct dirent *de;
  while ((de = readdir(dp)))
  {
    int pid = atoi(de->d_name);
    if (pid <= 1)
      continue;

    char comm_path[256];
    snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

    char comm[64] = {0};
    if (read_first_line(comm_path, comm, sizeof(comm)) == 0)
    {
      if (strcmp(comm, "filesrv") == 0)
      {
        closedir(dp);
        return pid;
      }
    }
  }
  closedir(dp);
  return -1;
}

static int start_filesrv(void)
{
  // 已经运行就不重复启动
  int pid = read_pid_file();
  if (pid_alive(pid) || port_open_local(FILESRV_PORT, 200))
    return 2; // already

  pid_t c = fork();
  if (c < 0)
    return -1;

  if (c == 0)
  {
    // child：脱离终端、重定向输出
    setsid();
    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0)
    {
      dup2(fd, 0);
      dup2(fd, 1);
      dup2(fd, 2);
      if (fd > 2)
        close(fd);
    }

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", FILESRV_PORT);

    execl(FILESRV_PATH, "filesrv", "-p", port_str, "-d", INBOX_DIR, (char *)0);
    _exit(127);
  }

  // parent：等待端口开放（最多约 1s）
  for (int i = 0; i < 10; i++)
  {
    if (port_open_local(FILESRV_PORT, 100))
      return 0;
    usleep(100 * 1000);
  }
  return -2;
}

static int stop_filesrv(void)
{
  int pid = read_pid_file();
  if (!pid_alive(pid))
  {
    // PID 文件可能丢失或陈旧，扫描 /proc
    pid = find_filesrv_pid_by_proc();
  }

  if (!pid_alive(pid) && !port_open_local(FILESRV_PORT, 100))
    return 2; // already stopped

  if (pid > 1)
    kill(pid, SIGTERM);

  // 等待退出（最多约 1s）
  for (int i = 0; i < 10; i++)
  {
    if (!pid_alive(pid) && !port_open_local(FILESRV_PORT, 100))
    {
      unlink(PID_PATH);
      return 0;
    }
    usleep(100 * 1000);
  }

  // 仍未退出则强杀
  if (pid > 1)
    kill(pid, SIGKILL);

  for (int i = 0; i < 10; i++)
  {
    if (!pid_alive(pid) && !port_open_local(FILESRV_PORT, 100))
    {
      unlink(PID_PATH);
      return 0;
    }
    usleep(100 * 1000);
  }

  return -1;
}

int main(void)
{
  cgi_header();
  ensure_dirs();

  const char *qs = getenv("QUERY_STRING");
  if (!qs)
    qs = "";

  if (strstr(qs, "action=start"))
  {
    int r = start_filesrv();
    int running = port_open_local(FILESRV_PORT, 200);
    log_line(LOG_PATH, "ctl start r=%d running=%d", r, running);

    if (r == 0)
    {
      printf("{\"ok\":true,\"msg\":\"started\",\"running\":%s}\n", running ? "true" : "false");
      return 0;
    }
    if (r == 2)
    {
      printf("{\"ok\":true,\"msg\":\"already running\",\"running\":%s}\n", running ? "true" : "false");
      return 0;
    }
    printf("{\"ok\":false,\"msg\":\"start failed\",\"running\":%s}\n", running ? "true" : "false");
    return 0;
  }

  if (strstr(qs, "action=stop"))
  {
    int r = stop_filesrv();
    int running = port_open_local(FILESRV_PORT, 200);
    log_line(LOG_PATH, "ctl stop r=%d running=%d", r, running);

    if (r == 0)
    {
      printf("{\"ok\":true,\"msg\":\"stopped\",\"running\":%s}\n", running ? "true" : "false");
      return 0;
    }
    if (r == 2)
    {
      printf("{\"ok\":true,\"msg\":\"already stopped\",\"running\":%s}\n", running ? "true" : "false");
      return 0;
    }
    printf("{\"ok\":false,\"msg\":\"stop failed\",\"running\":%s}\n", running ? "true" : "false");
    return 0;
  }

  // status
  int pid = read_pid_file();
  if (!pid_alive(pid))
    pid = find_filesrv_pid_by_proc();
  int running = port_open_local(FILESRV_PORT, 200);

  printf("{\"ok\":true,\"msg\":\"status\",\"pid\":%d,\"running\":%s}\n",
         pid, running ? "true" : "false");
  return 0;
}
