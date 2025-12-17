#!/usr/bin/env bash
set -euo pipefail

cd /home/zayn/project/Embedded-System-Design/webui
npm run build

echo "[OK] webui built: /home/zayn/project/Embedded-System-Design/webui/dist"
ls -lh dist
