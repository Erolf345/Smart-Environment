systemctl stop NetworkManager.service


iw dev wlp0s20f3 del
iw phy phy0 interface add wlp0s20f3 type ibss
ip link set up mtu 1532 dev wlp0s20f3 #todo change this
ifconfig wlp0s20f3 down
iwconfig wlp0s20f3 mode ad-hoc essid my-mesh-network ap A6:00:28:01:5F:F8 channel 5
ifconfig wlp0s20f3 up

ifconfig bat0 169.254.100.100 netmask 255.255.0.0

batctl if add wlp0s20f3

