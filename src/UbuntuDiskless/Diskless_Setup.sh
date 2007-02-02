#!/bin/bash

. /usr/pluto/bin/Network_Parameters.sh
. /usr/pluto/bin/Config_Ops.sh
. /usr/pluto/bin/Section_Ops.sh
. /usr/pluto/bin/LockUtils.sh
. /usr/pluto/bin/Diskless_Utils.sh


DEVICEDATA_Extra_Parameters=139
DEVICEDATA_Extra_Parameters_Override=140
DEVICEDATA_Architecture=112
DEVICEDATA_DisklessBoot=9


function setup_tftp_boot 
{
	#Moon_MAC

	local Moon_BootConfFile="/tftpboot/pxelinux.cfg/01-$(echo ${Moon_MAC//:/-} | tr 'A-Z' 'a-z')"
	local BootConf=""

	local BootParams="quiet apicpmtimer noirqdebug"
	local BootParams_Extra=$(RunSQL "SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_Device = $Moon_DeviceID AND FK_DeviceData = $DEVICEDATA_Extra_Parameters")
	local BootParams_Override=$(RunSQL "SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_Device = $Moon_DeviceID AND FK_DeviceData = $DEVICEDATA_Extra_Parameters_Override")
	if [[ "$BootParams_Override" == "1" ]] ;then
		BootParams="${BootParams} ${BootParams_Extra}"
	else
		BootParams="${BootParams_Extra}"
	fi


	mkdir -p /tftpboot/pxelinux.cfg

	BootConf="${BootConf}DEFAULT Pluto\n"
	BootConf="${BootConf}LABLE Pluto\n"
	BootConf="${BootConf}KERNEL ${Moon_DeviceID}/vmlinuz\n"
	BootConf="${BootConf}APPEND ${Moon_DeviceID}/initrd ramdisk=10240 rw boot=nfs nfsroot=${IntIP}:/usr/pluto/diskless/${Moon_DeviceID} ${BootParams_Extra}\n"
	
	echo -e "$BootConf" > "$Moon_BootConfFile"
}


function setup_mysql_access 
{
	RunSQL "GRANT ALL PRIVILEGES ON *.* TO 'root'@$Moon_IP; GRANT ALL PRIVILEGES ON *.* TO 'eib'@$Moon_IP"
	RunSQL "FLUSH PRIVILEGES"
}


function generate_diskless_installer 
{
	
	## Copy installer files
	mkdir -p $Moon_RootLocation/usr/pluto/install
	Files="Common.sh ConfirmDependencies_Debian.sh Initial_Config_MD.sh Initial_Config_Finish.sh ramdisk.tar.bz2"
	for Stuff in $Files; do
		cp /usr/pluto/install/$Stuff $Moon_RootLocation/usr/pluto/install
	done

	## Generate another installer script
	if [[ ! -f $Moon_RootLocation/usr/pluto/install/activation.sh ]]; then
		/usr/pluto/bin/ConfirmDependencies -r -D pluto_main -h dcerouter -u root -p '' -d $Moon_DeviceID install > $Moon_RootLocation/usr/pluto/install/activation.sh
	fi
	
	## Modify a install script to run as for diskless
	sed '/^Type=/ s/^.*$/Type="diskless"/' /usr/pluto/install/Initial_Config.sh >$Moon_RootLocation/usr/pluto/install/Initial_Config.sh
	chmod +x $Moon_RootLocation/usr/pluto/install/Initial_Config.sh
	mkdir -p $Moon_RootLocation/usr/pluto/deb-cache
}


function setup_hosts_file 
{
	local Content=""
	local Q="
		SELECT 
			PK_Device, 
			IPaddress
		FROM 
			Device
			JOIN Device_DeviceData ON PK_Device = FK_Device AND FK_DeviceData = $DEVICEDATA_DisklessBoot
		WHERE 
			FK_DeviceTemplate = '28'
			AND
			FK_Device_ControlledVia IS NULL
			AND
			IK_DeviceData = '1'
	"
	local R=$(RunSQL "$Q")

	for Row in $R ;do
		local DeviceID=$(Field 1 "$Row")
		local IP=$(Field 2 "$Row")

		if [[ "$IP" != "" ]] ;then
			continue
		fi

		Content="${Content}${IPaddress}		moon${DeviceID}\n"
	done
	
	PopulateSection "/etc/hosts" "DisklessMD" "$Content"
}


## Parse parameters
SkipLock=n
for ((i = 1; i <= "$#"; i++)); do
        Parm="${!i}"
        case "$Parm" in                 --skiplock) SkipLock=y
        esac
done


## Aquire Lock
if [[ "$SkipLock" == n ]]; then
        if ! TryLock "Diskless_Setup" "Diskless_Setup"; then
                echo "Another Diskless-related operation is running"
                exit 1
        fi
        trap 'Unlock "Diskless_Setup" "Diskless_Setup"' EXIT
fi


## Check for valid DHCP settings else exit
if [ -z "$DHCPsetting" ]; then
	echo "Diskless MDs can't exist in the current setup (no DHCP). Not setting up any."
	exit
fi


## Getting a list of Media Directors
echo "Getting list of Media Directors"
Q="
	SELECT 
		PK_Device, 
		IPaddress, 
		MACaddress, 
		Device.Description,
		Device.NeedConfigure
	FROM 
		Device
		JOIN Device_DeviceData ON PK_Device = FK_Device AND FK_DeviceData = $DEVICEDATA_DisklessBoot
	WHERE 
		FK_DeviceTemplate = '28'
		AND
		FK_Device_ControlledVia IS NULL
		AND
		IK_DeviceData = '1'
"
R=$(RunSQL "$Q")

## Processing Moons
for Row in $R; do
	## Gathering information about this moon
	Moon_DeviceID=$(Field 1 "$Row")

	Moon_IP=$(Field 2 "$Row")
	Moon_IP=$(/usr/pluto/bin/PlutoDHCP.sh -d "$Moon_DeviceID" -a)
	if [[ -z "$Moon_IP" ]]; then
		echo "WARNING : No free IP left to assign for moon$Moon_DeviceID"
		continue
	fi
	
	Moon_MAC=$(Field 3 "$Row")
	Moon_MAC=$(echo ${Moon_MAC//-/:} | tr 'a-z' 'A-Z')
	if ! /usr/pluto/bin/CheckMAC.sh "$Moon_MAC" ;then
		echo "WARNING : Mac ($Moon_MAC) of moon$Moon_DeviceID is invalid"
		continue
	fi

	Moon_Description=$(Field 4 "$Row")
	Moon_NeedConfigure=$(Field 5 "$Row")
	if [[ "$NeedConfigure" != "1" ]] ;then
		echo "INFO : Skiping moon$Moon_DeviceID because NeedConfigure flag is not set"
	fi

	Moon_Architecture=$(RunSQL "SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_Device='$Moon_DeviceID' AND FK_DeviceData='$DEVICEDATA_Architecture'")
	if [[ "$Moon_Architecture" == "" ]] ;then
		Moon_Architecture="686"
	fi

	Moon_RootLocation="/usr/pluto/diskless/$Moon_DeviceID"
	Moon_BootConfFile="/tftpboot/pxelinux.cfg/01-$(echo ${Moon_MAC//:/-} | tr 'A-Z' 'a-z')"

	
	## Adding moon to hosts (/etc/hosts)
	hosts_DisklessMD="{$hosts_DisklessMD}${Moon_IP}	moon${Moon_DeviceID}\n"

	## Create the a filesystem for this MD
	/usr/pluto/bin/Diskless_CreateFS.sh "$Moon_IP" "$Moon_MAC" "$Moon_DeviceID" "$Moon_Architecture"


	## Setting Up
	setup_tftp_boot
	setup_mysql_access
	generate_diskless_installer

	## Create /var/log/pluto for this device as a symlink
	mkdir -p "/home/logs/diskless_$Moon_DeviceID"
	if [[ -d $Moon_RootLocation/var/log/pluto	]] ;then
		mv -r $Moon_RootLocation/var/log/pluto/* /home/log/diskless_$Moon_DeviceID
		rm -rf $Moon_RootLocation/var/log/pluto/
	fi
	if [[ ! -e "$Moon_RootLocation"/var/log/pluto ]]; then
		ln -s "/home/logs/diskless_$Moon_DeviceID" "$Moon_RootLocation/var/log/pluto"
	fi

	## Dome configuring this MD
	RunSQL "UPDATE Device SET NeedConfigure = 0 WHERE PK_Device=$Moon_DeviceID"
done

setup_hosts_file


#/usr/pluto/bin/Update_StartupScrips.sh
echo "Finished setting up network boot for media directors."
echo "If new media director(s) were added, do a quick reload router."

