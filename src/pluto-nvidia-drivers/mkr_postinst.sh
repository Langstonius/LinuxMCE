#!/bin/bash
/usr/pluto/bin/Debug_LogKernelModules.sh "$0" || :

mkdir -p /etc/X11
bash -x /usr/pluto/bin/Xconfigure.sh --update-video-driver --keep-resolution | tee-pluto /var/log/pluto/Xconfigure.log
rm -f /etc/modprobe.d/lrm-video || :
sed -i 's/"nv"/"vesa"/g' /etc/X11/xorg.conf || :
sed -i 's/"nvidia"/"vesa"/g' /etc/X11/xorg.conf || :
for KVer in <-mkr_t_MakeRelease_Kernel->; do
	depmod "$KVer" || :
done
