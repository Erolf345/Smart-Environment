read -p "What is the name of your mesh interface: " mesh_if


systemctl stop NetworkManager.service

sleep 1
iw dev $mesh_if del
sleep 1
iw phy phy0 interface add $mesh_if type ibss
sleep 1
ip link set up mtu 1532 dev $mesh_if #todo change this
sleep 1
ifconfig $mesh_if down
sleep 1
iwconfig $mesh_if mode ad-hoc essid my-mesh-network ap A6:00:28:01:5F:F8 channel 5
sleep 1
ifconfig $mesh_if up
sleep 1
batctl if add $mesh_if
sleep 1
ifconfig bat0 169.254.100.100 netmask 255.255.0.0 # TODO: replace this with avahi
sleep 1
sudo sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
ping -c 2 -b 169.254.255.255
