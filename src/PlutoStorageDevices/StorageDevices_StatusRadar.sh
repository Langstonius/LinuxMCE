#!/bin/bash

if [[ $1 != "background" ]] ;then
        echo "Backgrounding ..."
        screen -d -m -S "StorageStatusRadar" "$0" "background"
        exit 0
fi


. /usr/pluto/bin/SQL_Ops.sh 2>/dev/null || exit 1
. /usr/pluto/bin/Config_Ops.sh 2>/dev/null || exit 1

TPL_BUFFALO=1794
TPL_FILESERVER=1837
TPL_WINDOWS_SHARE=1768
TPL_NFS_SHARE=1769
TPL_INTERNAL_DRIVE=1790
TPL_RAID_0=1854
TPL_RAID_1=1851
TPL_RAID_5=1849
TPL_MEDIA_PLUGIN=2

CMD_REFRESH_LIST_OF_ONLINE_DEVICES=831

DD_SHARE_NAME=126
DD_USERNAME=127
DD_PASSWORD=128
DD_ONLINE=195
DD_BLOCK_DEV=152
DD_READONLY=194

EV_User_Password_Mismatch=70

EVP_PK_Device=26
EVP_Comment=58

function SetDeviceOnline() {
	local Device_ID=$1
	local OnlineValue=$2
	local Q=""
	
	Q="SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_Device = '$Device_ID' AND FK_DeviceData='$DD_ONLINE'"
	OldValue=$(RunSQL "$Q")
	OldValue=$(Field "1" "$OldValue")

	if [[ "$OldValue" != "$OnlineValue" ]] ;then

		Q="UPDATE Device_DeviceData SET IK_DeviceData = '$OnlineValue' WHERE FK_Device = '$Device_ID' AND FK_DeviceData= '$DD_ONLINE'"
		RunSQL "$Q"

		if [[ $OnlineValue == "0" ]] ;then
			umount -lf /mnt/device/$Device_ID 2>/dev/null 1>/dev/null
		fi

		/usr/pluto/bin/MessageSend "$DCERouter" -targetType template $Device_ID $TPL_MEDIA_PLUGIN 1 $CMD_REFRESH_LIST_OF_ONLINE_DEVICES	

		Log "----------------- DEVICE $Device_ID $OnlineValue"		
	else
	
		Log "----------------- DEVICE $Device_ID $OnlineValue (unchanged)"	
	fi
}

function SetKidsOnline() {
	local Device_ID=$1
	local OnlineValue=$2
	local Q=""
	local R=""
	local Device_Child=""
	
	Q="SELECT PK_Device FROM Device WHERE FK_Device_ControlledVia = '$Device_ID'"
	R=$(RunSQL "$Q")
	for Device_Child in $R ;do
		Device_Child=$(Field 1 "$Device_Child")
		SetDeviceOnline "$Device_Child" "$OnlineValue"
	done
}

function Log() {
	local Message=$1
	
	echo "$(date -R) $Message"
}

while : ;do
	## Get a list with all the File Servers from the database
	Q="
		SELECT 
			Device.PK_Device, 
			Device.IPaddress,
			Username.IK_DeviceData,
			Password.IK_DeviceData
		FROM 
			Device
			LEFT JOIN Device_DeviceData Username ON Username.FK_Device = Device.PK_Device AND Username.FK_DeviceData = '$DD_USERNAME'
			LEFT JOIN Device_DeviceData Password ON Password.FK_Device = Device.PK_Device AND Password.FK_DeviceData = '$DD_PASSWORD'
		WHERE 
			Device.FK_DeviceTemplate IN ($TPL_BUFFALO, $TPL_FILESERVER)
			
	"
	FileServerList=$(RunSQL "$Q")

	## Test each file server
	for FileServer in $FileServerList ;do 
		Device_ID=$(Field 1 "$FileServer")
		Device_IP=$(Field 2 "$FileServer")
		Device_Username=$(Field 3 "$FileServer")
		Device_Password=$(Field 4 "$FileServer")
		
		if [[ "$Device_IP" == "" ]] ;then
			SetKidsOnline "$Device_ID" "0"
			Log "Device $Device_ID has no IP address associated with it."
			continue
		fi

		## Test to see if the file server is online or not
		ping -qnc 1 -W 1 "$Device_IP" &> /dev/null
		HostIsUp=$?
		if [[ "$HostIsUp" != "0" ]] ;then
			SetKidsOnline "$Device_ID" "0"
			Log "Device $Device_ID ($Device_IP) doesn't respond to our ping."
			continue
		fi

		
		Log "Device $Device_ID ($Device_IP) is online trying each of samba kids to see it they are still available"
		## Get a list of samba shares related to this device
		Q="
		SELECT 
			PK_Device,
			ShareName.IK_DeviceData,
			Username.IK_DeviceData,
			Password.IK_DeviceData
		FROM
			Device
			LEFT JOIN Device_DeviceData ShareName ON ShareName.FK_Device = Device.PK_Device AND ShareName.FK_DeviceData = '$DD_SHARE_NAME'
			LEFT JOIN Device_DeviceData Username ON Username.FK_Device = Device.PK_Device AND Username.FK_DeviceData = '$DD_USERNAME'
			LEFT JOIN Device_DeviceData Password ON Password.FK_Device = Device.PK_Device AND Password.FK_DeviceData = '$DD_PASSWORD'
		WHERE
			FK_Device_ControlledVia = '$Device_ID'
			AND
			FK_DeviceTemplate = '$TPL_WINDOWS_SHARE'
		"
		SambaShareList=$(RunSQL "$Q")

		## Test each samba share to see if is available or not
		for SambaShare in $SambaShareList ;do
			
			if [[ "$SambaShare" == "" ]] ;then
				continue
			fi
			
			Share_ID=$(Field "1" "$SambaShare")
			Share_Name=$(Field "2" "$SambaShare")
			Share_Username=$(Field "3" "$SambaShare")
			Share_Password=$(Field "4" "$SambaShare")

			## Test if the share is still in the list
			smbclient -U $Device_Username%$Device_Password --list=//$Device_IP --grepable 2>/dev/null | grep "^Disk" | cut -d'|' -f2 | grep -q "^${Share_Name}$"
			isShareInList=$?
			
			if [[ "$isShareInList" != "0" ]] ;then
				Log "Share $Share_ID ($Share_Name) is not advertised by the smb server"
				SetDeviceOnline "$Share_ID" "0"
				continue
			fi

			## Test if the share is still mountable with the username/password that we have
			smbclient -U $Share_Username%$Share_Password "//$Device_IP/$Share_Name" -c 'ls' 1>/dev/null 2>/dev/null
			isShareMountable=$?
			
			if [[ "$isShareMountable" != "0" ]] ;then
				Msg="Share $Share_ID ($Share_Name) is not mountable with the username/pass that i have"
				Log "$Msg"
                /usr/pluto/bin/MessageSend "$DCERouter" 0 -1001 2 "$EV_User_Password_Mismatch" "$EVP_PK_Device" "$Share_ID" "$EVP_Comment" "$Msg"
				SetDeviceOnline "$Share_ID" "0"
				continue
			fi

			SetDeviceOnline "$Share_ID" "1"
		done


		## Get a list of nfs shares related to this device
		Q="
                SELECT 
                        PK_Device,
                        ShareName.IK_DeviceData,
                        Username.IK_DeviceData,
                        Password.IK_DeviceData
                FROM
                        Device
                        LEFT JOIN Device_DeviceData ShareName ON ShareName.FK_Device = Device.PK_Device AND ShareName.FK_DeviceData = '$DD_SHARE_NAME'
                        LEFT JOIN Device_DeviceData Username ON Username.FK_Device = Device.PK_Device AND Username.FK_DeviceData = '$DD_USERNAME'
                        LEFT JOIN Device_DeviceData Password ON Password.FK_Device = Device.PK_Device AND Password.FK_DeviceData = '$DD_PASSWORD'
                WHERE
                        FK_Device_ControlledVia = '$Device_ID'
                        AND
                        FK_DeviceTemplate = '$TPL_NFS_SHARE'
                "
		NFSShareList=$(RunSQL "$Q")

		## Test each nfs share to see if is available on not
		for NFSShare in $NFSShareList ;do
			if [[ "$NFSShare" == "" ]] ;then 
				continue
			fi
			
			Share_ID=$(Field "1" "$NFSShare")
                        Share_Name=$(Field "2" "$NFSShare")
                        Share_Username=$(Field "3" "$NFSShare")
                        Share_Password=$(Field "4" "$NFSShare")
								
			## Test if the share is still in the list
			showmount -e $Device_IP | cut -d ' ' -f1 | grep "^${ShareName}$"
			isShareInList=$?

			if [[ "$isShareInList" != "0" ]] ;then
				Log "Share $Share_ID ($Share_Name) is not advertised by the nfs server"
				SetDeviceOnline "$Share_ID" "0"
				continue
			fi

                        ## See if the share is mountable
                        mountDirTemp=$(mktemp -d)
                        mount ${Device_ID}:${Share_Name} $mountDirTemp 1>/dev/null 2>/dev/null
                        isShareMountable=$?

                        if [[ "$isShareMountable" != "0" ]] ;then
                                Log "Share $Share_ID ($Share_Name) cannot be mounted"
                                SetDeviceOnline "$Share_ID" "0"
				rm -f $mountDirTemp
				continue
                        else
                                umount -lf $mountDirTemp 1>/dev/null 2>/dev/null
                                rm -r $mountDirTemp
                        fi

			SetDeviceOnline "$Share_ID" "1"
		done
	done

	## Get a list of internal drives
	Q="
		SELECT
			PK_Device,
			FK_Device_ControlledVia,
			BlockDev.IK_DeviceData
		FROM
			Device
			LEFT JOIN Device_DeviceData BlockDev ON BlockDev.FK_Device = Device.PK_Device AND BlockDev.FK_DeviceData = '$DD_BLOCK_DEV'
		WHERE
			FK_DeviceTemplate IN ($TPL_INTERNAL_DRIVE, $TPL_RAID_0, $TPL_RAID_1, $TPL_RAID_5)
	"
	InternalDriveList=$(RunSQL "$Q")
	
	## Test each internal drive and see if is available
	for InternalDrive in $InternalDriveList ;do
		if [[ "$InternalDrive" == "" ]] ;then
			continue;
		fi

		IDrive_ID=$(Field "1" "$InternalDrive")
		IDrive_Parent=$(Field "2" "$InternalDrive")
		IDrive_BlockDev=$(Field "3" "$InternalDrive")

		## Is one of our internal drives
		if [[ "$IDrive_Parent" == "$PK_Device" ]] ;then
			
			## See if is still available in /proc/partitions
			cat /proc/partitions | grep -q "${IDrive_BlockDev##/dev/}$"
			isDriveAvailable=$?
			
			if [[ "$isDriveAvailable" != "0" ]] ;then
				Log "Drive $IDrive_ID ($IDrive_BlockDev) cannot be found in /proc/partitions"
				SetDeviceOnline "$IDrive_ID" "0"
				continue
			fi

			## See if the drive is mountable
			mountDirTemp=$(mktemp -d)			
			mount ${IDrive_BlockDev} $mountDirTemp 1>/dev/null 2>/dev/null
			isDriveMountable=$?
			
			if [[ "$isDriveMountable" != "0" ]] ;then
				Log "Drive $IDrive_ID ($IDrive_BlockDev) cannot be mounted"
				SetDeviceOnline "$IDrive_ID" "0"
				rm -r $mountDirTemp
				continue
			else
				umount -lf $mountDirTemp 1>/dev/null 2>/dev/null
				rm -r $mountDirTemp
			fi

			SetDeviceOnline "$IDrive_ID" "1"
			

		## Is a internal drive located on a remote computer
		else

			## Get the ip of the parent device
			IDrive_IP=$(RunSQL "SELECT IPaddress FROM Device WHERE PK_Device='${IDrive_Parent}'")
			IDrive_IP=$(Field "1" "$IDrive_IP")

                        ## Test to see if the parent is online or not
			ping -qnc 1 -W 1 "$IDrive_IP" &> /dev/null
			HostIsUp=$?
			if [[ "$HostIsUp" != "0" ]] ;then
				SetKidsOnline "$IDrive_Parent" "0"
				Log "Device $IDrive_Parent ($IDrive_IP) doesn't respond to our ping."
				continue
			fi

                        ## Test if the share is still in the list
                        smbclient -A /usr/pluto/var/sambaCredentials.secret --list=//$IDrive_IP --grepable 2>/dev/null | grep "^Disk" | cut -d'|' -f2 | grep -q "^Storage$IDrive_ID\$$"
                        isShareInList=$?

                        if [[ "$isShareInList" != "0" ]] ;then
                                Log "Drive $IDrive_ID ($IDrive_BlockDev) is not advertised by it's parent smb server ($IDrive_IP)"
                                SetDeviceOnline "$IDrive_ID" "0"
                                continue
                        fi

                        ## Test if the share is still mountable with the username/password that we have
                        smbclient -A /usr/pluto/var/sambaCredentials.secret "//$IDrive_IP/Storage$IDrive_ID\$" -c 'ls' 1>/dev/null 2>/dev/null
                        isShareMountable=$?

                        if [[ "$isShareMountable" != "0" ]] ;then
                                Msg="Drive $IDrive_ID ($IDrive_IP $IDrive_BlockDev) cannot be mounted"
                                Log "$Msg"
                                SetDeviceOnline "$IDrive_ID" "0"
                                continue
                        fi

                        SetDeviceOnline "$IDrive_ID" "1"
		fi
	done
	

	sleep 5
done
