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

#include <usb_config.h>
#include <usb.h>
#include <usb_i2ctinyusb.h>

static uint8_t reply[4];
static uint16_t length;
static uint16_t flags;
static uint8_t addr_rw;
static uint8_t cmd;
static uint8_t status = STATUS_IDLE;
static uint8_t buffer[I2C_BUFFER_LENGTH];
//static bool busy;

void I2CGetDataFromControlEndpoint() {
    if (cmd & I2CTINYUSB_CMD_I2C_IO_BEGIN) I2CStart();
    I2CWrite(addr_rw);
    __delay_ms(1);
    status = I2CAck() ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NAK;
    if (status == STATUS_ADDRESS_ACK) {
        for (size_t t = 0; t < length; ++t) {
            I2CWrite(buffer[t]);
            __delay_ms(1);
            status = I2CAck() ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NAK;
            if (status == STATUS_ADDRESS_NAK) break;
        }
    }
    if (cmd & I2CTINYUSB_CMD_I2C_IO_END || status == STATUS_ADDRESS_NAK) I2CStop();
}

int8_t process_i2ctinyusb_setup_request(struct setup_packet* setup) {

    DebugSerial("I2C Setup", true);
    uint8_t* data = (uint8_t*) setup;

    if (data[1] == I2CTINYUSB_CMD_ECHO) {
        reply[0] = data[2];
        reply[1] = data[3];
        usb_send_data_stage(reply, 2, 0, 0);
    } else if (data[1] == I2CTINYUSB_CMD_GET_FUNC) {
        uint32_t functionality = I2C_FUNC_TINYUSB;
        memcpy(reply, &functionality, 4);
        usb_send_data_stage(reply, 4, 0, 0);
    } else if (data[1] == I2CTINYUSB_CMD_SET_DELAY) {
        // This reply signals the linux driver that this interface is used for i2ctinyusb
        usb_send_data_stage(0, 0, 0, 0);
    } else if (data[1] == I2CTINYUSB_CMD_GET_STATUS) {
        usb_send_data_stage(&status, 1, 0, 0);
    } else if (data[1] & I2CTINYUSB_CMD_I2C_IO) {
        cmd = setup->bRequest;
        flags = setup->wValue;
        addr_rw = (setup->wIndex & 0xFF) << 1;
        (flags & I2C_M_RD) ? addr_rw |= 1 : addr_rw &= 0xFE;
        length = setup->wLength;
        if (length > I2C_BUFFER_LENGTH) length = I2C_BUFFER_LENGTH;

        if (length == 0) {
            if (cmd & I2CTINYUSB_CMD_I2C_IO_BEGIN) {
                I2CStart();
            }
            I2CWrite(addr_rw);
            __delay_ms(1);
            status = I2CAck() ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NAK;
            if (cmd & I2CTINYUSB_CMD_I2C_IO_END || status == STATUS_ADDRESS_NAK) {
                I2CStop();
            }
            usb_send_data_stage(0, 0, 0, 0);
        } else if ((length > 0) && ((flags & I2C_M_RD) == 0)) {
            usb_start_receive_ep0_data_stage(buffer, length, I2CGetDataFromControlEndpoint, 0);
        } else if (flags & I2C_M_RD) {
            if (cmd & I2CTINYUSB_CMD_I2C_IO_BEGIN) I2CStart();
            I2CWrite(addr_rw);
            __delay_ms(1);
            status = I2CAck() ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NAK;
            if (status == STATUS_ADDRESS_ACK) {
                for (size_t t = 0; t < length; ++t) {
                    buffer[t] = I2CRead(t != length - 1);
                }
            } else {
                //length = 0; // there is no device to read data from, but the driver read expects "length" bytes, so we send unknown data
            }
            if (cmd & I2CTINYUSB_CMD_I2C_IO_END) {
                I2CStop();
            }
            usb_send_data_stage(buffer, length, 0, 0);
        }

    } else {
        return -1;
    }
    return 0;
}
