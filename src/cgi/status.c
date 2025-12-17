#include "../common/common.h"
#include <stdio.h>
#include <string.h>

static void cgi_header(void)
{
  printf("Content-Type: application/json\r\n\r\n");
}

int main(void)
{
  cgi_header();

  char load[128] = "0 0 0";
  char up[128] = "0 0";
  char mem[1024] = {0};

  read_first_line("/proc/loadavg", load, sizeof(load));
  read_first_line("/proc/uptime", up, sizeof(up));

  // 简化：从 /proc/meminfo 取 MemTotal/MemAvailable；若无 MemAvailable 则用 MemFree+Buffers+Cached 近似
  FILE *fp = fopen("/proc/meminfo", "r");
  long mem_total = -1, mem_avail = -1;
  long mem_free = 0, buffers = 0, cached = 0;

  if (fp)
  {
    while (fgets(mem, sizeof(mem), fp))
    {
      sscanf(mem, "MemTotal: %ld kB", &mem_total);
      if (sscanf(mem, "MemAvailable: %ld kB", &mem_avail) == 1)
        continue;
      sscanf(mem, "MemFree: %ld kB", &mem_free);
      sscanf(mem, "Buffers: %ld kB", &buffers);
      sscanf(mem, "Cached: %ld kB", &cached);
    }
    fclose(fp);
  }

  if (mem_avail < 0)
    mem_avail = mem_free + buffers + cached;

  // 只取 load1
  double load1 = 0.0;
  sscanf(load, "%lf", &load1);
  double uptime_s = 0.0;
  sscanf(up, "%lf", &uptime_s);

  printf("{\"uptime_s\":%.0f,\"load1\":%.2f,\"mem_total_kb\":%ld,\"mem_avail_kb\":%ld}\n", uptime_s, load1, mem_total, mem_avail);
  return 0;
}
