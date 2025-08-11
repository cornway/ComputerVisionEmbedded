git submodule init --update

west init -l u5
west update

west build -b stm32u5g9j_dk2 u5/app/
west flash

###########################################

# To switch on tflite lib :
# in u5/app/prj.conf
# CONFIG_TFLITE_LIB=y
# That also enables example

###########################################

# Opencv library
# in u5/app/prj.conf :
# CONFIG_OPENCV_LIB=y

# Note: for opencv exceptions and rtti must be enabled

###########################################

west build -b stm32u5g9j_dk2 u5/app/
west flash
