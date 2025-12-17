#!/usr/bin/env bash
set -euo pipefail
cd /home/zayn/project/Embedded-System-Design

export CROSS=arm-linux-

make clean
make -j"$(nproc)" all

echo "[OK] outputs in: /home/zayn/project/Embedded-System-Design/build"
ls -lh build
