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

#include "uart.h"

/* local data buffers for CDC functionality */
static uint8_t PC2PIC_Buffer[EP_3_LEN];
static uint8_t PIC2PC_Buffer[EP_3_LEN];

/* variables to track positions and occupancy in local CDC buffers */
uint8_t PIC2PC_pending_count;
size_t PC2PIC_buffer_occupancy;
uint8_t PC2PIC_read_index;
uint8_t pc2pic_data_ready;
uint8_t *in_buf;
const uint8_t *out_buf;


void UARTInit(void) {
    /* RX on RC5 is an input */
    TRISCbits.TRISC5 = 1;

    /* TX on RC4 is an output */
    TRISCbits.TRISC4 = 0;

    TXSTA = 0x24;
    RCSTA = 0x90;

    /* 115234 bps */
    SPBRG = 103;

    BAUDCON = 0x08;

    /* clear any data in receive buffer */
    (volatile void) RCREG;
    
    pc2pic_data_ready = 0;
    PIC2PC_pending_count = 0;
    PC2PIC_buffer_occupancy = 0;
}

void HandleUART() {
    /* if the UART has received another byte, add it to the queue if there is room */
    if (PIR1bits.RCIF && (PIC2PC_pending_count < (EP_3_LEN - 1))) {
        if (RCSTAbits.OERR)
            RCSTAbits.CREN = 0; /* in case of overrun error, reset the port */
        PIC2PC_Buffer[PIC2PC_pending_count] = RCREG;
        RCSTAbits.CREN = 1; /* and then (re-)enable receive */
        ++PIC2PC_pending_count;
    }

    /* if our PC2PIC buffer is not empty *AND* the USART can accept another byte, transmit another byte */
    if (pc2pic_data_ready && TXSTAbits.TRMT) {
        /* transmit another byte from buffer */
        TXREG = PC2PIC_Buffer[PC2PIC_read_index];
        /* update the read index */
        ++PC2PIC_read_index;
        /* if the read_index has reached the occupancy value, signal that the buffer is empty again */
        if (PC2PIC_read_index == PC2PIC_buffer_occupancy)
            pc2pic_data_ready = 0;
    }

    /* proceed further only if the PC can accept more data */
    if (usb_in_endpoint_halted(3) || usb_in_endpoint_busy(3))
        return;

    /* if we've reached here, the USB stack can accept more; if we have PIC2PC data to send, we hand it over */
    if (PIC2PC_pending_count > 0) {
        in_buf = usb_get_in_buffer(3);
        memcpy(in_buf, PIC2PC_Buffer, PIC2PC_pending_count);
        usb_send_in_buffer(3, PIC2PC_pending_count);
        PIC2PC_pending_count = 0;
    }

    /* if our PC2PIC buffer is NOT empty, we won't go further (where we would try to accept more from the PC) */
    if (pc2pic_data_ready)
        return;

    /* if we pass this test, we are committed to make the usb_arm_out_endpoint() call */
    if (!usb_out_endpoint_has_data(3))
        return;

    /* ask USB stack for more PC2PIC data */
    PC2PIC_buffer_occupancy = usb_get_out_buffer(3, &out_buf);

    /* if there was any, put it in the buffer and update the state variables */
    if (PC2PIC_buffer_occupancy > 0) {
        memcpy(PC2PIC_Buffer, out_buf, PC2PIC_buffer_occupancy);
        /* signal that the buffer is not empty */
        pc2pic_data_ready = 1;
        /* rewind read index to the beginning of the buffer */
        PC2PIC_read_index = 0;
    }

    usb_arm_out_endpoint(3);

}
