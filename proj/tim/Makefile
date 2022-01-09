TARGET:=stm32f4_test
# TODO change to your ARM gcc toolchain path
TOOLCHAIN_ROOT:=/opt/gcc-arm-none-eabi
TOOLCHAIN_PATH:=$(TOOLCHAIN_ROOT)/bin
TOOLCHAIN_PREFIX:=arm-none-eabi

# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL:=s
DBG:=-g

STARTUP:=$(CURDIR)/../../hardware
LINKER_SCRIPT:=$(CURDIR)/../../Utilities/stm32_flash.ld

INCLUDE=-I$(CURDIR)/../../hardware
INCLUDE+=-I$(CURDIR)/../../Libraries/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(CURDIR)/../../Libraries/CMSIS/Include
INCLUDE+=-I$(CURDIR)/../../Libraries/STM32F4xx_StdPeriph_Driver/inc
INCLUDE+=-I$(CURDIR)/../../config
INCLUDE+=-I$(CURDIR)/../common

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary
COMMON_OBJ_DIR = $(CURDIR)/../build_obj

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/../../Libraries/STM32F4xx_StdPeriph_Driver/src $(CURDIR)/../../hardware  $(CURDIR)/../common

vpath %.s $(STARTUP)
ASRC=startup_stm32f4xx.s

# Project Source Files
SRC+=main.c

# Project Source Files
COMMON_SRC+=misc.c
COMMON_SRC+=stm32f4xx_it.c
COMMON_SRC+=system_stm32f4xx.c
COMMON_SRC+=dbg_pin.c

# Standard Peripheral Source Files
COMMON_SRC+=stm32f4xx_tim.c
COMMON_SRC+=stm32f4xx_usart.c
COMMON_SRC+=stm32f4xx_rcc.c
COMMON_SRC+=stm32f4xx_gpio.c

CDEFS=-DUSE_STDPERIPH_DRIVER
CDEFS+=-DSTM32F4XX
CDEFS+=-DSTM32F40_41xxx
CDEFS+=-DHSE_VALUE=8000000
CDEFS+=-D__FPU_PRESENT=1
CDEFS+=-D__FPU_USED=0
CDEFS+=-DARM_MATH_CM4

MCUFLAGS=-mcpu=cortex-m4 -mthumb -mfloat-abi=soft -mfpu=fpv4-sp-d16 -fsingle-precision-constant -finline-functions -Wdouble-promotion -std=gnu99 --specs=nosys.specs
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall -ffunction-sections -fdata-sections
CFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS)

LDLIBS=
LDFLAGS=$(MCUFLAGS) -u _scanf_float -u _printf_float -fno-exceptions -Wl,--gc-sections,-T$(LINKER_SCRIPT),-Map,$(BIN_DIR)/$(TARGET).map

CC=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
LD=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-objcopy
AS=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-as
AR=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-ar
GDB=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gdb
SIZE=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-size

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
COMMON_OBJ = $(COMMON_SRC:%.c=$(COMMON_OBJ_DIR)/%.o)

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(COMMON_OBJ_DIR)/%.o: %.c 
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

all: $(OBJ) $(COMMON_OBJ)
	@echo [AS] $(ASRC)
	@$(AS) -o $(ASRC:%.s=$(COMMON_OBJ_DIR)/%.o) $(STARTUP)/$(ASRC)
	@echo [LD] $(TARGET).elf
	@$(CC) -o $(BIN_DIR)/$(TARGET).elf $(LDFLAGS) $(OBJ) $(COMMON_OBJ) $(ASRC:%.s=$(COMMON_OBJ_DIR)/%.o) $(LDLIBS)
	@echo [HEX] $(TARGET).hex
	@$(OBJCOPY) -O ihex $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).hex
	@echo [BIN] $(TARGET).bin
	@$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).bin
	@$(SIZE) $(BIN_DIR)/$(TARGET).elf
	
.PHONY: clean

clean:
	@echo [RM] OBJ
	@rm -f $(OBJ)
	@rm -f $(ASRC:%.s=$(COMMON_OBJ_DIR)/%.o)
	@echo [RM] BIN
	@rm -f $(BIN_DIR)/$(TARGET).elf
	@rm -f $(BIN_DIR)/$(TARGET).hex
	@rm -f $(BIN_DIR)/$(TARGET).bin
	@rm -r $(BIN_DIR)/$(TARGET).map
	
clean_common:
	@echo [RM] COMMON_OBJ
	@rm -f $(COMMON_OBJ_DIR)/*.o
	
flash:
	@st-flash write $(BIN_DIR)/$(TARGET).bin 0x8000000
