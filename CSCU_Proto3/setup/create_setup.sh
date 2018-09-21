#!/bin/sh

BIN_DIR=$1
PROJECT_DIR=$2
TYPE=$3
WORK_DIR=setup
OUT_DIR=${WORK_DIR}/mnt/nandflash
SETUP=setup.tar.gz
FILESYSTEM_9280_DIR=${PROJECT_DIR}/FileSystemFiles_9280/nandflash
FILESYSTEM_335X_DIR=${PROJECT_DIR}/FileSystemFiles_335X/nandflash
FILESYSTEM_COMM_DIR=${PROJECT_DIR}/FileSystemFiles_Common/nandflash

[ -z "$BIN_DIR" ] && exit 0
[ -z "$PROJECT_DIR" ] && exit 0

rm -fr ${PROJECT_DIR}/${WORK_DIR}/mnt
mkdir -p ${PROJECT_DIR}/${OUT_DIR}

if [ "$TYPE" = "IM_9280" ] ;then
TAR=setup_9280.tar.gz
cp -fr ${FILESYSTEM_9280_DIR}/* ${PROJECT_DIR}/${OUT_DIR}
cp -fr ${FILESYSTEM_COMM_DIR}/* ${PROJECT_DIR}/${OUT_DIR}
elif [ "$TYPE" = "IM_335X" ] ;then
TAR=setup_335X.tar.gz
cp -fr ${FILESYSTEM_335X_DIR}/* ${PROJECT_DIR}/${OUT_DIR}
cp -fr ${FILESYSTEM_COMM_DIR}/* ${PROJECT_DIR}/${OUT_DIR}
mkdir -p ${PROJECT_DIR}/${OUT_DIR}/bin
cp -a ${BIN_DIR}/bin/TEUI ${PROJECT_DIR}/${OUT_DIR}/bin
fi

cp -a ${BIN_DIR}/bin/CSCU_A1 ${PROJECT_DIR}/${OUT_DIR}/bin
cp -a ${BIN_DIR}/bin/*.so* ${PROJECT_DIR}/${OUT_DIR}/lib

rm -fr ${PROJECT_DIR}/${WORK_DIR}/${SETUP}
tar -czf ${PROJECT_DIR}/${WORK_DIR}/${SETUP} -C ${PROJECT_DIR}/${WORK_DIR} mnt
mv ${PROJECT_DIR}/${WORK_DIR}/${SETUP} ${PROJECT_DIR}/${WORK_DIR}/${TAR}
rm -fr ${PROJECT_DIR}/${WORK_DIR}/mnt
