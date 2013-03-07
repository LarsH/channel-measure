TOPDIR := ..
THISMAKEFILE := $(TOPDIR)/src/Makefile

firstTarget: all

# setup prefixes for cross-compability
PREFIX := $(TOPDIR)/tools/arm-none-eabi-
SUFFIX := .exe

# Toolchain programs
CC := $(PREFIX)gcc$(SUFFIX)
LD := $(PREFIX)gcc$(SUFFIX)
AS := $(PREFIX)gcc$(SUFFIX)
AR = $(PREFIX)ar$(SUFFIX)
OBJCP := $(PREFIX)objcopy$(SUFFIX)

# Program flags
COMPILE_OPTS := -mcpu=cortex-m3 -mthumb -Wall -g -O0 -MD
COMPILE_OPTS += -B $(TOPDIR)/tools

INCLUDES := -I$(TOPDIR)/src -I$(TOPDIR)/src/lib/inc/

CFLAGS  := $(COMPILE_OPTS) $(INCLUDES)
ASFLAGS := $(COMPILE_OPTS) -c

# LDFLAGS should be = and not :=, $@ should be evaluated at link time
LDFLAGS = -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler $(INCLUDE_DIRS) $(LIBRARY_DIRS) -T $(TOPDIR)/src/stm32f103c8.ld #-T stm32.ld
LDFLAGS += -B $(TOPDIR)/tools -B $(TOPDIR)/lib

ARFLAGS := cr

OBJCPFLAGS := -O binary

vpath %.c  $(TOPDIR)/src
vpath %.c  $(TOPDIR)/src/lib/src
vpath %.s  $(TOPDIR)/src/lib/src
vpath %.ld $(TOPDIR)/src

# Object definitions
TARGET_OBJECTS := main.o

TARGET_NAME := channel-measure
TARGET_ELF  := $(TARGET_NAME).elf
TARGET_BIN  := $(TARGET_NAME).bin
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
all: $(TARGET_BIN)

%.o:%.c $(THISMAKEFILE)
	$(CC) $(CFLAGS) -c $<

%.o:%.s $(THISMAKEFILE)
	$(AS) $(ASFLAGS) $<

$(TARGET_ELF): $(TARGET_OBJECTS) $(THISMAKEFILE) $(LIBSTM32_OUT)
	$(LD) $(LDFLAGS) $(TARGET_OBJECTS) --output $@

%.bin: %.elf
	$(OBJCP) $(OBJCPFLAGS) $< $@

$(LIBSTM32_OUT): $(LIBSTM32_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBSTM32_OBJS)

# flash
.PHONY:flash
flash: $(MAIN_OUT)
	@cp $(MAIN_OUT_ELF) jtag/flash
	@cd jtag; openocd -f flash.cfg
	@rm jtag/flash

.PHONY:clean
clean:
	-@del $(TARGET_BIN) $(TARGET_ELF) $(ALLOBJS) $(DEPS) $(TARGET_MAP) $(LIBSTM32_OUT)

