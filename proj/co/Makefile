PRJS = stm32f4_co \
		stm32f030_board_co

PRJ_PATH=

TARGET= all clean

.PHONY: $(TARGETS)
default: all

all:
	@for proj in $(PRJS) ; do $(MAKE) -C $(PRJ_PATH)$$proj $@ ; done
	
clean:
	@for proj in $(PRJS) ; do $(MAKE) -C $(PRJ_PATH)$$proj $@ ; done