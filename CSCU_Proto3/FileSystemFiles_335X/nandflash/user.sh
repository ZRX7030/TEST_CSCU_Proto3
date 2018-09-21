#!/bin/sh

# The startup entry of user APPs
# NOTE: The environment variables in this file is invisible out of the file!

echo "-------------------- /mnt/nandflash/user.sh Start -----------------------"

# Set environment variables
source "/etc/profile.d/set_env.sh"

# Run scripts
/mnt/nandflash/bin/udisk_auto_update.sh
/mnt/nandflash/bin/sqlite3_init.sh
/mnt/nandflash/bin/net_config.sh eth0
/mnt/nandflash/bin/vpn_start.sh

#create qrcode
/mnt/nandflash/bin/create_qrcode.sh

# Run APPs
mkdir -p /var/spool/cron/crontabs
crontab /mnt/nandflash/etc/cron.d/root
crond

# Run APPs
ln -s /mnt/nandflash/etc/localtime /etc/
/mnt/nandflash/bin/cscu_a1.init > /dev/null 2>&1 &
/mnt/nandflash/bin/teui.init > /dev/null 2>&1 &
/mnt/nandflash/sbin/screensaver/screensaver &
/mnt/nandflash/sbin/other/traffic_statistics &
/mnt/nandflash/sbin/mobile/mobile.sh >/dev/null &
#/mnt/nandflash/sbin/mobile/mobile.sh | tee -a /mnt/nandflash/log/log_mobile.txt &

sleep 10
/mnt/nandflash/bin/get_version.sh
echo "-------------------- /mnt/nandflash/user.sh End -------------------------"

