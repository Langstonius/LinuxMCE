#!/bin/bash

. /usr/pluto/bin/Network_Parameters.sh
. /usr/pluto/bin/Config_Ops.sh
. /usr/pluto/bin/SQL_Ops.sh

# vars:
# CORE_INTERNAL_ADDRESS

Vars="CORE_INTERNAL_ADDRESS"

if [[ "$NetIfConf" == 0 || "$NPflagReconfNetwork" == yes ]]; then
	if [[ "$NetIfConf" == 0 ]]; then
		echo "First network config"
	elif [[ "$NPflagReconfNetwork" == yes ]]; then
		echo "Forced network config"
	fi
	echo "Populating network settings from current system config"
	NetSettings=$(ParseInterfaces)
	ExtData=$(echo "$NetSettings" | head -1)
	ExtractData "$ExtData"
	if [[ -z "$DHCPsetting" ]]; then
		echo "No DHCP Setting found. Not storing internal interface data"
		if [ "$ExtIP" == "dhcp" ]; then
			NetIfConf="$ExtIf,dhcp|"
		else
			NetIfConf="$ExtIf,$ExtIP,$ExtNetmask,$Gateway,$DNS|"
		fi
		IntIf="$ExtIf:0"
		IntIP=192.168.80.1
		IntNetmask=255.255.255.0
	else
		echo "Using DHCP Setting to set up internal interface data: '$DHCPsetting'"
		echo "Found $NCards network cards"
		IntNetmask="255.255.255.0"
		IntIP="$(echo "$DHCPsetting" | cut -d. -f-3).1"
		if [[ "$NCards" -eq 1 ]]; then
			IntIf="eth0:0"
#		elif [[ "$NCards" -eq 2 ]]; then
		else
			[[ "$ExtIf" == "eth0" ]] && IntIf="eth1" || IntIf="eth0"
		fi
		if [[ "$ExtIP" == "dhcp" ]]; then
			NetIfConf="$ExtIf,dhcp|$IntIf,$IntIP,$IntNetmask"
		else
			NetIfConf="$ExtIf,$ExtIP,$ExtNetmask,$Gateway,$DNS|$IntIf,$IntIP,$IntNetmask"
		fi
	fi
	Q="REPLACE INTO Device_DeviceData(FK_Device,FK_DeviceData,IK_DeviceData) VALUES('$PK_Device',32,'$NetIfConf')"
	RunSQL "$Q"
fi

Q="GRANT ALL PRIVILEGES ON *.* TO 'root'@$IntIP"
RunSQL "$Q"
echo "Writing network configuration with one in database"

/etc/init.d/networking stop

File=/etc/network/interfaces
#cp "$File" "$File.%(date +%F-%T)"
IfConf="auto lo
iface lo inet loopback
"
echo "$IfConf" >"$File"

[[ "$ExtIP" == "dhcp" ]] && Setting="dhcp" || Setting="static"
if [[ "$Setting" == "static" ]]; then
	IfConf="auto $ExtIf
iface $ExtIf inet static
	address $ExtIP
	netmask $ExtNetmask
	gateway $Gateway"
	echo "$IfConf" >>"$File"

	DNSservers=$(echo "$DNS,"; echo "$NetSettings" | tail -1)
	DNSservers=$(echo "$DNSservers" | tr ',' '\n' | sort -u | tr '\n' ' ')
	: >/etc/resolv.conf
	for i in $DNSservers; do
		echo "nameserver $i" >>/etc/resolv.conf
	done
else
	IfConf="auto $ExtIf
iface $ExtIf inet dhcp"
	echo "$IfConf" >>"$File"
fi

IfConf="auto $IntIf
iface $IntIf inet static
	address $IntIP
	netmask $IntNetmask"
echo "$IfConf" >>"$File"

if [[ -n "$DHCPcard" ]]; then
	echo "INTERFACES=\"$DHCPcard\"" >/etc/default/dhcp3-server
elif [[ "$IntIf" != *:* ]]; then
	echo "INTERFACES=\"$IntIf\"" >/etc/default/dhcp3-server
else
	echo "INTERFACES=\"$ExtIf\"" >/etc/default/dhcp3-server
fi

/etc/init.d/networking start
if [[ "$Setting" == static ]]; then
	/usr/pluto/bin/Network_DNS.sh
else
	touch /etc/bind/named.conf.forwarders # Make sure file exists so BIND starts
fi

awk 'BEGIN { Replace = 0 }
/\/\/ forwarders/ {
	Replace = 3;
	print("include \"/etc/bind/named.conf.forwarders\";\n");
}
Replace == 0 { print }
Replace != 0 { Replace-- }
' /etc/bind/named.conf.options >/etc/bind/named.conf.options.$$
mv /etc/bind/named.conf.options.$$ /etc/bind/named.conf.options

if ! grep -qF 'zone "activate.plutohome.com"' /etc/bind/named.conf.local; then
	cat /usr/pluto/templates/named.zone.pluto.activate.local.conf.tmpl >>/etc/bind/named.conf.local
fi
cat /usr/pluto/templates/named.zone.pluto.activate.data.tmpl >"/etc/bind/named.zone.pluto.activate"

CORE_INTERNAL_ADDRESS="$IntIP"
ReplaceVars /etc/bind/named.zone.pluto.activate

rndc reload

AskPort80="
Right now your Pluto Core will only allow you to access from within the local
network in your home. (Technical: All incoming ports are blocked).  

Do you want to access the Pluto web sites from a web browser outside the home
(ie open port 80)?  Note that if you answer 'yes' we recommend you get an SSL
certificate to encrypt the connection, otherwise it would be possible for
someone to get your username and password and gain access to the system. If in
doubt, just answer 'no'.

Allow web access? [y/N] "

AskPort22="
Do you want to allow incoming SSH connections by opening port 22?  This is a
secure way of accessing the system, but is only for techies and Linux users.

Allow SSH connections? [y/N]?"

#clear

#echo -n "$AskPort80"
#read Answer

#if [ "$Answer" == "y" -o "$Answer" == "Y" ]; then
#	echo "Storing your option to open port 80 (allowing web access)"
#	Q="INSERT INTO Firewall(Protocol,SourcePort,RuleType) VALUES('tcp',80,'core_input')"
#	RunSQL "$Q"
#fi

#echo -n "$AskPort22"
#read Answer

#if [ "$Answer" == "y" -o "$Answer" == "Y" ]; then
#	echo "Storing your option to open port 22 (allowing SSH access)"
#	Q="INSERT INTO Firewall(Protocol,SourcePort,RuleType) VALUES('tcp',22,'core_input')"
#	RunSQL "$Q"
#fi

RealExtIP="$ExtIP"
dcerouterIP="$IntIP"
if [[ "$ExtIP" == "dhcp" ]]; then
	RealExtIP=$(ip addr ls "$ExtIf" | grep "inet .*$ExtIf\$" | awk '{print $2}' | cut -d/ -f1)
fi
if [[ -z "$dcerouterIP" ]]; then
	dcerouterIP="$RealExtIP" #dcerouterIP="127.0.0.1"
fi
Q="UPDATE Device SET IPaddress='$dcerouterIP' WHERE PK_Device='$PK_Device'"
RunSQL "$Q"

hosts="
127.0.0.1       localhost.localdomain   localhost
$dcerouterIP	dcerouter $(/bin/hostname)
#%MOON_HOSTS%

# The following lines are desirable for IPv6 capable hosts
::1     ip6-localhost ip6-loopback
fe00::0 ip6-localnet
ff00::0 ip6-mcastprefix
ff02::1 ip6-allnodes
ff02::2 ip6-allrouters
ff02::3 ip6-allhosts
"

echo "$hosts" >/etc/hosts

Q="FLUSH PRIVILEGES"
RunSQL "$Q"


## Setup smb.conf.pluto
DD_Domain=187
DD_ComputerName=188

Q="SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_DeviceData=$DD_Domain AND FK_Device=$PK_Device"
DomainName=$(RunSQL "$Q")
DomainName=$(Field "1" "$DomainName")

Q="SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_DeviceData=$DD_ComputerName AND FK_Device=$PK_Device"
ComputerName=$(RunSQL "$Q")
ComputerName=$(Field "1" "$ComputerName")

echo "
[global]
	workgroup = $DomainName
	server string =	$ComputerName
	netbios name = $ComputerName
" > /etc/samba/smb.conf.pluto

# This code here - for safe keeping - has something to do with PPPoE and PMTU
#iptables -A FORWARD -o ppp0 -p tcp -m tcp --tcp-flags SYN,RST SYN -m tcpmss --mss 1400:1536 -j TCPMSS --clamp-mss-to-pmtu
