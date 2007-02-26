#!/bin/bash -x

. /root/Ubuntu_Helpers/SQL_Ops.sh

TMP_DIR="/var/ubuntu/"
MAX_LEVEL=6

function GetMaxLevel 
{
	local package=$1

	Q="SELECT Level FROM Dependencies WHERE PackageName='$package'"
	levels=$(RunSQL "$Q")
	max="0"
	for level in $levels ;do
		if [[ "$max" < "$level" ]] ;then
			max="$level"
		fi
	done
	echo "$max"
}

function ComputeDependencies
{
	local package=$1
	local level=$2
	
	maxlevel=$(GetMaxLevel "$package")

	if [[ "$maxlevel" > "$level" ]] || [[ "$maxlevel" == "0" ]] ;then

		#echo "--->>>> P: $package ->>> L: $level"

		if [[ "$level" < "$MAX_LEVEL" ]] ;then
			Q="INSERT INTO Dependencies(PackageName,Level,Done) VALUES('$package',$level,0)"
			RunSQL "$Q"
			description=$(aptitude show $package)
			depends=$(echo "$description" | grep "^Depends: "  | sed 's/^Depends://g' | sed 's/ ([^)]*)//g' | tr ',' ' ' | tr '|' ' ' | tr '(>=' ' ' | tr '(=' ' ')
			predepends=$(echo "$description" | grep "^PreDepends: "  | sed 's/^PreDepends://g' | sed 's/ ([^)]*)//g' | tr ',' ' ' | tr '|' ' '| tr '(>=' ' ' | tr '(=' ' ')
			level=$(($level+1))
			for depend in $depends ;do
				ComputeDependencies "$depend" "$level"
			done
			for predepend in $predepends ;do
				ComputeDependencies "$predepend" "$level"
			done
		elif [[ "$level" == "$MAX_LEVEL" ]] ;then
			Q="INSERT INTO Dependencies(PackageName,Level,Done) VALUES('$package',$level,0)"
			RunSQL "$Q"
		fi
	fi

}



if [[ ! -e "$TMP_DIR" ]] ;then
	mkdir "$TMP_DIR"
fi

if /bin/false ;then
Q="CREATE TABLE IF NOT EXISTS Dependencies(PackageName varchar(40), Level int(2), Done int(1))"
RunSQL "$Q"

Q="DELETE FROM Dependencies"
RunSQL "$Q"

rm -f /var/ubuntu/*
apt-get clean
apt-get update

Q="SELECT Package_Source.Name 
	FROM Package_Source_Compat 
		INNER JOIN Package_Source ON FK_Package_Source=PK_Package_Source 
		JOIN RepositorySource ON PK_RepositorySource=Package_Source.FK_RepositorySource
	WHERE (Package_Source_Compat.FK_Distro = 14 or Package_Source_Compat.FK_Distro is NULL) AND 
	(Package_Source.FK_RepositorySource IN (20,21)) AND
	FK_RepositoryType = 1;
"

packages=$(RunSQL "$Q")

for package in $packages ;do
	ComputeDependencies "$package" "1"
done
fi

Q="SELECT DISTINCT PackageName FROM Dependencies"
packages=$(RunSQL "$Q")

for package in $packages ;do
	apt-get --download-only -y install --reinstall $package
done

#rm /var/cache/apt/archives/*pluto*
cp /var/cache/apt/archives/* $TMP_DIR

pushd $TMP_DIR
dpkg-scanpackages . /dev/null > Packages
cat Packages | gzip -9c > Packages.gz
popd
					

mkisofs -f -J -r -o linuxmce-0.1-packages.iso $TMP_DIR

#for package in $(dpkg -l | cut -d " " -f 3) ;do 
#	pkg=$(grep "Package: $package$" $REP_LISTS/*_Packages | cut -d":" -f1,3 | cut -d '_' -f4,5,7 | cut -d':' -f2)
#	if [[ -n "$pkg" ]] ;then
#		apt-get --download-only install $pkg
#		for full_name in $(ls "$CACHE" | grep $pkg | grep -v pluto) ;do
#			cp $CACHE$full_name $TMP_DIR
#		done
#	fi
#done
