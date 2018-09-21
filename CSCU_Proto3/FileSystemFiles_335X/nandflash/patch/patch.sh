#!/bin/sh

# Yannis: 2017-10-11
# Patch for kernel & rootfs V01B03D00

installed=0
kernel_ver=$(cat /proc/version_tgood | grep "Kernel" -A 1 | awk -F "Version=" '{print $2}'| tr -d ' ' | tr -d '\n')
rootfs_ver=$(cat /etc/version_tgood | grep "Filesystem" -A 1 | awk -F "Version=" '{print $2}'| tr -d ' ' | tr -d '\n')

# kernel
if [ "$kernel_ver" \< "V01B08D00" ]; then
    installed=1
    echo
    echo "Installing kernel patch, please wait..."
    /mnt/nandflash/sbin/programmer/program_kernel.sh /mnt/nandflash/patch/uImage
    echo "Installed kernel patch"
    echo
fi

# rootfs
if [ "$rootfs_ver" \< "V01B08D00" ]; then
    installed=1
    echo
    echo "Installing rootfs patch, please wait..."

    rm -f  /bin/chat
    rm -f  /bin/pppd
    rm -f  /bin/pppdump
    rm -f  /bin/pppstats

    rm -rf /etc/ppp/chat/
    rm -f  /etc/ppp/peers/*
    rm -f  /etc/ppp/connect-errors
    rm -f  /etc/ppp/gprs-connect-chat
    rm -f  /etc/ppp/ppp-on-dialer

    rm -rf /lib/pppd/

    rm -rf /mnt/nfs/

    rm -f  /sbin/ftpd

    rm -rf /tgood/

    rm -rf /usr/lib/pppd/
    rm -f  /usr/sbin/chat
    rm -f  /usr/sbin/pppd
    rm -f  /usr/sbin/pppdump
    rm -f  /usr/sbin/pppstats

    tar -xvf /mnt/nandflash/patch/rootfs_patch.tar.gz -C /

    echo "Installed rootfs patch"
    echo
fi

if [ $installed -eq 1 ]; then
    reboot
fi

