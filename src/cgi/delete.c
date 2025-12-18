#include "../common/common.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define INBOX_DIR "/data/inbox"

static void cgi_json(void)
{
  printf("Content-Type: application/json\r\n\r\n");
}

static int get_param_name(char *out, size_t out_sz)
{
  const char *qs = getenv("QUERY_STRING");
  if (!qs)
    return -1;

  const char *p = strstr(qs, "name=");
  if (!p)
    return -1;
  p += 5;

  char enc[512] = {0};
  size_t i = 0;
  while (p[i] && p[i] != '&' && i + 1 < sizeof(enc))
  {
    enc[i] = p[i];
    i++;
  }
  enc[i] = '\0';

  char raw[512] = {0};
  if (url_decode(enc, raw, sizeof(raw)) < 0)
    return -1;

  if (sanitize_filename(raw, out, out_sz) < 0)
    return -1;
  return 0;
}

int main(void)
{
  cgi_json();

  char safe[260] = {0};
  if (get_param_name(safe, sizeof(safe)) < 0)
  {
    printf("{\"ok\":false,\"msg\":\"bad name\"}\n");
    return 0;
  }

  char path[512];
  snprintf(path, sizeof(path), "%s/%s", INBOX_DIR, safe);

  if (unlink(path) == 0)
  {
    printf("{\"ok\":true,\"msg\":\"deleted\"}\n");
    return 0;
  }

  // 删除失败：返回 errno 信息（简洁即可）
  printf("{\"ok\":false,\"msg\":\"delete failed (errno=%d)\"}\n", errno);
  return 0;
}
