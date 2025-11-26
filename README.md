## This is a draft playground repository to check opencv capabilities with zephyr rtos in embedded world
Currently it is only tested with stm32u5xx boards, as it takes ~3mb of rom/flash

For a fresh start
* Clone this repo
* `git submodule init --update`
* `west init -l u5`
* `west update`
# Outdated
* `west build -b stm32u5g9j_dk2 u5/app/` (builds for stm32u5g9j_dk2 board, you can google what that board is)
* `west flash`

### Important note: since stm32u5g9j_dk2 doesn't have camera sensor, you need to find out your own way how to transfer images to the board; I used uart for that purpose

### Apply stm32hal.patch, in case -O2 (CONFIG_SPEED_OPTIMIZATIONS=y) is used
* `modules/hal/stm32/`
* `git apply ../../../stm32hal.patch`

## To enable OpenCV library and example code
* in `u5/app/prj.conf` : CONFIG_OPENCV_LIB=y

Note: there are maybe more dependencies, so watch out

## Also possible to use tflite micro
* in `u5/app/prj.conf` : CONFIG_TFLITE_LIB=y

### Planning to add support for stm32n6 boards
### there are more custom boards based on stm32u5xx family, PCB repository is coming
If you are willing to roll out your own board, please don't hesitate to contact me


### Note: this project is in it's very very early phase
