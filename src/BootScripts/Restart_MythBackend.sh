#!/bin/bash

# XXX: This script restarts the mythtv backend if it should be running but isn't

. /usr/pluto/bin/pluto.func
. /usr/pluto/bin/LockUtils.sh
. /usr/pluto/bin/Network_Parameters.sh

while :; do
	# Lock the MythBackend lock to prevent backend restarting when MythTV Setup is running
	# launchMythSetup holds this lock while MythTV is running
	WaitLock "MythBackend" "MythBackend_Restart" nolog
	Backend=$(pgrep mythbackend)
	#MythSetup=$(pgrep mythtv-setup)
	if [[ -z "$Backend" && -x /etc/init.d/mythtv-backend ]]; then
		Logging "$TYPE" "$SEVERITY_CRITICAL" "MythBackend_Restart" "MythBackend not found running; restarting it"
		echo "LOCK TABLE schemalock WRITE;" | mysql mythconverg  # Be sure we're not in the middle of a schema upgrade -- myth doesn't check this
		/etc/init.d/mythtv-backend restart
		Logging "$TYPE" "$SEVERITY_CRITICAL" "MythBackend_Restart" "MythBackend restarted"
	fi
	Unlock "MythBackend" "MythBackend_Restart" nolog
	sleep 5
done
