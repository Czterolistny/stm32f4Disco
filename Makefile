TARGET:=stm32f4
PRJ_PATH=proj
PRJS = 9f5608\
		adc\
		cam\
		co\
		exti\
		i2c\
		ps2\
		pwm\
		spi\
    	tim\
		usart
		
TOOLCHAIN_ROOT:=/opt/gcc-arm-none-eabi
TOOLCHAIN_PATH:=$(TOOLCHAIN_ROOT)/bin
TOOLCHAIN_PREFIX:=arm-none-eabi

export COMMON_OBJ_DIR:=$(CURDIR)/$(PRJ_PATH)/build_obj

export CC:=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
export OBJCOPY:=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-objcopy
export AS:=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-as
export GDB:=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gdb
export SIZE:=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-size

export OPTLVL:=0
export DBG:=-g

export INCLUDE:=
INCLUDE=-I$(CURDIR)/hardware
INCLUDE+=-I$(CURDIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(CURDIR)/Libraries/CMSIS/Include
INCLUDE+=-I$(CURDIR)/Libraries/STM32F4xx_StdPeriph_Driver/inc
INCLUDE+=-I$(CURDIR)/config


HSE_CLK?=-DHSE_VALUE=8000000
CPU_CORE:=cortex-m4

export CDEFS:=
CDEFS:=-DUSE_STDPERIPH_DRIVER
CDEFS+=-DSTM32F4XX
CDEFS+=-DSTM32F40_41xxx
CDEFS+=$(HSE_CLK)
CDEFS+=-D__FPU_PRESENT=1
CDEFS+=-D__FPU_USED=0
CDEFS+=-DARM_MATH_CM4

export MCUFLAGS:=-mcpu=$(CPU_CORE) -mthumb -mfloat-abi=soft -mfpu=fpv4-sp-d16 -fsingle-precision-constant\
					-finline-functions -Wdouble-promotion -std=gnu99 --specs=nosys.specs

export COMMONFLAGS:=-O$(OPTLVL) $(DBG) -Wall -ffunction-sections -fdata-sections
export CFLAGS:=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS)

export LDLIBS:=
#export LDFLAGS:=$(MCUFLAGS) -u _scanf_float -u _printf_float -fno-exceptions -Wl,--gc-sections,

TARGETS=all clean

.PHONY: $(TARGETS)
default: all

$(TARGETS):
	[ -d $(COMMON_OBJ_DIR) ] || mkdir $(COMMON_OBJ_DIR)
	@for proj in $(PRJS) ; do $(MAKE) -C $(PRJ_PATH)/$$proj $@ ; done

$(PRJS):
	$(MAKE) -C  $(PRJ_PATH)/$@

substr = $(word $2,$(subst ., ,$1))

%.clean:
	@echo $@
	$(MAKE) -C $(PRJ_PATH)/$(call substr,$*,1)/$(call substr,$*,2) clean
	
%.flash:
	@echo $@
	$(MAKE) -C $(PRJ_PATH)/$(call substr,$*,1)/$(call substr,$*,2) flash
	
%.compile:
	@echo $@
	$(MAKE) -C $(PRJ_PATH)/$(call substr,$*,1)/$(call substr,$*,2)

clean_common:
	@echo [RM] COMMON_OBJ
	@rm -f $(COMMON_OBJ_DIR)/*.o

clean_all:
	@for proj in $(PRJS) ; do $(MAKE) -C $(PRJ_PATH)/$$proj clean ; done
	$(MAKE) clean_common

compile: $(PRJS)
	@echo "make" $@
