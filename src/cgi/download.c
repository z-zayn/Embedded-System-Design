#include "../common/common.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define INBOX_DIR "/data/inbox"

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

static void http_404(const char *msg)
{
  printf("Status: 404 Not Found\r\n");
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s\n", msg ? msg : "not found");
}

int main(void)
{
  char safe[260] = {0};
  if (get_param_name(safe, sizeof(safe)) < 0)
  {
    http_404("bad name");
    return 0;
  }

  char path[512];
  snprintf(path, sizeof(path), "%s/%s", INBOX_DIR, safe);

  struct stat st;
  if (stat(path, &st) != 0 || !S_ISREG(st.st_mode))
  {
    http_404("file not found");
    return 0;
  }

  FILE *fp = fopen(path, "rb");
  if (!fp)
  {
    http_404("open failed");
    return 0;
  }

  printf("Content-Type: application/octet-stream\r\n");
  printf("Content-Disposition: attachment; filename=\"%s\"\r\n", safe);
  printf("Content-Length: %ld\r\n", (long)st.st_size);
  printf("\r\n");

  char buf[8192];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
  {
    fwrite(buf, 1, n, stdout);
  }
  fclose(fp);
  return 0;
}
