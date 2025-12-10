#include "SocketHeader.h"
#include <string.h>

int main(void)
{
  struct sockaddr_in local, client;
  unsigned int client_len;
  int s, s1, rc;
  char *filename = "recv_from_pc.dat";
  FILE *fp;
  char buf[1024];
  int total = 0;

  /* 1. 填写本地监听地址 */
  local.sin_family = AF_INET;
  local.sin_port = htons(12345);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  /* 2. 创建 socket */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    print_error("socket");

  /* 3. 绑定并监听 */
  rc = bind(s, (struct sockaddr *)&local, sizeof(local));
  if (rc < 0)
    print_error("bind");

  rc = listen(s, 1);
  if (rc < 0)
    print_error("listen");

  printf("FileServer: waiting for connection...\n");

  /* 4. 接受客户端连接 */
  client_len = sizeof(client);
  s1 = accept(s, (struct sockaddr *)&client, &client_len);
  if (s1 < 0)
    print_error("accept");

  printf("FileServer: client connected.\n");

  /* 5. 打开本地文件用于写入 */
  fp = fopen(filename, "wb");
  if (!fp)
  {
    perror("fopen");
    exit(1);
  }

  /* 6. 循环接收并写入文件 */
  while ((rc = recv(s1, buf, sizeof(buf), 0)) > 0)
  {
    fwrite(buf, 1, rc, fp);
    total += rc;
  }

  if (rc < 0)
    print_error("recv");

  fclose(fp);
  printf("FileServer: receive finished, total bytes = %d, saved to %s\n",
         total, filename);

  return 0;
}
