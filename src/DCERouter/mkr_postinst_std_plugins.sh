#!/bin/bash
/usr/pluto/bin/Debug_LogKernelModules.sh "$0"

. /usr/pluto/bin/Config_Ops.sh

if [[ -z "$Bookmark_Media" ]]; then
	ConfSet Bookmark_Media "4,5"
fi
