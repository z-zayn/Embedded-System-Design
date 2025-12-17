#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static void cgi_header(void)
{
  printf("Content-Type: application/json\r\n\r\n");
}

int main(void)
{
  cgi_header();
  const char *dir = "/data/inbox";
  DIR *dp = opendir(dir);
  if (!dp)
  {
    printf("[]\n");
    return 0;
  }

  printf("[");
  int first = 1;
  struct dirent *de;
  while ((de = readdir(dp)))
  {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, de->d_name);

    struct stat st;
    if (stat(path, &st) != 0)
      continue;
    if (!S_ISREG(st.st_mode))
      continue;

    if (!first)
      printf(",");
    first = 0;

    printf("{\"name\":\"%s\",\"size\":%ld,\"mtime\":%ld}", de->d_name, (long)st.st_size, (long)st.st_mtime);
  }
  printf("]\n");
  closedir(dp);
  return 0;
}
