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

#ifndef DEBUG_H
#define	DEBUG_H

#include "config.h"

#include <xc.h>
#include <stdint.h> //uint8_t
#include <stdbool.h> //bool
#include <stdlib.h> //size_t
#include <string.h> //strlen


#if DEBUGSERIAL != 0 && DEBUGSERIAL != 1 && DEBUGSERIAL != -1
#error DEBUGSERIAL macro is neither 1, 0  or -1. Use #define DEBUGSERIAL 1 in \
config.h to enable debug statements through uart, #define DEBUGSERIAL 0 to \
disable them or #define DEBUGSERIAL -1 to remove them through the preprocessor. \
(if the compiler does not optimize unreachable code away)
#endif

#if DEBUGSERIAL == (-1)

//Prints a string to UART
#define DebugSerial(string, newline)

//Prints an int as string to UART
#define DebugSerialInt(i, newline)

//Prints data pointed to as hexadecimal numbers (ex. h61 for 'A')
#define DebugSerialBytes(pData, length, newline)

#else
#define DebugSerial(string, newline) do { if(DEBUGSERIAL) _DebugSerial(string, newline); } while(0)
#define DebugSerialInt(i, newline) do { if(DEBUGSERIAL) _DebugSerialInt(i, newline); } while(0)
#define DebugSerialBytes(pData, length, newline) do { if(DEBUGSERIAL) _DebugSerialBytes(pData, length, newline); } while(0)
void _DebugSerial(const char* string, bool newline);
void _DebugSerialInt(int32_t i, bool newline);
void _DebugSerialBytes(uint8_t* pData, size_t length, bool newline);
#endif /* DEBUGSERIAL  == (-1) */

#endif	/* DEBUG_H */

