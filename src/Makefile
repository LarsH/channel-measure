TOPDIR := ..
THISMAKEFILE := $(TOPDIR)/src/Makefile

firstTarget: all

$(if $(filter %src,$(PWD)),$(error "Should not run make from src"))

# setup prefixes for cross-compability
PREFIX := $(TOPDIR)/tools/arm-none-eabi-
SUFFIX := .exe

# Toolchain programs
CC := $(PREFIX)gcc$(SUFFIX)
LD := $(PREFIX)gcc$(SUFFIX)
AS := $(PREFIX)gcc$(SUFFIX)
AR = $(PREFIX)ar$(SUFFIX)
OBJCP := $(PREFIX)objcopy$(SUFFIX)
LINT := $(TOPDIR)/tools/splint.exe

# Program flags
LINTFLAGS := -larchpath $(TOPDIR)/tools/splintlib -maintype -duplicatequals -I $(TOPDIR)/src/lib/inc/ -I $(TOPDIR)/src/

COMPILE_OPTS := -mcpu=cortex-m3 -mthumb -Wall -Werror -Wno-main -pedantic -g -O0 -MD
COMPILE_OPTS += -B $(TOPDIR)/tools

INCLUDES := -I$(TOPDIR)/src -I$(TOPDIR)/src/lib/inc/

CFLAGS  := $(COMPILE_OPTS) $(INCLUDES)
ASFLAGS := $(COMPILE_OPTS) -c

LINKERSCRIPT := $(TOPDIR)/src/stm32.ld
# LDFLAGS should be = and not :=, $@ should be evaluated at link time
LDFLAGS = -Wl,--gc-sections,-Map=$@.map,-cref,-u,g_pfnVectors $(INCLUDE_DIRS)
LDFLAGS += $(LIBRARY_DIRS) -T $(LINKERSCRIPT) -B$(TOPDIR)/src/lib -nostdlib
LDFLAGS += -B $(TOPDIR)/tools -B $(TOPDIR)/lib -nodefaultlibs -nostartfiles

ARFLAGS := cr

OBJCPFLAGS := -O ihex

# Virtual paths

vpath %.c  $(TOPDIR)/src
vpath %.c  $(TOPDIR)/src/lib/src
vpath %.s  $(TOPDIR)/src/lib/src
vpath %.ld $(TOPDIR)/src

# Object definitions
# XXX New C-files are added here!
TARGET_CFILES := main.c hw.c uartio.c stm32f10x_it.c fatfs_diskio_sdcard_spi.c
TARGET_OBJECTS := $(TARGET_CFILES:.c=.o)

TARGET_NAME := channel-measure
TARGET_ELF  := $(TARGET_NAME).elf
TARGET_HEX  := $(TARGET_NAME).hex
TARGET_MAP  := $(TARGET_ELF).map

# libstm32.a
LIBSTM32_OUT = libstm32.a
LIBSTM32_OBJS := stm32f10x_adc.o  stm32f10x_bkp.o \
 stm32f10x_can.o stm32f10x_dma.o stm32f10x_exti.o \
 stm32f10x_flash.o stm32f10x_gpio.o  stm32f10x_i2c.o \
 stm32f10x_iwdg.o stm32f10x_lib.o stm32f10x_nvic.o \
 stm32f10x_pwr.o stm32f10x_rcc.o stm32f10x_rtc.o \
 stm32f10x_spi.o stm32f10x_systick.o stm32f10x_tim.o \
 stm32f10x_tim1.o stm32f10x_usart.o stm32f10x_wwdg.o \
 cortexm3_macro.o stm32f10x_vector.o


# Dependencies
ALLOBJS := $(LIBSTM32_OBJS) $(TARGET_OBJECTS)
DEPS := $(ALLOBJS:.o=.d)
-include $(DEPS)


# Target definitions
all: $(TARGET_HEX)

%.o:%.c $(THISMAKEFILE)
	$(CC) $(CFLAGS) -c $<

%.o:%.s $(THISMAKEFILE)
	$(AS) $(ASFLAGS) $<

$(TARGET_ELF): lint $(TARGET_OBJECTS) $(THISMAKEFILE) $(LIBSTM32_OUT) $(LINKERSCRIPT)
	$(LD) $(LDFLAGS) $(TARGET_OBJECTS) $(LIBSTM32_OUT) --output $@

%.hex: %.elf
	$(OBJCP) $(OBJCPFLAGS) $< $@

$(LIBSTM32_OUT): $(LIBSTM32_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBSTM32_OBJS)

lint: $(TARGET_CFILES)
	$(LINT) $(LINTFLAGS) $^
	@echo > $@

# flash
.PHONY:flash
flash: $(MAIN_OUT)
	@cp $(MAIN_OUT_ELF) jtag/flash
	@cd jtag; openocd -f flash.cfg
	@rm jtag/flash

.PHONY:clean
clean:
	-@del $(TARGET_HEX) $(TARGET_ELF) $(ALLOBJS) $(DEPS) $(TARGET_MAP) $(LIBSTM32_OUT) lint

