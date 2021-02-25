#!/bin/bash

# NOTE: This script needs to be rewritten as there are probably a bunch of double assignments and sloppy configuration. It's working now thought so I will leave it as is

# Tell batman-adv which interface to use
sudo batctl if add wlan0


# Activates the interfaces for batman-adv
sudo ifconfig wlan0 up
sudo ifconfig bat0 up # bat0 is created via the first command

sleep 4 # give bat0 time to setup

# connects the actual mesh
sudo ip link set wlan0 down
sudo iwconfig wlan0 channel 3 # arbitrary channel
sudo ip link set wlan0 up 
sudo ifconfig bat0 mtu 1468 # decrease mtu of bat0 because we can't increase mtu of wlan0
sudo iw wlan0 ibss join my-mesh-network 2432 # join mesh (frequency must match channel)

sleep 1 # give time to setup
sudo sysctl net.ipv4.icmp_echo_ignore_broadcasts=0 # allow other pi's to discover us using broadcast pings
ping -c 2 -b 169.254.255.255 # discover other nodes by broadcast ping. (This will update their ip adresses in the batctl table)
