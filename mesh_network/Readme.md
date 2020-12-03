# Mesh networks with batman-adv


## Introduction
TODO


## Steps

In order to set up the mesh on a raspberry pi you need to install batctl. 
``` 
sudo apt install batctl 
```
should be fine but newer versions exist.

Once installed setup the interfaces by running ```sudo ./init.sh``` and reboot. Wifi connectivity may not work after this point.

Then just run start-batman-adv.sh as root. CAUTION: Wifi networking will be disabled. Either connect using ethernet or reconnect via mesh network.

Now you can use batctl to get information about the network. 

``` batctl dc ``` should return a mapping of hardware to ip-adresses to use with network layer connections.



## Connecting a laptop to mesh 

Only tested on ubuntu 18.04-20.04. Will only work if Wifi card supports IBSS or Ad-hoc (run ```iw list``` to check)

Copy and paste each line of ubuntu_connect into a root terminal (some commands need delay)
Once finished batctl commands should work as expected. The laptop is assigned the IP 169.254.100.100 so if you are adding multiple laptops, one will have to change

## Sources

https://www.open-mesh.org/projects/batman-adv/wiki
