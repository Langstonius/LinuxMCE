#!/bin/bash

trap true SIGINT

Width="$1"
Height="$2"
Refresh="$3"
Type="$4"

while [[ "$#" -gt 0 ]]; do
	case "$1" in
		"-f") Force=yes ;;
	esac
	shift
done

ShowDialog()
{
	local pidOfX="$1"
	local Msg="$2"

	DISPLAY=:1 xsetbg -tile /usr/pluto/share/resolutions.png
	DISPLAY=:1 Xdialog --ignore-eof --cancel-label "Close" --infobox "$Msg" 0x0 0
	kill "$pidOfX"
}

ModeName="\"${Width}x${Height}\""
Modeline="\"${Width}x${Height}\"$(/usr/pluto/bin/xtiming.pl "$Width" "$Height" "$Refresh" "$Type" | cut -d' ' -f2-)"

awk -v "Force=$Force" '
	BEGIN { Monitor = 0; Display = 0; }
	/Modeline/ || /Modes/ { next }
	/Section..*"Monitor"/ { print; Monitor = 1; next; }
	Monitor == 1 && (/HorizSync/ || /VertRefresh/) { next; }
	/EndSection/ && Monitor == 1 {
		print "\tModeline '"${Modeline//\"/\\\"}"'";
		if (Force == "yes") {
			print "\tHorizSync 28-500";
		}
		else
		{
			print "\tHorizSync 28-90";
		}
		print "\tVertRefresh '"$Refresh"'";
		print;
		Monitor = 0;
		next;
	}
	/SubSection..*"Display"/ { print; Display = 1; next; }
	/EndSubSection/ && Display == 1 { print "\t\tModes '"${ModeName//\"/\\\"}"'"; print; Display = 0; next; }
	{ print }
' /etc/X11/XF86Config-4 >/etc/X11/XF86Config-4.test

X :1 -ac -xf86config /etc/X11/XF86Config-4.test &
pidOfX=
Timeout=5
while [[ -z "$pidOfX" && $Timeout > 0 ]]; do
	pidOfX="$(ps ax|grep 'X :1 -ac -xf86config /etc/X11/XF86Config-4.test'|grep -v grep|awk '{print $1}')"
	((Timeout--))
	sleep 1
done

if [[ -z "$pidOfX" ]]; then
	echo "X didn't start"
	exit 10
fi

setting="$(xrandr -d :1 -q|grep '^\*')"
resolution="${setting:5:11}"
refresh="${setting:39:4}"
#DISPLAY=:1 Xdialog --default-no --yesno "Current resolution: $resolution @ $refresh Hz\nIs this OK?" 0x0
Msg="Current resolution: $resolution @ $refresh Hz"
echo "$Msg"
ShowDialog "$pidOfX" "$Msg" &

disown -a
exit 0
