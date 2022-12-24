#!bin/python3

DEV=$1
esptool.py --baud 115200 --port $DEV write_flash 0x00000 boot_v1.1.bin
esptool.py --baud 115200 --port $DEV write_flash 0x01000 user1.bin
esptool.py --baud 115200 --port $DEV write_flash 0xfc000 esp_init_data_default.bin
esptool.py --baud 115200 --port $DEV write_flash 0xfe000 blank.bin
