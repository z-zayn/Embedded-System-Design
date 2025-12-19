#!/bin/sh
set -eu

SRC=/mnt/webui/dist
DST=/www

[ -d "$SRC" ] || { echo "[ERR] dist not found: $SRC"; exit 1; }
mkdir -p "$DST" "$DST/cgi-bin"

# 1) 清理旧静态文件（保留 /www/cgi-bin）
for p in "$DST"/*; do
  [ -e "$p" ] || continue
  [ "$(basename "$p")" = "cgi-bin" ] && continue
  rm -rf "$p"
done

# 2) 复制 dist 的全部内容到 /www
# 说明：dist 里可能只有 index.html，也可能还有 favicon/icons/assets 等目录
cp -rf "$SRC"/* "$DST"/ 2>/dev/null || true

echo "[OK] UI deployed: $SRC -> $DST (kept $DST/cgi-bin)"
