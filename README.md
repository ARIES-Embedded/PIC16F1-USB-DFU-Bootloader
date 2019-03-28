Firmware for PIC16F1454 on MX10 and Spider SoM
=====================================================

As base for development /example-apps/minimalCDC was used.

The firmware enables access to the bootloader, to serial, i2c and jtag interfaces.  
I2C protocol and driver derived from: [i2c-tiny-usb](https://github.com/harbaum/I2C-Tiny-USB)  
JTAG protocol derived from: [USB-Blaster](https://github.com/ntfreak/openocd/blob/master/src/jtag/drivers/usb_blaster/usb_blaster.c)  

# Bootloader
A bootloader exists to update the firmware using tools such as dfu-util.
```
dfu-util -d 04d8:efd0 -D firmware.dfu
```
Bootloader sources are under /firmware/
To update the bootloader itself an ICSP device (PICkit or similiar) is required as the bootloader region is write protected.

# Serial Numbers
The serial number is programmed into the bootloader. To retain the current serial number the bootloader source file has to be updated:
To find the serial number out use the command:
```
lsusb -v -d 04d8:efd0
```
then there is (example):
```
iSerial		1       SN0EXMPL
```
If the SN was for example: "SN0EXMPL" - in line 819 of "/firmware/bootloader.asm" change
```
SERIAL_NUMBER_STRING_DESCRIPTOR
dt	SERIAL_NUM_DESC_LEN	; bLength
dt	0x03		; bDescriptorType (STRING)
dt	'0'                , 0x00
dt	'0'                , 0x00
dt	'0'                , 0x00
dt	'0'                , 0x00
dt	'0'+SN1+((SN1>9)*7), 0x00	; convert hex digits to ASCII
dt	'0'+SN2+((SN2>9)*7), 0x00
dt	'0'+SN3+((SN3>9)*7), 0x00
dt	'0'+SN4+((SN4>9)*7), 0x00
```
to
```
SERIAL_NUMBER_STRING_DESCRIPTOR
dt	SERIAL_NUM_DESC_LEN	; bLength
dt	0x03		; bDescriptorType (STRING)
dt	'S', 0x00
dt	'N', 0x00
dt	'0', 0x00
dt	'E', 0x00
dt	'X', 0x00
dt	'M', 0x00
dt	'P', 0x00
dt	'L', 0x00
```
# Firmware
Firmware is located under /example-apps/aries-firmware  
To make use of the I2C interface the i2c-aries-emb driver (derived from [i2c-tiny-usb](https://github.com/harbaum/I2C-Tiny-USB)) is required.  
The JTAG interface is used with OpenOCD >=0.10
Compiled hex files are located under /example-apps/aries-firmware/hex-images/

# From PIC16F1-USB-DFU-Bootloader

For MPLAB/XC8 compilation, the following options are needed:

```
--codeoffset=0x200
--rom=default,-0-1FF,-1F7F-1F7F
```

The utility provided in the ./tools/ subdirectory converts a .hex file into a CRC-14 protected binary image:

```
454hex2dfu foo.hex foo.dfu
```

Downloading can be accomplished with the existing [dfu-util](http://dfu-util.sourceforge.net/) utilities:

```
dfu-util -D write.dfu
```

### Limitations

* A PIC16F145x chip of silicon revision A5 or later is required due to an issue with writing to program memory on revision A2 parts. The value at address 0x8006 in configuration space should be 0x1005 or later. See the [silicon errata document](http://ww1.microchip.com/downloads/en/DeviceDoc/80000546F.pdf) for more information.

* The configuration words are hard-coded in the bootloader (see the __config lines in bootloader.asm); the downloaded app inherits these settings and cannot invoke different values.

### License

The contents of this repository are released under a [3-clause BSD license](http://opensource.org/licenses/BSD-3-Clause).

