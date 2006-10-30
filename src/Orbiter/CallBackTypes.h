#ifndef _CALLBACKTYPES_H_
#define _CALLBACKTYPES_H_

//used by screen handler class
enum CallBackType
{
    cbUnused,
    cbDataGridSelected,
	cbDataGridRendering,
    cbObjectSelected,
	cbObjectHighlighted,
    cbOnDialogCreate,   // create a unique dialog
    cbOnDialogDelete,   // close dialog without saving data
    cbOnDialogRefresh,  // refresh dialog with new data
    cbOnDialogSave,     // save data from dialog
    cbOnDialogWaitUser, // wait for an user action
    cbOnKeyDown,
    cbOnKeyUp,
    cbOnMouseDown,
    cbOnMouseUp,
    cbCapturedKeyboardBufferChanged,
    cbOnTimer,
    cbMessageIntercepted,
    cbOnRenderScreen,
	cbOnCustomRender,
    cbOnGotoScreen
};

#endif
// _CALLBACKTYPES_H_
