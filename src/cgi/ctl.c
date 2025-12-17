#include "../common/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define PID_PATH "/var/run/filesrv.pid"
#define LOG_PATH "/var/log/emb_mgr.log"

static void cgi_header(void)
{
  printf("Content-Type: application/json\r\n\r\n");
}

static int read_pid(void)
{
  FILE *fp = fopen(PID_PATH, "r");
  if (!fp)
    return -1;
  int pid = -1;
  fscanf(fp, "%d", &pid);
  fclose(fp);
  return pid;
}

int main(void)
{
  cgi_header();

  const char *qs = getenv("QUERY_STRING");
  if (!qs)
    qs = "";

  // 仅支持：/cgi-bin/ctl.cgi?action=start 或 action=stop
  if (strstr(qs, "action=start"))
  {
    // 你可按实际路径修改 filesrv 位置
    int ret = system("/usr/bin/filesrv -p 9000 -d /data/inbox >/dev/null 2>&1 &");
    log_line(LOG_PATH, "ctl start ret=%d", ret);
    printf("{\"ok\":true,\"msg\":\"started\"}\n");
    return 0;
  }

  if (strstr(qs, "action=stop"))
  {
    int pid = read_pid();
    if (pid > 1)
    {
      kill(pid, SIGTERM);
      unlink(PID_PATH);
      log_line(LOG_PATH, "ctl stop pid=%d", pid);
      printf("{\"ok\":true,\"msg\":\"stopped\"}\n");
    }
    else
    {
      printf("{\"ok\":false,\"msg\":\"no pid\"}\n");
    }
    return 0;
  }

  printf("{\"ok\":false,\"msg\":\"usage: ?action=start|stop\"}\n");
  return 0;
}
