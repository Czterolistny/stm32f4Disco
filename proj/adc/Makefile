TARGET:=stm32f4_$(shell echo $(CURDIR) | sed -e "s/.*\///")

# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL+=
DBG+=

STARTUP:=$(CURDIR)/../../hardware
LINKER_SCRIPT:=$(CURDIR)/../../Utilities/stm32f4.ld

BUILD_DIR = build
BIN_DIR = binary

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/../../Libraries/STM32F4xx_StdPeriph_Driver/src \
			$(CURDIR)/../../hardware $(CURDIR)/../common $(CURDIR)/../usart

vpath %.s $(STARTUP)
ASRC=startup_stm32f4xx.s

# Project Source Files
SRC+=main.c
SRC+=uart.c

COMMON_SRC+=stm32f4xx_it.c
COMMON_SRC+=system_stm32f4xx.c
COMMON_SRC+=dbg_pin.c

COMMON_SRC+=misc.c
# Standard Peripheral Source Files
COMMON_SRC+=stm32f4xx_adc.c
COMMON_SRC+=stm32f4xx_dma.c
COMMON_SRC+=stm32f4xx_usart.c
COMMON_SRC+=stm32f4xx_rcc.c
COMMON_SRC+=stm32f4xx_gpio.c

INCLUDE+=-I../common
INCLUDE+=-I../usart

MCUFLAGS+=
COMMONFLAGS+=
CFLAGS+=

LDLIBS+=
LDFLAGS=$(MCUFLAGS) -u _scanf_float -u _printf_float -fno-exceptions -Wl,--gc-sections,-T$(LINKER_SCRIPT),-Map,$(BIN_DIR)/$(TARGET).map

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
COMMON_OBJ = $(COMMON_SRC:%.c=$(COMMON_OBJ_DIR)/%.o)

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $(notdir $<)
	@[ -d $(BUILD_DIR) ] || mkdir $(BUILD_DIR)
	@[ -d $(BIN_DIR) ] || mkdir $(BIN_DIR)
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

debug:
	@$(DEFAULT_TERM) -e $(DBG_SCRIPT_DIR)/stm32.sh $TARGET &
	@$(GDB) $(BIN_DIR)/$(TARGET).elf --tui
