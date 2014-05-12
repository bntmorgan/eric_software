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

define ELF_2_ROM
  $(foreach src,$(1),$(patsubst %elf,%rom,$(src)))
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
TOOLS = ../eric_tools
ERIC = ../eric
TFTPY = ../tftpy

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

targets: $(TOOLS)/bin2hex $(TOOLS)/mkmmimg $(TARGETS)

%.rom: %.bin
	$(TOOLS)/bin2hex $< $@ 16384 32
	cp $@ $(ERIC)/sources/cores/bram/rtl

%.bin: %.elf
	@echo [OC] $@
	@touch $@
	@$(OBJCOPY) $(TARGET_SEGMENTS) -O binary $< $@
	@echo -n "---- "
	@$(TOOLS)/mkmmimg $@ write

%.elf:
	@mkdir -p $(dir $@)
	@echo [LD] $@
	@$(LD) $(LDFLAGS) $(LD_OBJECTS) $(TARGET_LDFLAGS) -N -o $@ 

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

$(TOOLS)/%:
	@echo [TOOLS] $@
	@make -s -C $(TOOLS)

mr-proper: mr-proper-vim clean

mr-proper-vim:
	@echo [CLR] *.swp
	@find . | grep .swp | xargs rm -f

boot.bin: binary/bios-sstic/bios.bin
	@echo [CP] $^ $(TFTPY)/root/$@
	@cp $^ $(TFTPY)/root/$@

bios.rom: binary/bios/bios.rom
	@echo [CP] $^ $(ERIC)/sources/cores/bram/rtl/$@
	@cp $^ $(ERIC)/sources/cores/bram/rtl/$@
