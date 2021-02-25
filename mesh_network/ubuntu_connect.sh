#!/bin/bash
read -p "What is the name of your mesh interface: " mesh_if

asdf=$(sudo iw dev | grep -B 1 "$mesh_if" | grep "phy")
phys_if=$(awk '{gsub(/\#/,"")}1' <<< "$asdf") # ATTENTION: THIS PART IS VERY EXPERIMTENTAL. If the script doesn't work make sure that the number defined here is actually the physical interface of the device you want to connect

systemctl stop NetworkManager.service

sleep 1
iw dev $mesh_if del
sleep 1
echo "$phys_if"
iw phy "$phys_if" interface add $mesh_if type ibss
sleep 1
ip link set up mtu 1532 dev $mesh_if #todo change this
sleep 1
ifconfig $mesh_if down
sleep 1
sudo batctl if add $mesh_if
sleep 1
ifconfig $mesh_if up
sleep 1
ifconfig bat0 up
sleep 1
iwconfig $mesh_if channel 3
sleep 1
iw $mesh_if ibss join my-mesh-network 2432
sleep 1
sudo avahi-autoipd -D bat0
echo "Now wait for avahi autoipd to generate ip adress"
