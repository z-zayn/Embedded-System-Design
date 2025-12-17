#include "../common/common.h"
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_PORT 9000
#define LOG_PATH "/var/log/emb_mgr.log"
#define PID_PATH "/var/run/filesrv.pid"

static uint64_t ntohll_u64(uint64_t x)
{
  uint32_t hi = ntohl((uint32_t)(x >> 32));
  uint32_t lo = ntohl((uint32_t)(x & 0xffffffffu));
  return ((uint64_t)lo << 32) | hi;
}

static int recv_all(int fd, void *buf, size_t len)
{
  size_t got = 0;
  while (got < len)
  {
    ssize_t r = recv(fd, (char *)buf + got, len - got, 0);
    if (r == 0)
      return -1;
    if (r < 0)
    {
      if (errno == EINTR)
        continue;
      return -1;
    }
    got += (size_t)r;
  }
  return 0;
}

static int send_all(int fd, const void *buf, size_t len)
{
  size_t sent = 0;
  while (sent < len)
  {
    ssize_t w = send(fd, (const char *)buf + sent, len - sent, 0);
    if (w < 0)
    {
      if (errno == EINTR)
        continue;
      return -1;
    }
    sent += (size_t)w;
  }
  return 0;
}

static void write_pid(void)
{
  ensure_dir("/var/run");
  FILE *fp = fopen(PID_PATH, "w");
  if (fp)
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }
}

int main(int argc, char **argv)
{
  int port = DEFAULT_PORT;
  const char *dir = "/data/inbox";

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-p") && i + 1 < argc)
      port = atoi(argv[++i]);
    else if (!strcmp(argv[i], "-d") && i + 1 < argc)
      dir = argv[++i];
  }

  ensure_dir("/var/log");
  ensure_dir("/data");
  ensure_dir(dir);

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
  {
    perror("socket");
    return 1;
  }

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons((uint16_t)port);

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    return 1;
  }
  if (listen(s, 8) < 0)
  {
    perror("listen");
    return 1;
  }

  write_pid();
  log_line(LOG_PATH, "filesrv started on port=%d dir=%s", port, dir);

  for (;;)
  {
    struct sockaddr_in cli;
    socklen_t clen = sizeof(cli);
    int c = accept(s, (struct sockaddr *)&cli, &clen);
    if (c < 0)
    {
      if (errno == EINTR)
        continue;
      perror("accept");
      continue;
    }

    char ip[64];
    snprintf(ip, sizeof(ip), "%s", inet_ntoa(cli.sin_addr));

    uint32_t name_len_n = 0;
    uint64_t size_n = 0;

    if (recv_all(c, &name_len_n, sizeof(name_len_n)) < 0 ||
        recv_all(c, &size_n, sizeof(size_n)) < 0)
    {
      close(c);
      continue;
    }

    uint32_t name_len = ntohl(name_len_n);
    uint64_t fsize = ntohll_u64(size_n);

    if (name_len == 0 || name_len > 255 || fsize > (1024ULL * 1024ULL * 1024ULL))
    {
      const char *msg = "ERR:bad header\n";
      send_all(c, msg, strlen(msg));
      close(c);
      continue;
    }

    char rawname[300];
    memset(rawname, 0, sizeof(rawname));
    if (recv_all(c, rawname, name_len) < 0)
    {
      close(c);
      continue;
    }
    rawname[name_len] = '\0';

    char safe[260];
    if (sanitize_filename(rawname, safe, sizeof(safe)) < 0)
    {
      const char *msg = "ERR:bad filename\n";
      send_all(c, msg, strlen(msg));
      close(c);
      continue;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, safe);

    FILE *fp = fopen(path, "wb");
    if (!fp)
    {
      log_line(LOG_PATH, "from=%s open failed path=%s err=%d", ip, path, errno);
      const char *msg = "ERR:open failed\n";
      send_all(c, msg, strlen(msg));
      close(c);
      continue;
    }

    char buf[4096];
    uint64_t left = fsize;
    while (left > 0)
    {
      size_t chunk = (left > sizeof(buf)) ? sizeof(buf) : (size_t)left;
      ssize_t r = recv(c, buf, chunk, 0);
      if (r <= 0)
      {
        fclose(fp);
        unlink(path);
        goto done_err;
      }
      fwrite(buf, 1, (size_t)r, fp);
      left -= (uint64_t)r;
    }
    fclose(fp);

    log_line(LOG_PATH, "from=%s saved=%s size=%" PRIu64, ip, path, fsize);
    const char *ok = "OK\n";
    send_all(c, ok, strlen(ok));
    close(c);
    continue;

  done_err:
    log_line(LOG_PATH, "from=%s recv failed name=%s", ip, safe);
    {
      const char *msg = "ERR:recv failed\n";
      send_all(c, msg, strlen(msg));
    }
    close(c);
  }
}
