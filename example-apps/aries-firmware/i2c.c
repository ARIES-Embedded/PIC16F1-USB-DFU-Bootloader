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

#include "i2c.h"

void I2CInit() {

    //Set pins to digital mode
    ANSELCbits.ANSC0 = 0;
    ANSELCbits.ANSC1 = 0;

    SSPCON = 0b00101000;
    SSPCON2 = 0;
    SSPCON3 = 0;
    SSPADD = 119; // 48 MHz Oscillator -> Fosc / 4(SSPADD+1) = Data Rate -> 100 KHz
    SSPSTAT = 0b00010000;
    TRISC0 = 1;
    TRISC1 = 1;
    
}

void I2CStart() {
    I2CWait();
    SEN = 1;
}

void I2CRestart() {
    I2CWait();
    RSEN = 1;
}

void I2CStop() {
    I2CWait();
    PEN = 1;
}

bool I2CReady() {
    return !(SSPSTATbits.R_nW | SSPCON2bits.ACKEN | SSPCON2bits.PEN | SSPCON2bits.SEN | SSPCON2bits.RCEN | SSPCON2bits.RSEN);
}

void I2CWait() {
    __delay_us(10);
    while (!I2CReady());
}

void I2CWrite(uint8_t byte) {
    I2CWait();
    SSPBUF = byte;
}

bool I2CAck() {
    return (ACKSTAT) ? false : true;
}

uint8_t I2CRead(bool sendAck) {
    I2CWait();
    RCEN = 1;
    I2CWait();
    uint8_t res = SSPBUF;
    I2CWait();
    RCEN = 0;
    ACKDT = (sendAck) ? 0 : 1;
    ACKEN = 1;
    return res;
}
