################################## FUNCTIONS ###################################

define SRC_2_OBJ
  $(foreach src,$(1),$(patsubst sources/%,build/%,$(src)))
endef

define SRC_2_BIN
  $(foreach src,$(1),$(patsubst sources/%,binary/%,$(src)))
endef

define ELF_2_BIN
  $(foreach src,$(1),$(patsubst %elf,%bin,$(src)))
endef

################################## STARTING RULE ###############################

all: targets

################################## GLOBALS  ####################################

CROSS_COMPILER = /opt/rtems-4.11/bin/lm32-rtems4.11-
CC = $(CROSS_COMPILER)gcc
AR = $(CROSS_COMPILER)ar
AS = $(CROSS_COMPILER)as
LD = $(CROSS_COMPILER)ld
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILER)objdump
RANLIB  = $(CROSS_COMPILER)ranlib

CFLAGS = 
LDFLAGS = -nostdlib -nodefaultlibs
ASFLAGS = $(INCLUDES) -nostdinc

################################## INCLUDES ####################################

# Overriden in rules.mk
TARGETS :=
OBJECTS :=

dir	:= sources
include	$(dir)/rules.mk

################################## RULES #######################################

targets: $(TARGETS)

%.bin: %.elf

%.elf:
	@mkdir -p $(dir $@)
	@echo [LD] $@
	@$(LD) $(LD_OBJECTS) $(LDFLAGS) $(TARGET_LDFLAGS) -o $@ 

build/%.o: sources/%.c
	@mkdir -p $(dir $@)
	@echo [CC] $@
	@$(CC) -o $@ -c $< $(CFLAGS) $(OBJ_CFLAGS)

build/%.o: sources/%.S
	@mkdir -p $(dir $@)
	@echo [CC] $@
	@$(CC) -o $@ -c $< $(CFLAGS) $(OBJ_CFLAGS)

%.so:
	@mkdir -p $(dir $@)
	@echo [LD] $@
	@$(LD) -shared -o $@ $^ $(TARGET_LDFLAGS)

%.a:
	@mkdir -p $(dir $@)
	@echo [AR] $@
	@$(AR) clr $@ $^
	@$(RANLIB) $@

info:
	@echo TARGETS [$(TARGETS)]
	@echo OBJECTS [$(OBJECTS)]

clean:
	@echo [CLR] $(TARGETS)
	@echo [CLR] $(OBJECTS)
	@rm -fr $(dir $(TARGETS)) $(OBJECTS)

mr-proper: mr-proper-vim clean

mr-proper-vim:
	@echo [CLR] *.swp
	@find . | grep .swp | xargs rm -f
