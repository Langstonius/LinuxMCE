#!/bin/bash

version=8776
changedate="$(date -R)"
changelog=$(cat <<'END'
nvidia-graphics-drivers (1.0.$version-1) unstable; urgency=low

  * built with $version

 -- Razvan Gavril <razvan.g@plutohome.com>  $changedate

END
)

for i in debian/* debian.binary/*; do
	if [[ "$i" == */changelog ]]; then
		if ! grep -q "$version" "$i"; then
			eval "echo \"$changelog\"" >"$i".new
			echo >>"$i".new
			cat "$i" >>"$i".new
			mv "$i"{.new,}
		fi
		continue
	fi
	[[ -d "$i" ]] && continue
	sed -i 's/8774/'"$version"'/g; s/glxtokens/glxext/g' "$i"
done

sed -i 's/README\(.txt\)*/README.txt/g' debian/nvidia-glx.docs*
