sp              := $(sp).x
dirstack_$(sp)  := $(d)
d               := $(dir)

TARGET					:= $(call SRC_2_BIN, $(d)/libhal.a)
TARGETS 				+= $(TARGET)
OBJS_$(d)				:= $(call SRC_2_OBJ, $(d)/brd.o $(d)/time.o $(d)/sleep.o)

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
