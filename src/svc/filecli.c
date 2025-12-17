#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static uint64_t htonll_u64(uint64_t x)
{
  uint32_t hi = htonl((uint32_t)(x >> 32));
  uint32_t lo = htonl((uint32_t)(x & 0xffffffffu));
  return ((uint64_t)lo << 32) | hi;
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

int main(int argc, char **argv)
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: %s <ip> <port> <file>\n", argv[0]);
    return 1;
  }
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  const char *path = argv[3];

  FILE *fp = fopen(path, "rb");
  if (!fp)
  {
    perror("fopen");
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  if (sz < 0)
  {
    fclose(fp);
    return 1;
  }

  const char *fname = strrchr(path, '/');
  fname = fname ? fname + 1 : path;

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
  {
    perror("socket");
    fclose(fp);
    return 1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((uint16_t)port);
  if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
  {
    fprintf(stderr, "bad ip\n");
    return 1;
  }
  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("connect");
    return 1;
  }

  uint32_t name_len = (uint32_t)strlen(fname);
  uint32_t name_len_n = htonl(name_len);
  uint64_t size_n = htonll_u64((uint64_t)sz);

  if (send_all(s, &name_len_n, sizeof(name_len_n)) < 0 ||
      send_all(s, &size_n, sizeof(size_n)) < 0 ||
      send_all(s, fname, name_len) < 0)
  {
    fprintf(stderr, "send header failed\n");
    return 1;
  }

  char buf[4096];
  size_t r;
  while ((r = fread(buf, 1, sizeof(buf), fp)) > 0)
  {
    if (send_all(s, buf, r) < 0)
    {
      fprintf(stderr, "send data failed\n");
      return 1;
    }
  }
  fclose(fp);

  char resp[128] = {0};
  recv(s, resp, sizeof(resp) - 1, 0);
  printf("%s", resp);
  close(s);
  return 0;
}
