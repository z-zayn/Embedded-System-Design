#include "../common/common.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INBOX_DIR "/data/inbox"
#define MAX_UPLOAD_BYTES (200ULL * 1024ULL * 1024ULL) // 200MB，可按需调整

static void cgi_json(void)
{
  printf("Content-Type: application/json\r\n\r\n");
}

static int get_query_param(const char *key, char *out, size_t out_sz)
{
  const char *qs = getenv("QUERY_STRING");
  if (!qs)
    return -1;

  // 只做简单解析：key=value&...
  const char *p = strstr(qs, key);
  if (!p)
    return -1;
  p += strlen(key);
  if (*p != '=')
    return -1;
  p++;

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
  ensure_dir("/data");
  ensure_dir(INBOX_DIR);

  const char *method = getenv("REQUEST_METHOD");
  if (!method || strcmp(method, "POST") != 0)
  {
    printf("{\"ok\":false,\"msg\":\"only POST supported\"}\n");
    return 0;
  }

  char safe[260] = {0};
  if (get_query_param("name", safe, sizeof(safe)) < 0)
  {
    printf("{\"ok\":false,\"msg\":\"missing or bad name\"}\n");
    return 0;
  }

  const char *cl = getenv("CONTENT_LENGTH");
  if (!cl)
  {
    printf("{\"ok\":false,\"msg\":\"missing CONTENT_LENGTH\"}\n");
    return 0;
  }

  unsigned long long len = strtoull(cl, NULL, 10);
  if (len == 0)
  {
    printf("{\"ok\":false,\"msg\":\"empty upload\"}\n");
    return 0;
  }
  if (len > MAX_UPLOAD_BYTES)
  {
    printf("{\"ok\":false,\"msg\":\"file too large\"}\n");
    return 0;
  }

  char final_path[512];
  snprintf(final_path, sizeof(final_path), "%s/%s", INBOX_DIR, safe);

  char tmp_path[512];
  snprintf(tmp_path, sizeof(tmp_path), "%s/.upload_%d_%s.part", INBOX_DIR, (int)getpid(), safe);

  FILE *fp = fopen(tmp_path, "wb");
  if (!fp)
  {
    printf("{\"ok\":false,\"msg\":\"open temp failed (errno=%d)\"}\n", errno);
    return 0;
  }

  unsigned long long left = len;
  char buf[8192];
  while (left > 0)
  {
    size_t chunk = (left > sizeof(buf)) ? sizeof(buf) : (size_t)left;
    size_t r = fread(buf, 1, chunk, stdin);
    if (r == 0)
    {
      fclose(fp);
      unlink(tmp_path);
      printf("{\"ok\":false,\"msg\":\"read body failed\"}\n");
      return 0;
    }
    fwrite(buf, 1, r, fp);
    left -= (unsigned long long)r;
  }

  fclose(fp);

  // 原子替换
  if (rename(tmp_path, final_path) != 0)
  {
    unlink(tmp_path);
    printf("{\"ok\":false,\"msg\":\"rename failed (errno=%d)\"}\n", errno);
    return 0;
  }

  log_line("/var/log/emb_mgr.log", "upload ok name=%s size=%llu", safe, len);
  printf("{\"ok\":true,\"msg\":\"uploaded\",\"name\":\"%s\",\"size\":%llu}\n", safe, len);
  return 0;
}
