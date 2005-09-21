#ifndef __CM11ACONSTS_H_
#define __CM11ACONSTS_H_

// genral consts
#define CM11A_ACK				0x00
#define CM11A_INTERFACE_READY	0x55
#define CM11A_COMPUTER_READY	0xC3
#define CM11A_INTERFACE_CQ		0x5A
#define CM11A_CLOCK_REQ			0xA5


#define CM11A_STANDARD_ADDRESS	0x04
#define CM11A_STANDARD_FUNCTION	0x06

// function consts (foo values, not tested yet)
#define CM11A_FUNC_ALL_0FF		0x00
#define CM11A_FUNC_ALL_ON		0x01
#define CM11A_FUNC_0N			0x02
#define CM11A_FUNC_0FF			0x03
#define CM11A_FUNC_DIMM			0x04
#define CM11A_FUNC_BRIGHT		0x05
#define CM11A_FUNC_ALL_L_0FF	0x06

#define CM11A_SET_TIME        	0x9B

#endif
