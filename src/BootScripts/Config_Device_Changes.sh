#!/bin/bash

. /usr/pluto/bin/Config_Ops.sh
. /usr/pluto/bin/SQL_Ops.sh

NeedConfigure()
{
	local Device NeedConfigure PK_Device
	local i

	Device="$1"

	Q="SELECT NeedConfigure FROM Device WHERE PK_Device='$Device'"
	R=$(RunSQL "$Q")
	NeedConfigure=$(Field 1 "$R")

	if [ "$NeedConfigure" -eq 1 ]; then
		return 0
	fi

	Q="SELECT PK_Device FROM Device WHERE FK_Device_ControlledVia='$Device'"
	R=$(RunSQL "$Q")
	for i in $R; do
		if NeedConfigure $i; then
			return 0
		fi
	done
	
	return 1
}

Unset_NeedConfigure_Children()
{
	local Device
	local i

	Device="$1"
#	Q="UPDATE Device SET NeedConfigure=0 WHERE PK_Device='$Device'" # this line resets the orbiters and OrbiterGen doesn't run anymore
	Q="UPDATE Device
		JOIN DeviceTemplate ON FK_DeviceTemplate=PK_DeviceTemplate
		JOIN DeviceCategory ON FK_DeviceCategory=DeviceCategory.PK_DeviceCategory
		LEFT JOIN DeviceCategory AS DeviceCategory2 ON DeviceCategory2.PK_DeviceCategory=DeviceCategory.FK_DeviceCategory_Parent
		LEFT JOIN DeviceCategory AS DeviceCategory3 ON DeviceCategory3.PK_DeviceCategory=DeviceCategory2.FK_DeviceCategory_Parent
		SET NeedConfigure=0
		WHERE (DeviceCategory.PK_DeviceCategory<>5 OR DeviceCategory.PK_DeviceCategory IS NULL)
		AND (DeviceCategory2.PK_DeviceCategory<>5 OR DeviceCategory2.PK_DeviceCategory IS NULL)
		AND (DeviceCategory3.PK_DeviceCategory<>5 OR DeviceCategory3.PK_DeviceCategory IS NULL)
		AND (DeviceCategory3.FK_DeviceCategory_Parent<>5 OR DeviceCategory3.FK_DeviceCategory_Parent IS NULL)
		AND PK_Device='$Device'"
	RunSQL "$Q"

	Q="SELECT PK_Device FROM Device WHERE FK_Device_ControlledVia='$Device'"
	R=$(RunSQL "$Q")
	for i in $R; do
		Unset_NeedConfigure_Children $i
	done
}

if [[ "$1" != "F" ]]; then
	NeedConfigure "$PK_Device" || exit 0
fi

[ -n "$MySqlPassword" ] && Pass="-p$MySqlPassword"
CUsh="/usr/pluto/install/config_update.sh"

echo /usr/pluto/bin/ConfirmDependencies -n -h $MySqlHost -u $MySqlUser $Pass -d $PK_Device install
/usr/pluto/bin/ConfirmDependencies -n -h $MySqlHost -u $MySqlUser $Pass -d $PK_Device install >"$CUsh.$$"
linecount=$(cat "$CUsh.$$" | wc -l)
awk "NR<$linecount-8" "$CUsh.$$" >"$CUsh"
rm "$CUsh.$$"

chmod +x "$CUsh"
if bash -x "$CUsh" &> >(tee /var/log/pluto/Config_Device_Changes.newlog); then
	Unset_NeedConfigure_Children "$PK_Device"
fi
#rm "$CUsh"

echo /usr/pluto/bin/ConfirmDependencies -n -h $MySqlHost -u $MySqlUser $Pass -d $PK_Device buildall
mkdir -p /usr/pluto/sources

: >"/usr/pluto/sources/buildall.sh"
echo '#!/bin/bash' >>"/usr/pluto/sources/buildall.sh"
echo "cd /usr/pluto/sources" >>"/usr/pluto/sources/buildall.sh"
/usr/pluto/bin/ConfirmDependencies -n -h $MySqlHost -u $MySqlUser $Pass -d $PK_Device buildall >>"/usr/pluto/sources/buildall.sh"
rm -f "/usr/pluto/install/compile.sh" # old version mistake precaution
ln -sf "/usr/pluto/sources/buildall.sh" "/usr/pluto/install/compile.sh"
chmod +x "/usr/pluto/sources/buildall.sh"

if [[ "$2" == "StartLocalDevice" ]]; then
        echo "Starting local devices"
        /usr/pluto/bin/Start_LocalDevices.sh
fi

