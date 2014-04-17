MMDIR=../..
include $(MMDIR)/software-sstic/include.mak

OBJECTS=crt0.o isr.o main.o 
SEGMENTS=-j .text -j .data -j .rodata
LIBS=$(MMDIR)/software-sstic/libbase/libbase-light.a \
     $(MMDIR)/software-sstic/libhal/libhal.a     $(MMDIR)/software-sstic/libnet/libnet.a \
     $(MMDIR)/software-sstic/libnet/libchecker.a
BIOS_NETBOOT=0x$(shell lm32-rtems4.11-objdump ../../software/bios/bios.elf -t | grep netboot | cut -f 1 -d " ")

all: bios.bin

%.bin: %.elf
	$(MAKE) -C $(MMDIR)/tools mkmmimg
	$(OBJCOPY) $(SEGMENTS) -O binary $< $@
	chmod -x $@
	$(MMDIR)/tools/mkmmimg $@ write

bios.elf: linker.ld $(OBJECTS) $(LIBS)

%.elf:
	@echo bios_netboot address !! $(BIOS_NETBOOT)
	$(LD) $(LDFLAGS) -T $< -N -o $@ $(OBJECTS) -L$(MMDIR)/software-sstic/libhpdmc -L$(MMDIR)/software-sstic/libbase -L$(MMDIR)/software-sstic/libhal -L$(MMDIR)/software-sstic/libnet --start-group -lbase-light -lhal -lnet --end-group --defsym=bios_netboot=$(BIOS_NETBOOT)
	chmod -x $@

$(MMDIR)/software-sstic/libhpdmc/libhpdmc.a:
	make -C $(MMDIR)/software-sstic/libhpdmc/

$(MMDIR)/software-sstic/libbase/libbase-light.a:
	make -C $(MMDIR)/software-sstic/libbase/

$(MMDIR)/software-sstic/libhal/libhal.a:
	make -C $(MMDIR)/software-sstic/libhal/

$(MMDIR)/software-sstic/libhal/libchecker.a:
	make -C $(MMDIR)/software-sstic/checker/

$(MMDIR)/software-sstic/libnet/libnet.a:
	make -C $(MMDIR)/software-sstic/libnet/

.PHONY: clean depend $(LIBS)

depend:
	makedepend -Y -- $(CFLAGS) -- *.c

clean:
	rm -f *.o bios.elf bios.bin bios-rescue.elf bios-rescue.bin .*~ *~ Makefile.bak

# DO NOT DELETE

isr.o: ../../software-sstic/include/hw/interrupts.h
isr.o: ../../software-sstic/include/base/irq.h
isr.o: ../../software-sstic/include/base/uart.h
isr.o: ../../software-sstic/include/base/checker.h
isr.o: ../../software-sstic/include/hal/usb.h
isr.o: ../../software-sstic/include/hal/tmu.h
isr.o: ../../software-sstic/include/hw/tmu.h
isr.o: ../../software-sstic/include/hw/common.h
main.o: ../../software-sstic/include/base/stdio.h
main.o: ../../software-sstic/include/base/stdlib.h
main.o: ../../software-sstic/include/base/console.h
main.o: ../../software-sstic/include/base/string.h
main.o: ../../software-sstic/include/base/uart.h
main.o: ../../software-sstic/include/base/blockdev.h
main.o: ../../software-sstic/include/base/fatfs.h
main.o: ../../software-sstic/include/base/system.h
main.o: ../../software-sstic/include/base/board.h
main.o: ../../software-sstic/include/base/irq.h
main.o: ../../software-sstic/include/base/version.h
main.o: ../../software-sstic/include/base/crc.h
main.o: ../../software-sstic/include/net/mdio.h
main.o: ../../software-sstic/include/hw/fmlbrg.h
main.o: ../../software-sstic/include/hw/sysctl.h
main.o: ../../software-sstic/include/hw/common.h
main.o: ../../software-sstic/include/hw/gpio.h
main.o: ../../software-sstic/include/hw/flash.h
main.o: ../../software-sstic/include/hw/minimac.h
main.o: ../../software-sstic/include/hal/vga.h
main.o: ../../software-sstic/include/hal/tmu.h
main.o: ../../software-sstic/include/hw/tmu.h
main.o: ../../software-sstic/include/hal/brd.h
main.o: ../../software-sstic/include/hal/usb.h
main.o: ../../software-sstic/include/hal/ukb.h
main.o: ../../software-sstic/include/net/microudp.h
main.o: ../../software-sstic/include/base/checker.h
