#ifndef __INPUT_H__
#define __INPUT_H__
//--------------------------------------------------------------------------------------------------------
#include <string>
//--------------------------------------------------------------------------------------------------------
enum InputType
{
	itNone,
	itKeyboardInput,
	itChangeStatus
};
//--------------------------------------------------------------------------------------------------------
enum KeyboardInput
{
	kiNone,
	kiUp,
	kiDown,
	kiLeft,
	kiRight,
	kiEnter
};
//--------------------------------------------------------------------------------------------------------
class Input
{
public:

	InputType type;
	KeyboardInput keyboardinput;
	std::string status;
	std::string progress;
};
//--------------------------------------------------------------------------------------------------------
#endif //__INPUT_H__
