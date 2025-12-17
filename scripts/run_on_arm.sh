#!/bin/sh
set -eu

mkdir -p /data/inbox /var/log /var/run

# 启动 filesrv（也可以后续改为通过 ctl.cgi 启停）
/usr/bin/filesrv -p 9000 -d /data/inbox >/dev/null 2>&1 &

# 启动 busybox httpd（前台便于调试；若要后台可加 &）
busybox httpd -f -p 8081 -h /www
