sudo cp ./interfaces/bat0 /etc/network/interfaces.d/
sudo cp ./interfaces/wlan0 /etc/network/interfaces.d/

# Prevent DHCPCD from automatically configuring wlan0, THIS IS KEY
echo 'denyinterfaces wlan0' | sudo tee --append /etc/dhcpcd.conf