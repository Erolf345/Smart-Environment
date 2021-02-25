#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Copy preconfigured interface definitions to correct location
sudo cp ./interfaces/bat0 /etc/network/interfaces.d/
sudo cp ./interfaces/wlan0 /etc/network/interfaces.d/

# Prevent DHCPCD from automatically configuring wlan0, THIS IS KEY
echo 'denyinterfaces wlan0' | sudo tee --append /etc/dhcpcd.conf

# connect to mesh on startup
crontab -l > mycron
#echo new cron into cron file
echo "@reboot $SCRIPTPATH/start-batman-adv.sh" >> mycron
#install new cron file
crontab mycron
rm mycron
