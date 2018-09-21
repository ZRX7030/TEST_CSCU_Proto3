#!/bin/sh

FTP_HOST=ftp://qpe-bj-deeri.chinacloudapp.cn/pub
WORK_DIR=/mnt/nandflash/upload_log
WORK_TIME=`date +%Y%m%d%H%M%S`
LOG_FILE=log_can0_${WORK_TIME}.tar.gz
STATION=`cat /mnt/nandflash/config.ini | sed -n -e"/^\[PROTOBUF_SERVER\]/,/^\[/p" | sed -n  "/^Station=/p" | sed "s/Station=//"`

[ -z "$WORK_TIME" ] && exit 1
[ -z "$STATION" ] && exit 1

mkdir -p ${WORK_DIR}
cd ${WORK_DIR}
tar -czf ${LOG_FILE} ../log/log_can0.txt*

[ ! -f ${WORK_DIR}/${LOG_FILE} ] && exit 1

curl -s --retry 3 -u ftp:welcome --ftp-create-dirs --ftp-port eth0 -T ${WORK_DIR}/${LOG_FILE} ${FTP_HOST}/${STATION}/
rm -fr ${WORK_DIR}
