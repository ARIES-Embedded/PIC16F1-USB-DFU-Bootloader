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

#include "usb_config.h"
#include "usb_ch9.h"
#include "usb.h"
#include "usb_dfu.h"

STATIC_SIZE_CHECK_EQUAL(sizeof (dfu_descriptor), 9);
STATIC_SIZE_CHECK_EQUAL(sizeof (dfu_status_packet), 6);

uint8_t DFUState = 0;
dfu_status_packet DFUStatus;
bool DFUWaitForReset = false;
uint16_t DFUTimer0PostScale;
uint16_t DFUTimer0Timeout;

int8_t process_dfu_setup_request(struct setup_packet* setup) {

    DebugSerial("DFU Setup", true);
    
    //On DFU_DETACH send a packet with the current state and then trigger watchdog timeout to switch to DFU Bootloader
    if (setup->REQUEST.bmRequestType == 0b00100001 && setup->bRequest == DFU_DETACH) {
        DFUState = 1; // state is now appDetach
        usb_send_data_stage(0, 0, 0, 0);
		UCONbits.PKTDIS = 0;
		// Delay for USB IN request to be sent.
		__delay_ms(15);
        // Trigger watchdog
		WDTCON = 1;
		while (1);
    }

    if (setup->REQUEST.bmRequestType == 0b10100001 && setup->bRequest == DFU_GETSTATE) {
        usb_send_data_stage(&DFUState, 1, NULL, NULL);
        return 0;
    }

    if (setup->REQUEST.bmRequestType == 0b10100001 && setup->bRequest == DFU_GETSTATUS) {

        DFUStatus.bState = DFUState;
        DFUStatus.bStatus = 0;
        DFUStatus.PollTimeout_Byte1 = 0;
        DFUStatus.PollTimeout_Byte2 = 0;
        DFUStatus.PollTimeout_Byte3 = 0;
        DFUStatus.iString = -1; //Unsupported

        usb_send_data_stage(&DFUStatus, sizeof (dfu_status_packet), 0, 0);

        return 0;
    }

    return -1;

}
