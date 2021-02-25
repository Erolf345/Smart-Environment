# Mesh networks with batman-adv


## Introduction
TODO


## Requirements

In order to set up the mesh on a raspberry pi you need to install batctl. Ifconfig and iwconfig are also required but they should be preinstalled.
``` 
sudo apt install batctl 
```
should be fine but newer versions exist.

Once installed setup the interfaces by running ```sudo ./init.sh``` and reboot.

CAUTION: Wifi networking will be disabled. Either connect using ethernet or reconnect via mesh network.

Now you can use batctl to get information about the network although you may need to update the dc table (do this by running ping -b -c4 169.254.255, then wait up to 5 seconds).

``` batctl dc ``` should return a mapping of hardware to ip-adresses to use with network layer connections.

## Steps to undo

1. ```sudo rm /etc/network/interfaces.d/bat0 && sudo rm /etc/network/interfaces.d/wlan0```

2. Delete line ```denyinterfaces wlan0``` in /etc/dhcpcd.conf

3. ```sudo crontab -e``` delete cronjob for start-batman-adv

4. reboot

## Connecting a laptop to mesh (experimental)

Only tested on ubuntu 18.04-20.04. Will only work if Wifi card supports IBSS or Ad-hoc (run ```iw list``` to check)

Just run ubuntu_connect with root privileges.

Once finished batctl commands should work as expected. If they don't, make sure that the script detects the correct phys interface (output of iw dev isn't stable)


## Sources

https://www.open-mesh.org/projects/batman-adv/wiki
