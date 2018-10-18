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

#include "debug.h"

#if DEBUGSERIAL != -1

#define PrintSerial(x) do{while(!TXSTAbits.TRMT); TXREG = x;} while(0)

void _DebugSerial(const char* string, bool newline) {
    while (*string) {
        PrintSerial(*string);
        ++string;
    }
    if (newline) PrintSerial('\n');
}

void _DebugSerialInt(int32_t i, bool newline) {
    if (i < 0) {
        PrintSerial('-');
        i = -i;
    }
    if (i == 0) {
        PrintSerial('0');
        return;
    }
    int len = 0;
    for (int32_t k = 1; k <= i; k *= 10) {
        len++;
    }
    while (i > 0) {
        int32_t k;
        for (k = 1; k <= i; k *= 10);
        k /= 10;
        uint8_t val = (i / k) % 10;
        i -= k*val;
        val += '0';
        PrintSerial(val);
        len--;
    }
    for (len; len > 0; --len) {
        PrintSerial('0');
    }
    if (newline) PrintSerial('\n');
}

void _DebugSerialBytes(uint8_t* pData, size_t length, bool newline) {
    for (size_t t = 0; t < length; ++t) {
        PrintSerial('h');
        uint8_t b = pData[t];
        uint8_t f = b / 16;
        b -= (f * 16);
        PrintSerial(f < 10 ? '0' + f : 'A' + f - 10);
        f = b;
        PrintSerial(f < 10 ? '0' + f : 'A' + f - 10);
        PrintSerial(' ');
    }
    if (newline) PrintSerial('\n');
}

#endif
