#!/bin/bash

. /usr/pluto/bin/pluto.func
. /usr/pluto/bin/Config_Ops.sh
. /usr/pluto/bin/Utils.sh

resHD480=848x480
resHD720=1280x720
resHD1080=1920x1080
resHD576=720x576 # SDTV actually, EDTV with doublescan, but nVidia calls it HD so...

TestConfig()
{
	X :2 -probeonly -config "$ConfigFile" &>/dev/null
	return $?
}

GenModeline()
{
	local ResX="$1" ResY="$2" Refresh="$3" ScanType="$4"
	local Modeline
	Modeline=$(/usr/pluto/bin/xtiming.pl "$ResX" "$ResY" "$Refresh" "$ScanType")
	Modeline="${Modeline/@*\"/\"}"
	echo "$Modeline"
}

OrigParams=("$@")
ConfigFile="/etc/X11/xorg.conf"
while [[ $# -gt 0 ]]; do
	case "$1" in
		--defaults) Defaults=y ;;
		--resolution)
			Resolution="$2"
			if [[ "$Resolution" == *@* ]]; then
				Refresh=${Resolution##*@}
				Resolution=${Resolution%@*}
			else
				Refresh=60
			fi
			
			ResX=${Resolution%%x*}
			ResY=${Resolution#*x}
			if [[ "$ResY" == *i ]]; then
				ResY="${ResY%i}"
				Resolution="${Resolution%i}"
				ScanType="interlace"
			elif [[ "$ResY" == *p ]]; then
				ResY="${ResY%p}"
				Resolution="${Resolution%p}"
				ScanType=
			fi
			
			if [[ -z "$ResX" || -z "$ResY" || -z "$Refresh" || "$ResX" == *[![:digit:]]* || "$ResY" == *[![:digit:]]* || "$Refresh" == *[![:digit:]]* ]]; then
				echo "Mismatched resolution '$Resolution@$Refresh'. Use format: <ResX>x<ResY>@<Refresh>"
				exit 1
			fi
			shift
		;;
		--scantype)
			case "$2" in
				progressive) ;;
				interlace|doublescan) ScanType="$2"; shift ;;
				*) echo "Unknown scan type '$2'"; exit 1 ;;
			esac
		;;
		--force) Force=y ;;
		--update-video-driver) UpdateVideoDriver=y ;;
		--force-vesa) ForceVESA=y ;;
		--conffile) ConfigFile="$2"; shift ;;
		*) echo "Unknown option '$1'"; exit 1 ;;
	esac
	shift
done

if [[ ! -f /usr/pluto/bin/X-ChangeDisplayDriver.awk ]]; then
	Logging "$TYPE" "$SEVERITY_CRITICAL" "Xconfigure" "File not found: /usr/pluto/bin/X-ChangeDisplayDriver.awk."
	exit 1
fi

if [[ ! -f /usr/pluto/bin/X-GetDisplayDriver.awk ]]; then
	Logging "$TYPE" "$SEVERITY_CRITICAL" "Xconfigure" "File not found: /usr/pluto/bin/X-GetDisplayDriver.awk."
	exit 1
fi

if [[ ! -f /usr/pluto/bin/X-ModulesSection.awk ]]; then
	Logging "$TYPE" "$SEVERITY_CRITICAL" "Xconfigure" "File not found: /usr/pluto/bin/X-ModulesSection.awk."
	exit 1
fi

if [[ ! -f /usr/pluto/bin/X-UI_Sections.awk ]]; then
	Logging "$TYPE" "$SEVERITY_CRITICAL" "Xconfigure" "File not found: /usr/pluto/bin/X-UI_Sections.awk."
	exit 1
fi

if [[ ! -f "$ConfigFile" || ! -s "$ConfigFile" ]]; then
	# TODO: Detect incomplete/corrupt config files too
	Logging "$TYPE" "$SEVERITY_WARNING" "Xconfigure" "Config file '$ConfigFile' not found or empty. Forcing use of defaults."
	Defaults=y
fi

[[ -n "$Defaults" || -n "$UpdateVideoDriver" ]] && DisplayDriver=$(GetVideoDriver)
CurrentDisplayDriver=$(awk -f/usr/pluto/bin/X-GetDisplayDriver.awk "$ConfigFile")
if [[ "$Defaults" == y ]]; then
	if [[ ! -f /usr/pluto/templates/xorg.conf.in ]]; then
		Logging "$TYPE" "$SEVERITY_CRITICAL" "Xconfigure" "File not found: /usr/pluto/templates/xorg.conf.in. Can't setup defaults"
		exit 1
	fi
	cat /usr/pluto/templates/xorg.conf.in | awk -v"DisplayDriver=$DisplayDriver" -f/usr/pluto/bin/X-ChangeDisplayDriver.awk >"$ConfigFile"
	CurrentDisplayDriver="$DisplayDriver"
elif [[ -n "$UpdateVideoDriver" && "$DisplayDriver" != "$CurrentDisplayDriver" ]]; then
	awk -v"DisplayDriver=$DisplayDriver" -f/usr/pluto/bin/X-ChangeDisplayDriver.awk "$ConfigFile" >"$ConfigFile.$$"
	CurrentDisplayDriver="$DisplayDriver"
	mv "$ConfigFile"{.$$,}
fi
/usr/pluto/bin/X-UpdateModules.sh --conffile "$ConfigFile"
UI_Version=$(/usr/pluto/bin/X-WhichUI.sh)
if [[ "$CurrentDisplayDriver" == nvidia ]]; then
	UI_Version=2 # force UI v2 options for nVidia
fi
awk -v"UI=$UI_Version" -f/usr/pluto/bin/X-UI_Sections.awk "$ConfigFile" >"$ConfigFile.$$"
mv "$ConfigFile"{.$$,}

# Test detected display driver
# Don't test if driver is vesa (assumption: always works) or current driver (assumption: already tested and works)
if [[ "$DisplayDriver" != "$CurrentDisplayDriver" && "$DisplayDriver" != vesa ]] && [[ -n "$Defaults" || -n "$UpdateVideoDriver" ]]; then
	if ! TestConfig && [[ " ${OrigParams[*]} " != *" --force-vesa "* ]]; then
		exec "$0" "${OrigParams[@]}" --force-vesa
	fi
fi

if [[ -n "$Resolution" ]]; then
	for Var in ${!resHD*}; do
		if [[ "$Resolution" == "${!Var}" ]]; then
			nvHD=${Var#res}
			if [[ "$ScanType" == interlace && "$Var" != *720* ]]; then
				nvHD="${nvHD}i"
			else
				nvHD="${nvHD}p"
			fi
			break
		fi
	done
	Modeline="$(GenModeline "$ResX" "$ResY" "$Refresh" "$ScanType")"
	if [[ -z "$nvHD" && " 60 75 " == *" $Refresh "* ]]; then
		# don't use a modeline for standard refresh rates
		# (X seems to know only 60 and 75 as "standard", all the others either not working, or giving different resolutions)
		Modeline=
	fi

	if ! grep -q "Driver.*\"nvidia\"" "$ConfigFile"; then
		# we don't have a nVidia card; we use the nvHD variable to detect non-VESA modes too (not a perfect way to do so though)
		nvHD=
	fi
	awk -v"ResX=$ResX" -v"ResY=$ResY" -v"Refresh=$Refresh" -v"Modeline=$Modeline" -v"Force=$Force" -v"nvHD=$nvHD" -f/usr/pluto/bin/X-ChangeResolution.awk "$ConfigFile" >"$ConfigFile.$$"
	mv "$ConfigFile"{.$$,}
fi
