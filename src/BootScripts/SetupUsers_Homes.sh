#!/bin/bash
# set system password files

. /usr/pluto/bin/pluto.func
. /usr/pluto/bin/SQL_Ops.sh
. /usr/pluto/bin/Section_Ops.sh

TemplateDir=/usr/pluto/templates

BaseDir="/home"
PublicHomeDir="/home/public/data"
MakeUsers=yes
while [[ "$#" -gt 0 ]]; do
	case "$1" in
		-e) ExtraDir="$2"; ExtraLink="$3"; shift; shift ;;
		-b) BaseDir="$2"; shift ;;
		-n) MakeUsers="no" ;;
	esac
	shift
done

if [[ "$MakeUsers" == yes ]]; then
	# Extract non-pluto entries
	awk '!/^pluto_/' /etc/group >/etc/group.$$
	awk '!/^pluto_/' /etc/passwd >/etc/passwd.$$
	awk '!/^pluto_/' /etc/shadow >/etc/shadow.$$

	cp $TemplateDir/smbpasswd.tmpl /etc/samba/smbpasswd.$$
	: >/etc/samba/usermap.txt
fi

# get a list of all users
Q="SELECT PK_Users,UserName,Password_Unix,Password_Samba FROM Users"
R=$(RunSQL "$Q")
LinuxUserID=10000
DefaultSambaPassword=609FCABC7B0F9AEAAAD3B435B51404EE:DDFF3B733E17BE6500375694FE258864
DefaultLinuxPassword=

global_static_dirs="public temp_pvr mydvd cameras tv_listing"
user_dirs="audio pictures documents videos"
user_static_dirs="data"

UserList=
SambaUserShares=

for Users in $R; do
	PlutoUserID=$(Field 1 "$Users")
	UserName=$(Field 2 "$Users" | tr 'A-Z' 'a-z' | tr -dc "a-z0-9-")
	if [[ -z "$UserName" ]]; then
		continue
	fi
	LinuxPassword=$(Field 3 "$Users")
	SambaPassword=$(Field 4 "$Users")

	if [[ ! -d "$BaseDir/user_$PlutoUserID" ]]; then
		rm -rf "$BaseDir/user_$PlutoUserID"
	fi
	
	[[ -z "$LinuxPassword" ]] && LinuxPassword="$DefaultLinuxPassword"
	[[ -z "$SambaPassword" ]] && SambaPassword="$DefaultSambaPassword"
#	echo "$Users : $PlutoUserID - $UserName - $LinuxPassword - $SambaPassword"
	
	if [[ "$MakeUsers" == yes ]]; then
		SambaUserShares="${SambaUserShares}

		[$UserName]
		comment = $UserName's private files
		browseable = yes
		writable = yes
		create mask = 0770
		directory mask = 0770
		path = /home/user_$PlutoUserID
		public = no
		"
		# TODO: replace 00000001 with a real number of seconds (never set to 00000000)
		SambaEntry="pluto_$UserName:$LinuxUserID:$SambaPassword:[U          ]:LCT-00000001:,,,"
		ShadowEntry="pluto_$UserName:$LinuxPassword:12221:0:99999:7:::"
		PasswdEntry="pluto_$UserName:x:$LinuxUserID:$LinuxUserID:,,,:/home/user_$PlutoUserID:/bin/false"
		GroupEntry="pluto_$UserName:x:$LinuxUserID:www-data"
		SambaUnixMap="pluto_$UserName = $UserName"
		echo "$SambaEntry" >>/etc/samba/smbpasswd.$$
		echo "$ShadowEntry" >>/etc/shadow.$$
		echo "$PasswdEntry" >>/etc/passwd.$$
		echo "$GroupEntry" >>/etc/group.$$
		echo "$SambaUnixMap" >>/etc/samba/usermap.txt
	fi

	mkdir -p -m 0770 "$BaseDir/user_$PlutoUserID"

	for dir in $user_static_dirs; do
		mkdir -p -m 0770 "$BaseDir/user_$PlutoUserID/${dir/~/ }"
	done
	
	for dir in $user_dirs; do

# update pluto user dir structure 
                if [ -d $BaseDir/user_$PlutoUserID/data/audio/ ];then
                        mv -f $BaseDir/user_$PlutoUserID/data/music/* $BaseDir/user_$PlutoUserID/data/audio/ 2>/dev/null
                        rm -rf $BaseDir/user_$PlutoUserID/data/music/
                else
                        mv -f $BaseDir/user_$PlutoUserID/data/music/ $BaseDir/user_$PlutoUserID/data/audio/ 2>/dev/null
                fi
                mv -f $BaseDir/user_$PlutoUserID/data/movies/* $BaseDir/user_$PlutoUserID/data/videos 2>/dev/null
                rm -rf $BaseDir/user_$PlutoUserID/data/movies/
# finish
		mkdir -p -m 0770 "$BaseDir/user_$PlutoUserID/data/${dir/~/ }"
		if [[ -n "$ExtraDir" ]]; then
			Target="$ExtraDir/user_$PlutoUserID/data/${dir/~/ }/$ExtraLink"
			rm -f "$Target"
			ln -sf "$BaseDir/user_$PlutoUserID/data/${dir/~/ }" "$Target"
		fi
	done

	rm -f "$BaseDir/$UserName"
	ln -sf "user_$PlutoUserID" "$BaseDir/$UserName"
	UserList="$UserList $UserName"
# Files not updated at this point (woking on copies)
#	chown -R $UserName.$UserName /home/user_$PlutoUserID

	((LinuxUserID++))
done

if [[ "$MakeUsers" == yes ]] ;then
	PopulateSection "/etc/samba/smb.conf" "User Shares" "$SambaUserShares"
fi

for dir in $global_static_dirs; do
	mkdir -p -m 0755 "$BaseDir/${dir/~/ }"
done

for dir in $user_static_dirs; do
	mkdir -p -m 0755 "$BaseDir/public/${dir/~/ }"
done

for dir in $user_dirs; do

# update pluto public dir structure
        if [ -d $PublicHomeDir/audio ]; then
                mv -f $PublicHomeDir/music/* $PublicHomeDir/audio 2>/dev/null
                rm -rf $PublicHomeDir/music/
        else
                mv -f $PublicHomeDir/music/ $PublicHomeDir/audio 2>/dev/null
        fi
        mv -f $PublicHomeDir/movies/* $PublicHomeDir/videos 2>/dev/null
        rm -rf $PublicHomeDir/movies/
# end
	mkdir -p -m 0755 "$BaseDir/public/data/${dir/~/ }"
	if [[ -n "$ExtraDir" ]]; then
		Target="$ExtraDir/public/data/${dir/~/ }/$ExtraLink"
		rm -f "$Target"
		ln -sf "$BaseDir/public/data/${dir/~/ }" "$Target"
	fi
done

# Get a list of all Media Directors
Q="SELECT PK_Device FROM Device WHERE FK_DeviceTemplate=28"
R=$(RunSQL "$Q")

for Device in $R; do
	mkdir -p -m 0755 /home/tmp_$Device
done

if [[ "$MakeUsers" == yes ]]; then
	files="samba/smbpasswd shadow passwd group"
	for file in $files; do
		mv -f /etc/$file.$$ /etc/$file
	done

	# Only now we have the copies in place of the originals so we can add users and set ownerships
	addgroup public &>/dev/null
	for User in $UserList; do
		adduser pluto_$User public &>/dev/null
		chown --dereference -R pluto_$User.pluto_$User /home/$User/
	done

	if [[ "$(pidof smbd)" != "" ]] ;then
		invoke-rc.d samba reload
	fi
fi

chmod -R 2770 "$BaseDir"/user_* 2>/dev/null
chmod -R 2775 "$BaseDir"/public 2>/dev/null
chgrp -R public "$BaseDir"/public

## ReAdd the sambahelper user to smbpasswd
if [[ -r /usr/pluto/var/sambaCredentials.secret ]] ;then
	smbpass=$(cat /usr/pluto/var/sambaCredentials.secret | grep '^password' | cut -d '=' -f2)
	smbpass=$(/usr/pluto/bin/smbpass.pl $smbpass)

	smbuser=$(cat /usr/pluto/var/sambaCredentials.secret | grep '^user' | cut -d '=' -f2)
	smbuserid=$(id -u $smbuser)

	echo "$smbuser:$LinuxUserID:$smbpass:[U          ]:LCT-00000001:,,," >> /etc/samba/smbpasswd 
fi

## Rebuild NIS cache
make -C /var/yp
