#!/bin/sh
set -eu

mkdir -p /www/cgi-bin /data/inbox /var/log /var/run

cp -f /mnt/build/status.cgi /mnt/build/files.cgi /www/cgi-bin/
cp -f /mnt/build/upload.cgi /mnt/build/download.cgi /mnt/build/delete.cgi /www/cgi-bin/
chmod +x /www/cgi-bin/*.cgi

echo "[OK] CGI deployed: /www/cgi-bin"
