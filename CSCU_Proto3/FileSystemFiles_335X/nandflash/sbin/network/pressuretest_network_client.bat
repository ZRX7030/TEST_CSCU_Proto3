@echo off

set server_ip=172.0.0.12
set server_port=8888

: Client
iperf -c %server_ip% -p %server_port% -t 10000

