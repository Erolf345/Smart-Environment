# Mesh networks with batman-adv


## Introduction
TODO


## Steps

In order to set up the mesh on a raspberry pi you need to install batctl. 
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

## Connecting a laptop to mesh 

Only tested on ubuntu 18.04-20.04. Will only work if Wifi card supports IBSS or Ad-hoc (run ```iw list``` to check)

Copy and paste each line of ubuntu_connect into a root terminal (some commands need delay)
Once finished batctl commands should work as expected.

## Sources

https://www.open-mesh.org/projects/batman-adv/wiki
