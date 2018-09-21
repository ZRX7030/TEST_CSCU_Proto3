#!/bin/sh

server_port=8888

# Server
iperf -s -p $server_port -i 1

