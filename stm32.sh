#!/bin/bash

stm32cpu=$1
stm32cpu=${stm32cpu:0:7}"x"
openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/$stm32cpu.cfg 
