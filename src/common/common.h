#pragma once
#include <stddef.h>

int ensure_dir(const char *path);
int sanitize_filename(const char *in, char *out, size_t out_sz);
void log_line(const char *log_path, const char *fmt, ...);

int read_first_line(const char *path, char *buf, size_t bufsz);
