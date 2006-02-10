#!/bin/bash

. /usr/pluto/bin/Config_Ops.sh
. /usr/pluto/bin/SQL_Ops.sh
. /usr/pluto/bin/Utils.sh
. /usr/pluto/bin/PlutoVersion.h

Q="SELECT FK_DeviceCategory FROM Device JOIN DeviceTemplate ON FK_DeviceTemplate=PK_DeviceTemplate WHERE PK_Device=$PK_Device"
DeviceCategory=$(RunSQL "$Q")

DeviceCategory_Core=7
DeviceCategory_MD=8

if [[ -n "$DeviceCategory" ]]; then
	if [[ $DeviceCategory -eq $DeviceCategory_MD ]]; then
#		if ! update-rc.d -f discover remove; then
#			:
#		fi
#		update-rc.d discover start 80 1 2 3 4 5 .
#
#		if ! update-rc.d -f hotplug remove; then
#			:
#		fi
#		update-rc.d hotplug start 81 1 2 3 4 5 . stop 89 0 6 . || /bin/true
		# Diskless machines should not drop portmap or networking at any point
		rm -f /etc/rc{0,6}.d/{S32portmap,S35networking}
	fi
fi

PrevVer="$2"
DeviceCategory_VideoCards=125

if [[ -n "$PrevVer" ]]; then
	echo "Upgrading from version '$PrevVer'. Not setting up X again"
	exit 0
elif [[ -n "$(FindDevice_Category "$PK_Device" "$DeviceCategory_VideoCards")" ]]; then
	echo "Child device in 'Video Cards' category found. It will configure X, not us"
	exit 0
else
	echo "Configuring X (using X autoconfiguration mechanism)"
	config=$(/usr/bin/X11/X -configure 2>&1 | grep 'Your XF86Config file' | cut -d" " -f5-)
	retcode=$?
	if [[ "$retcode" -ne 0 || -z "$config" || ! -e "$config" ]]; then
		echo "Something went wrong while configuring X."
		exit 1
	fi

	[[ -e /etc/X11/XF86Config-4 ]] && mv /etc/X11/XF86Config-4 /etc/X11/XF86Config-4.orig
	awk '
{ print }
/Monitor.*Monitor0/ { print("\tDefaultDepth\t24") }
/Depth.*24/ { print("\t\tModes\t\"800x600\"") }
' "$config" >/etc/X11/XF86Config-4.new
	echo 'Section "ServerFlags"
	Option "AllowMouseOpenFail" "true"
EndSection' >>/etc/X11/XF86Config-4.new

	EnableWheel='
BEGIN {
	axis = 0;
	text = "";
	section = 0;
}

/Section "InputDevice"/,/EndSection/ {
	section = 1;
	if (match($0, /Driver.*mouse/))
		axis = 1;
	if (match($0, /ZAxisMapping/))
		axis = 0;
	if (match($0, /EndSection/) && axis == 1)
	{
		text = text "Option \"ZAxisMapping\" \"4 5\"\n";
		axis = 0;
	}
	text = text $0 "\n";
	next;
}

section == 1 {
	printf "%s", text;
	text = "";
	section = 0;
}

{ print }'

	egrep -v 'Load.*"dri"|Load.*"glx"' /etc/X11/XF86Config-4.new | awk "$EnableWheel" >/etc/X11/XF86Config-4
	rm -f "$config"

	sed -i 's!/dev/mouse!/dev/input/mice!g' /etc/X11/XF86Config-4
	# only on standalone MDs, not hybrids
	if ! PackageIsInstalled pluto-dcerouter; then
		sed -i 's/^NTPSERVERS=.*$/NTPSERVERS="dcerouter"/' /etc/default/ntpdate
	fi
fi
