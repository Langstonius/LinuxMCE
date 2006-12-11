#!/bin/bash

function isInList 
{
	local disk="$1"
	for dev in $availPart; do
		if [[ "$disk" == "$dev" ]]; then 
			echo '0'
		fi
 	done
	echo '1'
}

function substractParts {
    local availPart=$1
    local unavailPart=$2
    local retPart=""

    for aPart in $availPart ;do
        available=1
        for uPart in $unavailPart ;do
	    if [[ ${#aPart} == '3' ]]; then 
		len=$((${#uPart}-1))
                uPart=${uPart:0:$len}	
	    fi
			
            if [[ "$aPart" == "$uPart" ]]; then
               available=0
            fi
        done
        if [[ $available == 1 ]]; then
            retPart="$retPart $aPart"
        fi
    done
    echo $retPart
}
														    

## Available partitions
availPart=$(cat /proc/partitions | awk '/(sd|hd)/ { print $4 }')

## Remove Mounted partitions
mountedPart=$(mount | awk '/dev\/(sd|hd)/ {print $1}' | sed 's/\/dev\///g')
availPart=$(substractParts "$availPart" "$mountedPart")

## Remove Unmountable partitions (swap / extended)
auxPart=""
for part in $availPart ;do
     ## If is extended partition
     if file -s /dev/$part | grep -q "extended partition table$"  ;then
        continue
     fi

    ## If is swap partition
    if file -s /dev/$part | grep -q " Linux/i386 swap file"  ;then
    	continue
    fi
    auxPart="$auxPart $part"
done
availPart=$auxPart
																	     
for device in $availPart; do
	if [[ ${#device} == '3' ]]; then
		new_list="$new_list""$device "
	else
		len=$((${#device}-1))
		device=${device:0:$len}
		if [[ "$(isInList "$device")" == '1' ]]; then
			new_list="$new_list""$device "
		fi
	fi
done

for device in $new_list; do
	result=$(mdadm --examine --brief --scan /dev/$device)
	#if [[ -n $result ]]; then 
		#sunt hdd-uri care fac parte din array
	#else
	if [[ -z $result ]]; then
		result=$(mdadm --query /dev/$device | grep 'is not an md array')
		if [[ -n $result ]]; then
			size=$(fdisk -l /dev/$device | grep Disk | cut -d' ' -f3)
			free_disks="$free_disks/dev/$device,$size MB;"
		fi
	fi
done

echo $free_disks
