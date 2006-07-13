#!/bin/bash

Extension="$1"

if [[ -z "$Extension" ]]; then
	echo "Empty extension"
	exit 1
fi

. /usr/pluto/bin/SQL_Ops.sh
. /usr/pluto/bin/Config_Ops.sh

EVENT_Voice_Mail_Changed=37
EVENTPARAMETER_PK_Users=31
EVENTPARAMETER_Value=30

ExtensionAllUsers=100
VoiceMailDir=/var/spool/asterisk/voicemail/default/"$Extension"/INBOX

if [[ "$Extension" == "$ExtensionAllUsers" ]]; then
	PK_Users=0
else
	Q="SELECT PK_Users FROM Users WHERE Extension='$Extension'"
	PK_Users=$(RunSQL "$Q")
fi

FileCount=$(ls -1 /var/spool/asterisk/voicemail/default/"$Extension"/INBOX/*.gsm | wc -l)
/usr/pluto/bin/MessageSend $DCERouter $PK_Device -1001 2 $EVENT_Voice_Mail_Changed $EVENTPARAMETER_PK_Users $PK_Users $EVENTPARAMETER_Value "$FileCount 0"
