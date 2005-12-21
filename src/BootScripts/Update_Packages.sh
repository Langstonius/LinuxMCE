#!/bin/bash

. /usr/pluto/bin/SQL_Ops.sh

cronEntry="0 3 * * * root /usr/pluto/bin/Update_Packages.sh --download-only"
cronNotify="0 */10 * * * root /usr/pluto/bin/Update_Notify.sh"

Lock=/usr/pluto/locks/upgrade_ok
rm -f "$Lock"

if ! grep -qF "$cronEntry" /etc/crontab; then
	echo "$cronEntry" >>/etc/crontab
	/etc/init.d/cron reload
fi

[[ -n "$1" ]] && DownloadOnly="--download-only"

mv /var/log/pluto/upgrade.newlog /var/log/pluto/upgrade.log 2>/dev/null
exec &> >(tee /var/log/pluto/upgrade.newlog)

echo "Performing system update"
echo "- Updating package lists"
apt-get update
Fail=0

# Harmless, and it cleans up the dpkg journal too
echo "- Cleaning up dpkg journal"
dpkg --forget-old-unavail

echo "- Previewing dist-upgrade (step 1)"
InstPkgs="$(apt-get -s -f dist-upgrade | grep "^Conf " | cut -d' ' -f2 | tr '\n' ' ')"
RebootPkg="pluto-kernel-upgrade"
for Pkg in $RebootPkg; do
	if [[ "$InstPkgs" == *"$Pkg"* ]]; then
		DoReboot=y
		break
	fi
done

echo "- Previewing dist-upgrade (step 2)"
Count=$(apt-get -f -y -s dist-upgrade | egrep -c '^Inst |^Conf ')

echo "- Doing dist-upgrade"
if apt-get -V -f -y $DownloadOnly dist-upgrade; then
	if [[ -n "$DownloadOnly" ]]; then
		date -R >"$Lock"
	elif [[ "$Count" != "0" ]]; then
		Q="UPDATE Device SET NeedConfigure=1"
		RunSQL "$Q"
		if [[ "$DoReboot" == y ]]; then
			echo "New kernel installed. Rebooting"
			reboot
		fi
	fi
else
	Fail=1
	cp /var/log/pluto/upgrade.newlog /usr/pluto/coredump/
fi

echo "- Copying kernel package(s) for later use"
cp /var/cache/apt/archives/kernel-image* /usr/pluto/deb-cache/dists/sarge/main/binary-i386/ 2>/dev/null

if [[ -z "$DownloadOnly" && $Fail -eq 0 ]]; then
	apt-get clean
fi
