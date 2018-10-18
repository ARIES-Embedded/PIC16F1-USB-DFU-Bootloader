/*
	This file is a part of PIC16F1454 firmware used on the SpiderSoM and
	MX10 System-on-Modules.  Please, refer to http://www.spiderboard.org/
	and http://www.aries-embedded.de/?q=MX10

	Copyright (C) 2018 ARIES Embedded GmbH

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OFMERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OROTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/

#ifndef USB_DFU_H
#define	USB_DFU_H
#include <xc.h>
#include <string.h>
#include "usb_config.h"
#include "debug.h"

#define DESC_DFUFUNCTION 0x21

enum DFU_REQUESTS {
    DFU_DETACH = 0, DFU_DNLOAD, DFU_UPLOAD, DFU_GETSTATUS, DFU_CLRSTATUS, DFU_GETSTATE, DFU_ABORT
};

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bmAttributes;
    uint16_t wDetachTimeOut;
    uint16_t wTransferSize;
    uint16_t bcdDFUVersion;
} dfu_descriptor;

typedef struct {
    uint8_t bStatus;
    struct {
        unsigned PollTimeout_Byte1 : 8;
        unsigned PollTimeout_Byte2 : 8;
        unsigned PollTimeout_Byte3 : 8;
    };
    uint8_t bState;
    uint8_t iString;
} dfu_status_packet;

int8_t process_dfu_setup_request(struct setup_packet* setup);

//while this is true, a usb reset will switch the device to bootloader
extern bool DFUWaitForReset;

//this variable will be incremented when timer0 overflows (~21.3 us)
extern uint16_t DFUTimer0PostScale;

//when DFUTimer0PostScale reaches this variable, the device will enter bootloader (trigger watchdog)
extern uint16_t DFUTimer0Timeout; //will be set by DFU_DETACH request -- a value of 46875 means 1 second -- (12,000,000 / (256 * value)) = time in s

extern uint8_t DFUState;

extern dfu_status_packet DFUStatus;

#endif	/*USB_DFU_H*/

