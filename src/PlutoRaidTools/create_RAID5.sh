#!/bin/bash

. /usr/pluto/bin/SQL_Ops.sh

Device=$1
RAID5_DEVICE_TEMPLATE=1849
NEW_ADD_ID=204
HARD_DRIVE_DEVICE_TEMPLATE=1850
BLOCK_DEVICE_ID=152
SPARE_ID=202
DISK_SIZE_ID=201

Q="SELECT IK_DeviceData FROM Device_DeviceData WHERE FK_Device = $Device and FK_DeviceData = $NEW_ADD_ID"
NewAdd=$(RunSQL "$Q")
name=$(/usr/pluto/bin/get_RAID_name.sh)
name="/dev/$name"
Q="UPDATE Device_DeviceData SET IK_DeviceData='$name' WHERE FK_Device = $Device and FK_DeviceData = $BLOCK_DEVICE_ID"
RunSQL "$Q"
#if is new added get the list of active drives and spares
if [[ $NewAdd == 0 ]] ;then
	Q="SELECT PK_Device FROM Device WHERE FK_DeviceTemplate = $HARD_DRIVE_DEVICE_TEMPLATE AND FK_Device_ControlledVia = $Device"
	HardDriveList=$(RunSQL "$Q")
	ActiveDrives=
	SpareDrives=
	NrDrives=
	NrSpareDrives=
	for Drive in $HardDriveList; do
		Q="
			SELECT 
				Block.IK_DeviceData, 
				Spare.IK_DeviceData 
			FROM Device 
				JOIN Device_DeviceData Block ON Block.FK_Device = PK_Device AND Block.FK_DeviceData = $BLOCK_DEVICE_ID
				JOIN Device_DeviceData Spare ON Spare.FK_Device = PK_Device AND Spare.FK_DeviceData = $SPARE_ID
			WHERE 
				PK_Device = $Drive;
		"
		R=$(RunSQL "$Q")
		Disk=$(Field 1 "$R")
		IsSpare=$(Field 2 "$R")
		if [[ $IsSpare == 1 ]] ;then
			SpareDrives="$SpareDrives "$Disk
			NrSpareDrives=$(($NrSpareDrives+1))
		else
			ActiveDrives="$ActiveDrives "$Disk
			NrDrives=$(($NrDrives+1))
		fi
	done
	#NrDrives=$(($NrDrives+1))
	#create the array with mdadm
	rm -f "/dev/.static$name"
	if [[ $NrSpareDrives > 0 ]] ;then
		echo "y" | mdadm --create $name --auto --level=5 --spare-devices=$NrSpareDrives $SpareDrives --raid-devices=$NrDrives $ActiveDrives
	else	
		echo "y" | mdadm --create $name --auto --level=5 --raid-devices=$NrDrives $ActiveDrives

	fi
	#/usr/pluto/bin/start_RAID_monitoring.sh $name

	sleep 5
	raidSize=$(mdadm --query $name | head -1 |cut -d' ' -f2)
	Q="UPDATE Device_DeviceData SET IK_DeviceData = '$raidSize' WHERE FK_Device = $Device and FK_DeviceData = $DISK_SIZE_ID"
	RunSQL "$Q"
	## Format the new raid
	echo "y" | mkfs.ext3 $name

	## Create a pluto directory structure (depending on UsePlutoDirStructure device data)
	/usr/pluto/bin/StorageDevices_PlutoDirStructure.sh -d $Device
    /usr/pluto/bin/StorageDevices_Setup.sh
			
fi
