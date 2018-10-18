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

#include "jtag.h"

uint8_t JTAGBuffer[80] @ 0x2140; //data is saved only in bank 4

//static Variables used in assembler code - located in shared bank
static uint8_t OutLength @ 0x70;
static uint8_t JTAGBufferIndex @ 0x71;
static uint8_t staticVar3 @ 0x72;
static uint8_t staticVar4 @ 0x73;

#define MACRO_CONCAT_2(a,b) a ## b
#define MACRO_CONCAT(a,b) MACRO_CONCAT_2(a,b)

#define TMSPIN C2
#define TCKPIN C3
#define TDIPIN A4
#define TDOPIN A5

#define TMS MACRO_CONCAT(R, TMSPIN)
#define TCK MACRO_CONCAT(R, TCKPIN)
#define TDI MACRO_CONCAT(R, TDIPIN)
#define TDO MACRO_CONCAT(R, TDOPIN)

#define TRIS_TMS MACRO_CONCAT(TRIS, TMSPIN)
#define TRIS_TCK MACRO_CONCAT(TRIS, TCKPIN)
#define TRIS_TDI MACRO_CONCAT(TRIS, TDIPIN)
#define TRIS_TDO MACRO_CONCAT(TRIS, TDOPIN)

#define OUT 0
#define IN 1

void JTAGInit() {
    TRIS_TMS = OUT;
    TRIS_TCK = OUT;
    TRIS_TDI = OUT; //TDI is output for pic
    TRIS_TDO = IN; //TDO is input for pic
    JTAGBufferIndex = 0;
}

int8_t ProcessJTAGSetup(struct setup_packet* setup) {

    DebugSerial("FTDI Setup", true);
    //Currently: Acknowledge every type of request but send empty data
    //TODO: find out whether there a requests that have to be supported properly
    if (setup->REQUEST.bmRequestType == 0xC0) {
        usb_send_data_stage(0, 0, 0, 0);
    } else {
        usb_send_data_stage(0, 0, 0, 0);
    }
    return 0;
}

void JTAGAsm() {
    //FSR0 will iterate over out data containing commands (and shift data)
    //FSR1 will iterate over ReadBuffer writing values of TDO
    //Both start at 0x2140 (JTAGBuffer) + JTAGBufferIndex (@0x71)
    //After a command was processed it can be overwritten with TDO data
#asm
    //Switch to bank 0 to use PORT registers
    MOVLB 0
            //Move 0x2140 to File Select 0 and 0x2140+BufferIndex (@0x71) to File Select 1
            MOVLW 0x21
            MOVWF FSR0H
            MOVWF FSR1H
            MOVLW 0x40
            ADDWF 0x71, W
            MOVWF FSR0L
            MOVWF FSR1L
            //Compare length of data with 0 - Update Status Register (zero flag)
            MOVF 0x70, F
            _JtagAsm_Loop :
            //Check zero flag, if it is set return
            BTFSC STATUS, 2
            GOTO _JtagAsm_Return
            //Check bit 0 of data (bitbang or shift mode)
            BTFSC INDF0, 0
            GOTO _JtagAsmShiftMode
            _JtagAsmBitbang :
            //Check bit 1 of data (read TDO)
            BTFSS INDF0, 1
            GOTO _JtagAsmBitbang_SkipTDO
            //Read TDO in svar3 (temp)
            CLRF 0x72
            BTFSC PORTA, 5
            BSF 0x72, 7
            BSF 0x72, 6
            _JtagAsmBitbang_SkipTDO :
            //Set TMS accordingly
            BTFSS INDF0, 6
            BCF PORTC, 2
            BTFSC INDF0, 6
            BSF PORTC, 2
            //Set TDI
            BTFSS INDF0, 3
            BCF PORTA, 4
            BTFSC INDF0, 3
            BSF PORTA, 4
            //Set TCK
            BTFSS INDF0, 7
            BCF PORTC, 3
            BTFSC INDF0, 7
            BSF PORTC, 3
            //Byte is done, check if read TDO
            BTFSS INDF0, 1
            GOTO _JtagAsm_Bitbang_SkipWriteTDO
            //Write value to where FSR1 points to (ReadBuffer)
            MOVF 0x72, W
            MOVWF INDF1
            INCF FSR1L, F
            _JtagAsm_Bitbang_SkipWriteTDO :
            //Bitbang Loop Iteration Done
            INCF FSR0L, F
            DECF 0x70, F
            // Next command can be in either mode
            GOTO _JtagAsm_Loop

            _JtagAsmShiftMode :

            //Get byte length from command that has inverted bit sequence            
            CLRW
            BTFSC INDF0, 7
            BSF WREG, 0
            BTFSC INDF0, 6
            BSF WREG, 1
            BTFSC INDF0, 5
            BSF WREG, 2
            BTFSC INDF0, 4
            BSF WREG, 3
            BTFSC INDF0, 3
            BSF WREG, 4
            BTFSC INDF0, 2
            BSF WREG, 5
            MOVWF 0x72
            //0x72 contains length of data shift operation
            MOVF 0x72, F //probably useless instruction?
            //byte shift handles length+1 bytes of command buffer
            DECF 0x70, F
            SUBWF 0x70, F
            MOVF INDF0, W
            INCF FSR0L, F
            //Test if TDO has to be read
            BTFSC WREG, 1
            GOTO _JtagAsmShiftMode_TDO //TDO and !TDO code are similar however this duplicated code is way faster, at the cost of increased code size, since there is only 1 if check now

            _JtagAsmShiftMode_NotTDO :
            BTFSC STATUS, 2 //check if loop is finished
            GOTO _JtagAsmShiftMode_Return

            //Shift first bit from left and generate clock pulse
            BTFSC INDF0, 7
            BSF PORTA, 4
            BTFSS INDF0, 7
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 2nd bit from left and generate clock pulse
            BTFSC INDF0, 6
            BSF PORTA, 4
            BTFSS INDF0, 6
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 3rd bit from left and generate clock pulse
            BTFSC INDF0, 5
            BSF PORTA, 4
            BTFSS INDF0, 5
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 4th bit from left and generate clock pulse
            BTFSC INDF0, 4
            BSF PORTA, 4
            BTFSS INDF0, 4
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 5th bit from left and generate clock pulse
            BTFSC INDF0, 3
            BSF PORTA, 4
            BTFSS INDF0, 3
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 6th bit from left and generate clock pulse
            BTFSC INDF0, 2
            BSF PORTA, 4
            BTFSS INDF0, 2
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 7th bit from left and generate clock pulse
            BTFSC INDF0, 1
            BSF PORTA, 4
            BTFSS INDF0, 1
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift last bit from left and generate clock pulse
            BTFSC INDF0, 0
            BSF PORTA, 4
            BTFSS INDF0, 0
            BCF PORTA, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Move iteration forward
            INCF FSR0L, F
            DECF 0x72, F
            GOTO _JtagAsmShiftMode_NotTDO

            _JtagAsmShiftMode_TDO :
            BTFSC STATUS, 2 //Zero bit
            GOTO _JtagAsmShiftMode_Return

            //Clear WREG as TDO values will be read there
            CLRW

            //Shift 1st bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 7
            BSF PORTA, 4
            BTFSS INDF0, 7
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 7
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 2st bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 6
            BSF PORTA, 4
            BTFSS INDF0, 6
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 6
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 3rd bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 5
            BSF PORTA, 4
            BTFSS INDF0, 5
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 5
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 4th bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 4
            BSF PORTA, 4
            BTFSS INDF0, 4
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 4
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 5th bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 3
            BSF PORTA, 4
            BTFSS INDF0, 3
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 3
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 6th bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 2
            BSF PORTA, 4
            BTFSS INDF0, 2
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 2
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 7th bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 1
            BSF PORTA, 4
            BTFSS INDF0, 1
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 1
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Shift 8th bit from left, read TDO (Pin RA5) and generate clock pulse
            BTFSC INDF0, 0
            BSF PORTA, 4
            BTFSS INDF0, 0
            BCF PORTA, 4
            NOP
            BTFSC PORTA, 5
            BSF WREG, 0
            NOP
            BSF PORTC, 3
            NOP
            BCF PORTC, 3
            NOP

            //Save WREG to ReadBuffer & move iteration forward
            MOVWF INDF1, F
            INCF FSR0L, F
            INCF FSR1L, F
            DECF 0x72, F
            GOTO _JtagAsmShiftMode_TDO

            _JtagAsmShiftMode_Return :
            //Update Status (zero flag) with remaining command length == 0
            MOVF 0x70, F
            GOTO _JtagAsm_Loop

            _JtagAsm_Return :
            //Update ReadBuffer index
            MOVLW 0x40
            SUBWF FSR1L, W
            MOVWF 0x71
            RET
#endasm
}

void HandleJTAG() {

    uint8_t* out;
    uint8_t* in;

_BeginHandleFTDI:

    if (!(usb_in_endpoint_busy(1) || usb_in_endpoint_halted(1)) && JTAGBufferIndex) {

        in = usb_get_in_buffer(1);
        uint8_t transmitSize = JTAGBufferIndex;
        // When sending data back to host, the first 2 bytes are always (unused) status bytes of the FTDI chip
        // Therefore a IN transfer can only contain 62 data bytes
        if (transmitSize > 62) transmitSize = 62;
        memcpy(in + 2, JTAGBuffer, transmitSize);
        usb_send_in_buffer(1, transmitSize + 2);
        JTAGBufferIndex -= transmitSize;
        //Move data located higher in the buffer to the start of buffer
        memcpy(JTAGBuffer, JTAGBuffer + transmitSize, JTAGBufferIndex);

        //To speed up JTAG operations, do not return to main loop while there was work to be done
        goto _BeginHandleFTDI;
    }

    if ((JTAGBufferIndex < 10) && usb_out_endpoint_has_data(2)) {

        OutLength = usb_get_out_buffer(2, &out);
        //Copy data to JTAGBuffer so the USB Buffer can be rearmed
        memcpy(JTAGBuffer + JTAGBufferIndex, out, OutLength);
        usb_arm_out_endpoint(2);

        JTAGAsm();

        //To speed up JTAG operations, do not return to main loop while there was work to be done
        goto _BeginHandleFTDI;

    }
}
