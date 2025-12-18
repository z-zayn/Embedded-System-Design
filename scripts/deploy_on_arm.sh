#!/bin/sh
set -eu

# 运行在 ARM 内：/mnt 是共享工程目录
mkdir -p /www/cgi-bin /data/inbox /var/log /var/run /usr/bin

# 部署 CGI
cp -f /mnt/build/status.cgi /mnt/build/files.cgi /mnt/build/ctl.cgi /mnt/build/download.cgi /mnt/build/delete.cgi /www/cgi-bin/
chmod +x /www/cgi-bin/*.cgi

# 部署服务
cp -f /mnt/build/filesrv /usr/bin/filesrv
chmod +x /usr/bin/filesrv

echo "[OK] deployed: web=/www cgi=/www/cgi-bin filesrv=/usr/bin/filesrv"
