#include "SocketHeader.h"
#include <string.h>

int main(int argc, char *argv[])
{
  struct sockaddr_in peer;
  int s, rc;
  FILE *fp;
  char buf[1024];
  int n, total = 0;

  if (argc != 3)
  {
    printf("Usage: %s <server_ip> <filename>\n", argv[0]);
    return 1;
  }

  /* 1. 打开要发送的本地文件 */
  fp = fopen(argv[2], "rb");
  if (!fp)
  {
    perror("fopen");
    return 1;
  }

  /* 2. 填写服务器（ARM 板）地址 */
  peer.sin_family = AF_INET;
  peer.sin_port = htons(12345);
  peer.sin_addr.s_addr = inet_addr(argv[1]); /* 比如 10.0.0.2 */

  /* 3. 创建 socket */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    print_error("socket");

  /* 4. 连接服务器 */
  rc = connect(s, (struct sockaddr *)&peer, sizeof(peer));
  if (rc)
    print_error("connect");

  printf("FileClient: connected to %s:12345\n", argv[1]);

  /* 5. 循环读取文件并发送 */
  while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
  {
    rc = send(s, buf, n, 0);
    if (rc <= 0)
      print_error("send");
    total += rc;
  }

  fclose(fp);
  printf("FileClient: send finished, total bytes = %d\n", total);

  return 0;
}
