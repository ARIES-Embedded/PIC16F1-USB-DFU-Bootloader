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

/* This file only provides library like functions to use the i2c module.
 * The handling of i2c commands is done in usb_i2ctinyusb.h
 */

#ifndef I2C_H
#define	I2C_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h"

void I2CInit();
void I2CStart();
void I2CRestart();
void I2CStop();
bool I2CReady();
void I2CWait();
void I2CWrite(uint8_t byte);
bool I2CAck();
uint8_t I2CRead(bool sendAck);

#endif	/* I2C_H */

