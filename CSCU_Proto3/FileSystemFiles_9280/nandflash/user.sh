#!/bin/sh
export set QTDIR=/opt/qt-4.8 
export set QPEDIR=/opt/qt-4.8 
export set QWS_DISPLAY='LinuxFB:/dev/fb0' 
export set QWS_DISPLAY='LinuxFB:mmWidth130:mmHeight100:0' 
export set QWS_KEYBOARD='TTY:/dev/tty1' 

export set TSLIB_TSDEVICE=/dev/input/event0 
export set TSLIB_CALIBFILE=/etc/pointercal 
export set TSLIB_CONFFILE=/etc/ts.conf 
export set TSLIB_PLUGINDIR=/lib/ts 
export set QWS_MOUSE_PROTO='TSLIB:/dev/input/event0' 

export set QT_PLUGIN_PATH=$QTDIR/plugins/ 
export set QT_QWS_FONTDIR=$QTDIR/lib/fonts/ 
export set PATH=$QPEDIR/bin:/mnt/nandflash/bin:$PATH
export set LD_LIBRARY_PATH=$QTDIR/lib:$QPEDIR/plugins/imageformats:/mnt/nandflash/lib:$LD_LIBRARY_PATH 

cp -f /mnt/nandflash/etc/profile /etc/profile
mv /lib/libsqlite3.so.0 libsqlite3.so.0.bak

ln -s /mnt/nandflash/etc/localtime /etc/

/mnt/nandflash/bin/cscu.sh &
