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

//These configuration words have to be set when the PIC is programmed directly through MPLABX
//CONFIG1
#pragma config FOSC = INTOSC
#pragma config WDTE = SWDTEN
#pragma config PWRTE = ON
#pragma config MCLRE = OFF
#pragma config CP = OFF
#pragma config BOREN = ON
#pragma config CLKOUTEN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF

//CONFIG2
#pragma config WRT = OFF
#pragma config CPUDIV = NOCLKDIV
#pragma config USBLSCLK = 48MHz
#pragma config PLLMULT = 3x
#pragma config PLLEN = ENABLED
#pragma config STVREN = ON
#pragma config BORV = LO
#pragma config LPBOR = OFF
#pragma config LVP = OFF

#include "usb.h"
#include <xc.h>
#include <string.h>
#include "usb_config.h"
#include "usb_ch9.h"
#include "usb_cdc.h"
#include "usb_dfu.h"
#include "i2c.h"
#include "uart.h"
#include "jtag.h"

int main(void) {

#ifdef NO_BOOTLOADER
    OSCCON = 0b11111100;
    while ((OSCSTAT & 0b01010001) != 0b01010001);
    ACTSRC = 1;
    ACTEN = 1;
#endif

    ANSELA = 0;
    ANSELC = 0;

    JTAGInit();
    UARTInit();
    I2CInit();

    /* Configure interrupts, per architecture */
#ifdef USB_USE_INTERRUPTS
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
#endif

    usb_init();

#if DEBUGSERIAL ==  1
    DebugSerial("Start!\n", true);
#endif

    for (;;) {

#ifndef USB_USE_INTERRUPTS
        usb_service();
#endif

        /* if USB isn't configured, there is no point in proceeding further */
        if (!usb_is_configured()) continue;

#if DEBUGSERIAL != 1
        HandleUART();
#endif
        HandleJTAG();
        //I2C is handled in the moment a USB Packet arrives (usb_i2ctinyusb.c)
    }
}

//Interrupt routines are unused

void interrupt isr() {
#ifdef USB_USE_INTERRUPTS
    usb_service();
#endif
}
