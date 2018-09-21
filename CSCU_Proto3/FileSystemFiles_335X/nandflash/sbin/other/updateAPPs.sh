#!/bin/sh

# Update APPs 
APP_SRC_PATH="/mnt/nandflash_new"
APP_DST_PATH="/mnt/nandflash"

#APP_SRC_VER_FILE="$APP_SRC_PATH/version_tgood.sh"
#APP_DST_VER_FILE="$APP_DST_PATH/version_tgood.sh"
#APP_SRC_VER_STR=`cat $APP_SRC_VER_FILE`
#APP_DST_VER_STR=`cat $APP_DST_VER_FILE`

#if [ "$APP_SRC_VER_STR" \> "$APP_DST_VER_STR" ]; then 
if [ -d $APP_SRC_PATH ]; then
    echo "Updating APPs, please wait..."
    #rm -rf $APP_DST_PATH
    cp -rvf $APP_SRC_PATH/* $APP_DST_PATH/
    if [ $? -ne 0 ]; then 
        echo "Updating APPs failed!"
    else
        echo "Updating APPs successfully"
        chmod +x $APP_DST_PATH/bin/*
        rm -rf $APP_SRC_PATH
    fi
else
    echo "No need to update APPs"
fi                                     
