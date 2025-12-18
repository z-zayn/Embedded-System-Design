#include "common.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

int ensure_dir(const char *path)
{
  struct stat st;
  if (stat(path, &st) == 0)
    return S_ISDIR(st.st_mode) ? 0 : -1;
  if (mkdir(path, 0755) == 0)
    return 0;
  return -1;
}

int sanitize_filename(const char *in, char *out, size_t out_sz)
{
  // 只允许 [A-Za-z0-9._-]，其他替换为 '_'
  // 同时禁止出现 ".." 和 '/'，避免目录穿越
  if (!in || !out || out_sz < 2)
    return -1;
  size_t j = 0;
  for (size_t i = 0; in[i] && j + 1 < out_sz; i++)
  {
    char c = in[i];
    if (c == '/' || c == '\\')
      c = '_';
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')
    {
      out[j++] = c;
    }
    else
    {
      out[j++] = '_';
    }
  }
  out[j] = '\0';
  if (strstr(out, ".."))
    return -1;
  if (out[0] == '\0')
    return -1;
  return 0;
}

void log_line(const char *log_path, const char *fmt, ...)
{
  FILE *fp = fopen(log_path, "a");
  if (!fp)
    return;

  time_t t = time(NULL);
  struct tm tmv;
  localtime_r(&t, &tmv);
  char ts[64];
  strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tmv);

  fprintf(fp, "[%s] ", ts);

  va_list ap;
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);

  fprintf(fp, "\n");
  fclose(fp);
}

int read_first_line(const char *path, char *buf, size_t bufsz)
{
  FILE *fp = fopen(path, "r");
  if (!fp)
    return -1;
  if (!fgets(buf, (int)bufsz, fp))
  {
    fclose(fp);
    return -1;
  }
  fclose(fp);
  // 去掉换行
  size_t n = strlen(buf);
  while (n && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
    buf[--n] = '\0';
  return 0;
}

static int hexval(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

int url_decode(const char *in, char *out, size_t out_sz)
{
  if (!in || !out || out_sz < 2)
    return -1;
  size_t j = 0;
  for (size_t i = 0; in[i] && j + 1 < out_sz; i++)
  {
    if (in[i] == '%')
    {
      int a = hexval(in[i + 1]);
      int b = hexval(in[i + 2]);
      if (a < 0 || b < 0)
        return -1;
      out[j++] = (char)((a << 4) | b);
      i += 2;
    }
    else if (in[i] == '+')
    {
      out[j++] = ' ';
    }
    else
    {
      out[j++] = in[i];
    }
  }
  out[j] = '\0';
  return 0;
}