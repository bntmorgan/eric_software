sp              := $(sp).x
dirstack_$(sp)  := $(d)
d               := $(dir)

TARGET					:= $(call SRC_2_BIN, $(d)/bios.elf)
TARGETS 				+= $(TARGET)
OBJS_$(d)				:= $(call SRC_2_OBJ, $(d)/crt0.o $(d)/isr.o $(d)/main.o)

OBJECTS 				+= $(OBJS_$(d))

$(OBJS_$(d))		:  INCLUDES_NOLIBC := -nostdinc -Isources/include/base
$(OBJS_$(d))		:  INCLUDES := $(INCLUDES_NOLIBC) -Isources/include \
	-Isources/tools
$(OBJS_$(d))		:  OBJ_CFLAGS	:= -I$(d) -Isources/includes -O0 -Wall -Werror \
	-Wstrict-prototypes -Wold-style-definition -Wshadow -mbarrel-shift-enabled \
	-mmultiply-enabled -mdivide-enabled -msign-extend-enabled -fno-builtin \
	-fsigned-char -fsingle-precision-constant $(INCLUDES)

$(TARGET)				:  TARGET_LDFLAGS	:= -T $(d)/linker.ld --start-group \
	-lbase-light -lhal -lnet --end-group -Lbinary/libbase -Lbinary/libhal \
	-Lbinary/libnet --defsym=bios_netboot=0x`$(OBJDUMP) binary/bios/bios.elf -t \
	| grep netboot | cut -f 1 -d " "`
$(TARGET)				:  $(OBJS_$(d)) binary/libbase/libbase-light.a \
	binary/libhal/libhal.a binary/libnet/libnet.a  binary/bios/bios.elf
$(TARGET)				:  LD_OBJECTS := $(OBJS_$(d))

d               := $(dirstack_$(sp))
sp              := $(basename $(sp))
