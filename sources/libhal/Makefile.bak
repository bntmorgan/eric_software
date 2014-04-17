MMDIR=../..
include $(MMDIR)/software-sstic/include.mak

OBJECTS=brd.o # dmx.o font8x16.o mem.o pfpu.o snd.o time.o tmu.o ukb.o usb.o vga.o vin.o

all: libhal.a

libhal.a: $(OBJECTS)
	$(AR) clr libhal.a $(OBJECTS)
	$(RANLIB) libhal.a

$(MMDIR)/softusb-input/softusb-input.h:
	$(MAKE) -C $(MMDIR)/softusb-input

.PHONY: clean depend $(MMDIR)/softusb-input/softusb-input.bin

depend: $(MMDIR)/softusb-input/softusb-input.h
	makedepend -Y -- $(CFLAGS) -- *.c

clean:
	rm -f *.o libhal.a .*~ *~ Makefile.bak softusb-input.h

# DO NOT DELETE

brd.o: ../../software-sstic/include/base/stdlib.h
brd.o: ../../software-sstic/include/base/stdio.h
brd.o: ../../software-sstic/include/base/string.h
brd.o: ../../software-sstic/include/base/version.h
brd.o: ../../software-sstic/include/base/board.h
brd.o: ../../software-sstic/include/hw/sysctl.h
brd.o: ../../software-sstic/include/hw/common.h
brd.o: ../../software-sstic/include/hw/capabilities.h
brd.o: ../../software-sstic/include/hal/brd.h
dmx.o: ../../software-sstic/include/base/stdio.h
dmx.o: ../../software-sstic/include/base/stdlib.h
dmx.o: ../../software-sstic/include/hw/dmx.h
dmx.o: ../../software-sstic/include/hw/common.h
dmx.o: ../../software-sstic/include/hal/dmx.h
mem.o: ../../software-sstic/include/base/stdio.h
mem.o: ../../software-sstic/include/base/stdlib.h
mem.o: ../../software-sstic/include/base/malloc.h
mem.o: ../../software-sstic/include/hal/mem.h
pfpu.o: ../../software-sstic/include/base/stdio.h
pfpu.o: ../../software-sstic/include/base/stdlib.h
pfpu.o: ../../software-sstic/include/base/irq.h
pfpu.o: ../../software-sstic/include/base/system.h
pfpu.o: ../../software-sstic/include/hw/interrupts.h
pfpu.o: ../../software-sstic/include/hw/pfpu.h
pfpu.o: ../../software-sstic/include/hw/common.h
pfpu.o: ../../software-sstic/include/hal/pfpu.h
snd.o: ../../software-sstic/include/base/stdio.h
snd.o: ../../software-sstic/include/base/stdlib.h
snd.o: ../../software-sstic/include/base/irq.h
snd.o: ../../software-sstic/include/base/system.h
snd.o: ../../software-sstic/include/hw/ac97.h
snd.o: ../../software-sstic/include/hw/common.h
snd.o: ../../software-sstic/include/hw/interrupts.h
snd.o: ../../software-sstic/include/hw/sysctl.h
snd.o: ../../software-sstic/include/hw/capabilities.h
snd.o: ../../software-sstic/include/hal/snd.h
time.o: ../../software-sstic/include/base/stdio.h
time.o: ../../software-sstic/include/base/stdlib.h
time.o: ../../software-sstic/include/base/irq.h
time.o: ../../software-sstic/include/base/board.h
time.o: ../../software-sstic/include/hw/sysctl.h
time.o: ../../software-sstic/include/hw/common.h
time.o: ../../software-sstic/include/hw/interrupts.h
time.o: ../../software-sstic/include/hal/time.h
tmu.o: ../../software-sstic/include/base/stdio.h
tmu.o: ../../software-sstic/include/base/stdlib.h
tmu.o: ../../software-sstic/include/base/irq.h
tmu.o: ../../software-sstic/include/base/board.h
tmu.o: ../../software-sstic/include/hw/interrupts.h
tmu.o: ../../software-sstic/include/hw/sysctl.h
tmu.o: ../../software-sstic/include/hw/common.h
tmu.o: ../../software-sstic/include/hw/capabilities.h
tmu.o: ../../software-sstic/include/hw/tmu.h
tmu.o: ../../software-sstic/include/hal/brd.h
tmu.o: ../../software-sstic/include/hal/tmu.h
ukb.o: ../../software-sstic/include/base/stdio.h
ukb.o: ../../software-sstic/include/base/stdlib.h
ukb.o: ../../software-sstic/include/base/console.h
ukb.o: ../../software-sstic/include/hal/usb.h
ukb.o: ../../software-sstic/include/hal/ukb.h
usb.o: ../../software-sstic/include/hw/softusb.h
usb.o: ../../software-sstic/include/hw/common.h
usb.o: ../../software-sstic/include/hw/sysctl.h
usb.o: ../../software-sstic/include/hw/capabilities.h
usb.o: ../../software-sstic/include/hw/interrupts.h
usb.o: ../../software-sstic/include/base/uart.h
usb.o: ../../software-sstic/include/base/irq.h
usb.o: ../../software-sstic/include/base/stdio.h
usb.o: ../../software-sstic/include/base/stdlib.h
usb.o: ../../software-sstic/include/hal/usb.h input-comloc.h
usb.o: ../../softusb-input/softusb-input.h
vga.o: ../../software-sstic/include/base/stdio.h
vga.o: ../../software-sstic/include/base/stdlib.h
vga.o: ../../software-sstic/include/base/string.h
vga.o: ../../software-sstic/include/base/console.h
vga.o: ../../software-sstic/include/base/system.h
vga.o: ../../software-sstic/include/base/irq.h
vga.o: ../../software-sstic/include/hw/interrupts.h
vga.o: ../../software-sstic/include/hw/vga.h
vga.o: ../../software-sstic/include/hw/common.h
vga.o: ../../software-sstic/include/hal/tmu.h
vga.o: ../../software-sstic/include/hw/tmu.h
vga.o: ../../software-sstic/include/hal/vga.h
vin.o: ../../software-sstic/include/base/stdio.h
vin.o: ../../software-sstic/include/base/stdlib.h
vin.o: ../../software-sstic/include/hal/vin.h
vin.o: ../../software-sstic/include/hw/bt656cap.h
vin.o: ../../software-sstic/include/hw/common.h
