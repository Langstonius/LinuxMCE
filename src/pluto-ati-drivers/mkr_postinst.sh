#!/bin/bash

mkdir -p /etc/X11
/usr/pluto/bin/Xconfigure.sh --update-video-driver --keep-resolution
