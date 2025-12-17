#!/bin/sh
set -eu

WEB_PORT=8081
WEB_ROOT=/www

mkdir -p /data/inbox /var/log /var/run "$WEB_ROOT/cgi-bin"

# 如果端口已被占用，说明 httpd 已在运行，直接退出即可（避免 Address already in use）
if ss -lnt 2>/dev/null | grep -q ":$WEB_PORT" || netstat -lnt 2>/dev/null | grep -q ":$WEB_PORT"; then
  echo "[INFO] httpd already listening on :$WEB_PORT"
  exit 0
fi

# 后台启动 httpd
busybox httpd -f -p "$WEB_PORT" -h "$WEB_ROOT" >/var/log/httpd.log 2>&1 &
echo "[OK] httpd started on :$WEB_PORT (root=$WEB_ROOT)"
