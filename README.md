west init -l u5
west update

# WA for USB issue on this board
cd zephyr && git apply u5/patches/stm32u5g9j_dk2.patch
west build -b stm32u5g9j_dk2 u5/app/
west flash

# To switch on tflite lib :
# in u5/app/prj.conf
# CONFIG_TFLITE_LIB=y
# That also enables example