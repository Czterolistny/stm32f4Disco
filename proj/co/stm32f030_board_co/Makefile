TARGET:=$(shell echo $(CURDIR) | sed -e "s/.*\///")

# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL+=
DBG+=

STARTUP:=$(CURDIR)/../../../hardware
LINKER_SCRIPT:=$(CURDIR)/../../../Utilities/stm32f030.ld

BUILD_DIR = build
BIN_DIR = binary

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/../../../Libraries/STM32F0xx_StdPeriph_Driver/src \
			$(CURDIR)/../../../hardware $(CURDIR)/../../common $(CURDIR)/src \
			$(CURDIR)/../stm32f4_co

vpath %.s $(STARTUP)
ASRC=startup_stm32f030xc.s

# Project Source Files
SRC+=main.c
SRC+=sregs.c
SRC+=debugPins.c
SRC+=swuart.c
SRC+=esp.c
SRC+=gdisp.c
SRC+=gdispFonts.c
SRC+=i2c.c
SRC+=flash.c

# Project Source Files
COMMON_SRC+=stm32f0xx_it.c
COMMON_SRC+=stm32f0xx_misc.c
COMMON_SRC+=system_stm32f0xx.c
COMMON_SRC+=common.c

# Standard Peripheral Source Files
COMMON_SRC+=stm32f0xx_syscfg.c
COMMON_SRC+=stm32f0xx_tim.c
COMMON_SRC+=stm32f0xx_usart.c
COMMON_SRC+=stm32f0xx_rcc.c
COMMON_SRC+=stm32f0xx_gpio.c
COMMON_SRC+=stm32f0xx_exti.c
COMMON_SRC+=stm32f0xx_i2c.c
COMMON_SRC+=stm32f0xx_spi.c

# Standard Peripheral Header Files
INCLUDE+=-I$(CURDIR)/../../../Libraries/CMSIS/Device/ST/STM32F0xx/Include
INCLUDE+=-I$(CURDIR)/../../../Libraries/STM32F0xx_StdPeriph_Driver/inc

INCLUDE+=-Iinc/
INCLUDE+=-I$../../common
INCLUDE+=-I$../../usart
INCLUDE+=-I../stm32f4_co

HSE_CLK=-DHSE_VALUE=12000000
CPU_CORE=cortex-m0

CFLAGS:=$(CFLAGS:cortex-m4=$(CPU_CORE))
MCUFLAGS:=$(MCUFLAGS:cortex-m4=$(CPU_CORE))

CFLAGS:=$(CFLAGS:8000000=12000000)
CFLAGS:=$(CFLAGS:DSTM32F4XX=DSTM32F0XX)
CFLAGS:=$(CFLAGS:DSTM32F40_41xxx=DSTM32F030xC)

CDEFS=-DUSE_STDPERIPH_DRIVER
#CDEFS+=-DSTM32F0XX
#CDEFS+=-DSTM32F030xC
CDEFS+=-D__FPU_PRESENT=1
CDEFS+=-D__FPU_USED=0
CDEFS+=

MCUFLAGS+=
COMMONFLAGS+=
CFLAGS+= $(INCLUDE) $(CDEFS)

LDLIBS+=
LDFLAGS=$(MCUFLAGS) -u _scanf_float -u _printf_float -fno-exceptions -Wl,--gc-sections,-T$(LINKER_SCRIPT),-Map,$(BIN_DIR)/$(TARGET).map

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
COMMON_OBJ = $(COMMON_SRC:%.c=$(COMMON_OBJ_DIR)/%.o)

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $(notdir $<)
	[ -d $(BUILD_DIR) ] || mkdir $(BUILD_DIR)
	[ -d $(BIN_DIR) ] || mkdir $(BIN_DIR)
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
	@rm -f $(BIN_DIR)/$(TARGET).*
	
clean_common:
	@echo [RM] COMMON_OBJ
	@rm -f $(COMMON_OBJ_DIR)/*.o
	
flash:
	@st-flash write $(BIN_DIR)/$(TARGET).bin 0x8000000
	@st-flash reset

debug:
	@$(DEFAULT_TERM) -e $(DBG_SCRIPT_DIR)/f030.sh &
	@$(GDB) $(BIN_DIR)/$(TARGET).elf -x script.gdb --tui