sp              := $(sp).x
dirstack_$(sp)  := $(d)
d               := $(dir)

TARGET					:= $(call SRC_2_BIN, $(d)/libbase-light.a)
TARGETS 				+= $(TARGET)
OBJS_$(d)				:= $(call SRC_2_OBJ, $(d)/libc.o $(d)/crc32.o $(d)/console.o \
	$(d)/blockdev.o $(d)/fatfs.o $(d)/system.o $(d)/board.o $(d)/uart.o \
	$(d)/csr_ddr3.o $(d)/mpu.o $(d)/mpu_int.o $(d)/vsnprintf-nofloat.o)

OBJECTS 				+= $(OBJS_$(d))

$(OBJS_$(d))		:  INCLUDES_NOLIBC := -nostdinc -Isources/include/base
$(OBJS_$(d))		:  INCLUDES := $(INCLUDES_NOLIBC) -Isources/include \
	-Isources/tools
$(OBJS_$(d))		:  OBJ_CFLAGS	:= -I$(d) -Isources/includes -O0 -Wall -Werror \
	-Wstrict-prototypes -Wold-style-definition -Wshadow -mbarrel-shift-enabled \
	-mmultiply-enabled -mdivide-enabled -msign-extend-enabled -fno-builtin \
	-fsigned-char -fsingle-precision-constant $(INCLUDES)

$(TARGET)				:  TARGET_LDFLAGS	:=
$(TARGET)				:  $(OBJS_$(d))
$(TARGET)				:  LD_OBJECTS := $(OBJS_$(d))

d               := $(dirstack_$(sp))
sp              := $(basename $(sp))
