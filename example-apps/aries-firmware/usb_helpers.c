/*

	This file is a part of PIC16F1454 firmware used on the SpiderSoM and
	MX10 System-on-Modules.  Please, refer to http://www.spiderboard.org/
	and http://www.aries-embedded.de/?q=MX10

	Copyright (C) 2018 ARIES Embedded GmbH

	Based on:

	example minimal CDC serial port adapter using PIC16F1454 microcontroller
	this is specific to the PIC16F1454; TX is on pin RC4 and RX on pin RC5

	based on M-Stack by Alan Ott, Signal 11 Software

	culled from USB CDC-ACM Demo (by Alan Ott, Signal 11 Software)
	and ANSI C12.18 optical interface (by Peter Lawrence)

	Copyright (C) 2014,2015 Peter Lawrence

	Permission is hereby granted, free of charge, to any person obtaining a 
	copy of this software and associated documentation files (the "Software"), 
	to deal in the Software without restriction, including without limitation 
	the rights to use, copy, modify, merge, publish, distribute, sublicense, 
	and/or sell copies of the Software, and to permit persons to whom the 
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in 
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
	DEALINGS IN THE SOFTWARE.
 */

#include "usb.h"
#include <xc.h>
#include <string.h>
#include "usb_config.h"
#include "usb_ch9.h"
#include "usb_cdc.h"
#include "usb_dfu.h"
#include "usb_i2ctinyusb.h"
#include "jtag.h"

/* Callbacks. These function names are set in usb_config.h. */
void app_set_configuration_callback(uint8_t configuration) {

}

uint16_t app_get_device_status_callback() {
    return 0x0000;
}

void app_endpoint_halt_callback(uint8_t endpoint, bool halted) {

}

int8_t app_set_interface_callback(uint8_t interface, uint8_t alt_setting) {
    return 0;
}

int8_t app_get_interface_callback(uint8_t interface) {
    return 0;
}

void app_out_transaction_callback(uint8_t endpoint) {

}

void app_in_transaction_complete_callback(uint8_t endpoint) {

}

int8_t app_unknown_setup_request_callback(const struct setup_packet *setup) {
    uint16_t interface = setup->wIndex;
    
    DebugSerial("USB: ", false);
    DebugSerialBytes(setup, sizeof(struct setup_packet), true);
    
    // I2C Tiny-USB (interface 3) has the MSb set instead of using the index
    if (interface & 0x8000) return process_i2ctinyusb_setup_request(setup);
    //FTDI requests are sent by "Type: Vendor" in bmRequestType field
    if (setup->REQUEST.bmRequestType & 0x40) return ProcessJTAGSetup(setup);
    if (interface == 1 || interface == 2) return process_cdc_setup_request(setup);
    if (interface == 4) return process_dfu_setup_request(setup);

    return -1;
}

int16_t app_unknown_get_descriptor_callback(const struct setup_packet *pkt, const void **descriptor) {
    return -1;
}

void app_start_of_frame_callback(void) {

}

void app_usb_reset_callback(void) {

}

/* CDC Callbacks. See usb_cdc.h for documentation. */

int8_t app_send_encapsulated_command(uint8_t interface, uint16_t length) {
    return -1;
}

int16_t app_get_encapsulated_response(uint8_t interface,
        uint16_t length, const void **report,
        usb_ep0_data_stage_callback *callback,
        void **context) {
    return -1;
}

void app_set_comm_feature_callback(uint8_t interface,
        bool idle_setting,
        bool data_multiplexed_state) {

}

void app_clear_comm_feature_callback(uint8_t interface,
        bool idle_setting,
        bool data_multiplexed_state) {

}

int8_t app_get_comm_feature_callback(uint8_t interface,
        bool *idle_setting,
        bool * data_multiplexed_state) {
    return -1;
}

void app_set_line_coding_callback(uint8_t interface,
        const struct cdc_line_coding * coding) {
    // Formula is:
    // dataRate = (PICFREQ / (4 * Baudrate)) - 1
    // where PICFREQ is 48 MHz

    /* 
     * The cdc driver may change the baudrate after the device was connected.
     * In debug-serial mode the baudrate is fixed at ~115200 Hz and therefore 
     * this function is disabled.
     */

#if DEBUGSERIAL == 1
    return;
#else
    SPBRG = (12000000 / coding->dwDTERate) - 1;
#endif
}

int8_t app_get_line_coding_callback(uint8_t interface,
        struct cdc_line_coding * coding) {
    /* This is where baud rate, data, stop, and parity bits are set. */
    return -1;
}

int8_t app_set_control_line_state_callback(uint8_t interface,
        bool dtr, bool dts) {
    return 0;
}

int8_t app_send_break_callback(uint8_t interface, uint16_t duration) {
    return 0;
}
