#!/bin/bash
. /usr/pluto/bin/pluto.func

XClient=/usr/bin/icewm-session
XClientParm=()
XServerParm=(-logverbose 5)
Background=y

for ((i = 1; i <= "$#"; i++)); do
	case "${!i}" in
		-client) ((i++)); XClient=${!i} ;;
		-parm) ((i++)); XClientParm=("${XClientParm[@]}" ${!i}) ;;
		-fg) Background=n ;;
		-srvparm) ((i++)); XServerParm=("${XServerParm[@]}" ${!i});;
	esac
done

Logging "$TYPE" "$SEVERITY_NORMAL" "$0" "Starting X server (client: $XClient; parms: ${XClientParm[*]})"

# Start X11
if [[ "$Background" == y ]]; then
	screen -d -m -S XWindowSystem xinit "$XClient" "${XClientParm[@]}" -- :0 -ac -allowMouseOpenFail vt7 "${XServerParm[@]}"
	# Start everouter for gyration mouse
	#if [[ -x /usr/pluto/bin/StartGyrationEvrouter.sh ]]; then
	#	screen -d -m -S GyrationMouse /usr/pluto/bin/StartGyrationEvrouter.sh
	#fi
	sleep 1
	UI_Version=$(/usr/pluto/bin/X-WhichUI.sh)
	if [[ "$UI_Version" == 2 ]]; then
		DISPLAY=:0 /usr/bin/xcompmgr -c &
		disown -a
	fi
else
	xinit "$XClient" "${XClientParm[@]}" -- :0 -ac -allowMouseOpenFail vt7 "${XServerParm[@]}"
fi
