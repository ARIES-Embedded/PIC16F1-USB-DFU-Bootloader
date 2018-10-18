# customize the following paths for your computer
CC = xc8
CHIP = 16F1454

CFLAGS = --chip=$(CHIP) -Q -G  --double=24 --float=24
CFLAGS += --rom=default,-0-1FF,-1F7F-1F7F
CFLAGS += --codeoffset=0x200
CFLAGS += --opt=default,+asm,-asmfile,+speed,-space,-debug --addrqual=ignore
CFLAGS += --mode=pro -N64 -I. --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 
CFLAGS += --runtime=default,+clear,+init,-keep,-no_startup,+osccal,-resetbits,-download,-stackcall,+clib

FIRMWARE_OBJS = usb.p1 usb_cdc.p1 usb_descriptors.p1 main.p1 usb_helpers.p1 debug.p1 jtag.p1 i2c.p1 usb_dfu.p1 usb_i2ctinyusb.p1 usb_winusb.p1 uart.p1

FIRMWARE_HDRS = usb_config.h

all: firmware.hex

firmware.hex: $(FIRMWARE_OBJS)
	$(CC) $(CFLAGS) -o./$@ $(FIRMWARE_OBJS)

%.p1: %.c $(FIRMWARE_HDRS) Makefile
	$(CC) --pass1 $(CFLAGS) -o./$@ $<

clean:
#	rm -f firmware.hex
	rm -f *.p1 *.d *.pre *.sym *.cmf *.cof *.hxl *.lst *.obj *.rlf *.sdb
	rm -f funclist
