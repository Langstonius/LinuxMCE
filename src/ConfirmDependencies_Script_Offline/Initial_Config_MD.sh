#!/bin/bash

. /usr/pluto/install/Common.sh
. /etc/diskless.conf

## Append local deb-cache to sources.list and sources.list.offline
echo "deb file:/usr/pluto/deb-cache/ sarge main" | cat - /etc/apt/sources.list >/etc/apt/sources.list.2
mv /etc/apt/sources.list.2 /etc/apt/sources.list
echo "deb file:/usr/pluto/deb-cache/ sarge main" > /etc/apt/sources.list.offline
apt-get update &>/dev/null

## Populate pluto.conf file
echo "
# Pluto config file 
MySqlHost = dcerouter
MySqlUser = root
MySqlPassword = 
MySqlDBName = pluto_main
DCERouter = dcerouter
MySqlPort = 3306 
DCERouterPort = 3450 
UseVideoWizard = 1
PK_Device = $Device 
Activation_Code = 1111
" >> /etc/pluto.conf

chmod 666 /etc/pluto.conf


## Create the installation script (old activation.sh) and run it
ok_script=0
chmod +x /usr/pluto/install/activation.sh
/usr/pluto/install/activation.sh

# Replace Initial_Config.sh entry with regular one in inittab (OMFG Hack)
awk '
	/1:23:once/ { next }
	/^#1:2345:respawn/ { print substr($0, 2); next }
	{ print }
' /etc/inittab >/etc/inittab.new
mv -f /etc/inittab.new /etc/inittab

## Clean up after us
apt-get clean
rm -f /dead.letter /root/dead.letter


## Run Install finish script (shared core and mds) 
/usr/pluto/install/Initial_Config_Finish.sh


## Ok, let them have it :) !
reboot
exit 0
