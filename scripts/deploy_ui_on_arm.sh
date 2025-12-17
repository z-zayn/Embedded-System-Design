#!/bin/sh
set -eu

SRC=/mnt/webui/dist
DST=/www

if [ ! -d "$SRC" ]; then
  echo "[ERR] dist not found: $SRC (run scripts/build_ui.sh in WSL first)"
  exit 1
fi

mkdir -p "$DST"

# 只覆盖静态文件，不动 /www/cgi-bin
cp -f "$SRC/index.html" "$DST/"
cp -rf "$SRC/assets" "$DST/"

echo "[OK] UI deployed to $DST (cgi-bin kept)"
