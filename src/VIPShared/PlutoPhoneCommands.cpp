/*
 PlutoPhoneCommands
 
 Copyright (C) 2004 Pluto, Inc., a Florida Corporation
 
 www.plutohome.com		
 
 Phone: +1 (877) 758-8648
 
 This program is distributed according to the terms of the Pluto Public License, available at: 
 http://plutohome.com/index.php?section=public_license 
 
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.
 
 */

#include "VIPShared/PlutoPhoneCommands.h"

#include "BD/BD_WhatDoYouHave.h"
#include "BD_PC_Disconnect.h"
#include "BD_PC_KeyWasPressed.h"
#include "BD_PC_VMCTerminated.h"
#include "BD_PC_VMCHidden.h"
#include "BD_PC_ReportingVariables.h"
#include "BD_PC_OrderPlaced.h"
#include "BD_PC_SelectedFromList.h"
#include "BD_PC_SetVariable.h"
#include "BD_PC_ReportMyVersion.h"
#include "BD_PC_GetSignalStrength.h"
#include "BD_PC_SetImageQuality.h"

#include "BD/BD_HaveNothing.h"
#include "BD_CP_SendMeKeystrokes.h"
#include "BD_CP_ShowImage.h"
#include "BD_CP_ShowVMC.h"
#include "BD_CP_ShowList.h"
#include "BD_CP_Disconnect.h"
#include "BD_CP_UpdateVariables.h"
#include "BD_CP_CaptureKeyboard.h"
#include "BD_CP_SendFile.h"
#include "BD_CP_SimulateEvent.h"
#include "BD_CP_CurrentSignalStrength.h"

class BDCommand *BuildCommandFromData( unsigned long dwType )
{
	switch( dwType )
	{
	case BD_PC_WHAT_DO_YOU_HAVE:
		return new BD_WhatDoYouHave();
	case BD_PC_DISCONNECT:
		return new BD_PC_Disconnect(); 
	case BD_PC_KEY_WAS_PRESSED:
		return new BD_PC_KeyWasPressed();
	case BD_PC_VMC_TERMINATED:
		return new BD_PC_VMCTerminated();
	case BD_PC_REPORTING_VARIABLES:
		return new BD_PC_ReportingVariables();
	case BD_PC_ORDER_PLACED:
		return new BD_PC_OrderPlaced();
	case BD_PC_SELECTED_FROM_LIST:
		return new BD_PC_SelectedFromList();
	case BD_PC_SET_VARIABLE:
		return new BD_PC_SetVariable();
    case BD_PC_REPORT_MY_VERSION:
		return new BD_PC_ReportMyVersion();
    case BD_PC_GET_SIGNAL_STRENGTH:
        return new BD_PC_GetSignalStrength();
    case BD_PC_SET_IMAGE_QUALITY:
        return new BD_PC_SetImageQuality();


	case BD_CP_HAVE_NOTHING:
		return new BD_HaveNothing();
	case BD_CP_SEND_ME_KEYSTROKES:
		return new BD_CP_SendMeKeystrokes();
	case BD_CP_SHOW_IMAGE:
		return new BD_CP_ShowImage();
	case BD_CP_SHOW_VMC:
		return new BD_CP_ShowVMC();
	case BD_CP_SHOW_LIST:
		return new BD_CP_ShowList();
	case BD_CP_DISCONNECT:
		return new BD_CP_Disconnect();
	case BD_CP_UPDATE_VARIABLES:
		return new BD_CP_UpdateVariables();
	case BD_CP_CAPTURE_KEYBOARD:
		return new BD_CP_CaptureKeyboard();
	case BD_CP_SEND_FILE:
		return new BD_CP_SendFile();
	case BD_CP_SIMULATE_EVENT:
		return new BD_CP_SimulateEvent();
    case BD_CP_CURRENT_SIGNAL_STRENGTH:
        return new BD_CP_CurrentSignalStrength();

	default:
		// Log Error
		return NULL;
	};
}
