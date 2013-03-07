TOPLEVEL := ..
THISMAKEFILE := $(TOPLEVEL)/src/Makefile

firstTarget: all

# setup prefixes for cross-compability
PREFIX := $(TOPLEVEL)/tools/arm-none-eabi-
SUFFIX := .exe

# Toolchain programs
CC := $(PREFIX)gcc$(SUFFIX)
LD := $(PREFIX)gcc$(SUFFIX)
AS := $(PREFIX)gcc$(SUFFIX)
AR = $(PREFIX)ar$(SUFFIX)
OBJCP := $(PREFIX)objcopy$(SUFFIX)

# Program flags
COMPILE_OPTS := -mcpu=cortex-m3 -mthumb -Wall -g -O0 -MD
COMPILE_OPTS += -B $(TOPLEVEL)/tools

INCLUDES := -I$(TOPLEVEL)/src -I$(TOPLEVEL)/src/lib/inc/

CFLAGS  := $(COMPILE_OPTS) $(INCLUDES)
ASFLAGS := $(COMPILE_OPTS) -c

# LDFLAGS should be = and not :=, $@ should be evaluated at link time
LDFLAGS = -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler $(INCLUDE_DIRS) $(LIBRARY_DIRS) -T $(TOPLEVEL)/src/stm32f103c8.ld #-T stm32.ld
LDFLAGS += -B $(TOPLEVEL)/tools -B $(TOPLEVEL)/lib

ARFLAGS := cr

OBJCPFLAGS := -O binary

vpath %.c  $(TOPLEVEL)/src
vpath %.c  $(TOPLEVEL)/src/lib/src
vpath %.s  $(TOPLEVEL)/src/lib/src
vpath %.ld $(TOPLEVEL)/src

# Object definitions
TARGET_OBJECTS := main.o

TARGET_NAME := channel-measure
TARGET_ELF  := $(TARGET_NAME).elf
TARGET_BIN  := $(TARGET_NAME).bin

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

-include $(ALLOBJS:.o=.d)


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
	@rm *.o $(LIBSTM32_OUT) $(MAIN_OUT_ELF) $(MAIN_OUT_BIN)

