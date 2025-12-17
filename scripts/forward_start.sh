#!/bin/sh
nohup socat TCP-LISTEN:8080,fork,reuseaddr TCP:10.0.0.2:80  >/tmp/socat_web.log 2>&1 &
nohup socat TCP-LISTEN:9000,fork,reuseaddr TCP:10.0.0.2:9000 >/tmp/socat_file.log 2>&1 &
echo "Forwarding: http://localhost:8080  and  tcp://localhost:9000"
