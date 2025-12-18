#!/bin/sh
nohup socat TCP-LISTEN:8080,fork,reuseaddr TCP:10.0.0.2:8081  >/tmp/socat_web.log 2>&1 &
echo "Forwarding: http://localhost:8080"
