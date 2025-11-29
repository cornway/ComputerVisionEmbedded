## This repository contains a bunch of projects, primarily targeting stm32 based platforms
### The idea behind it - is to creatre unified space where you can build and start your own projects targeting Computer Vision and AI on the edge

Current version supports 3 customized boards, each board has well known predecessor \
Boards schematics and pcb's can be found [Here](https://github.com/cornway/ComputerVisionHw)\
Having this boards schematics you can easily figure out and adopt examples found here to your own hardware

For a fresh start
* Clone this repo
* `git submodule init --update`
* `west init -l u5`
* `west update`

### Apply stm32hal.patch, in case -O2 (CONFIG_SPEED_OPTIMIZATIONS=y) is used
* `cd modules/hal/stm32/`
* `git apply ../../../stm32hal.patch`

## Applicatons:
* **u5/uvc_gf_dk1/** - Simple one that utilizes UVC USB class to given board into a USB camera; only works for **grinreflex_dk1** board
* **u5/gf_dkx/** - unified application, can be used for all boards (so far), has two targets - face detection, and face + smile detection using opencv
* **uvc_gf_dk1/** - example video streaming application, available only for **grinreflex_n6_dk1** board

## Boards supported:
* **grinreflex_dk1** - uses schematic from [HW](https://github.com/cornway/ComputerVisionHw) repo **stm32u5_v1.0**
  * Extrnal flash
  * USB
  * OV2640 compatible camera
  * LCD serial display
  * A few GPIOS
  * ...
* **grinreflex_dk2** - counterpart of **stm32u5_v2.0** in [HW](https://github.com/cornway/ComputerVisionHw) repo
  * Simplified version of v1.0, unfortunatelly I put there MCU that seems to went out of stock
  * All hardware fetures from V1.0, except - there is no external flash and USB
* **grinreflex_n6_dk1** - most complicated so far, uses **stm32n6_v1.0** schematic in [HW] (https://github.com/cornway/ComputerVisionHw)
  * Hw Features
    * LCD serial display
    * OV2640 compatible camera
    * Eternal flash
    * External vddcore regulator
    * Boot switch
    * GPIO extension connector
  * Build variants (Just provide this option after **-board** to west command)
    * **grinreflex_n6_dk3//app** - XIP application, requires **--sysbuild** option, loaded by fsbl
    * **grinreflex_n6_dk3//fsbl_app** - application that is a part of fsbl
    * **grinreflex_n6_dk3//fsbl** - fsbl, usually required when you specify **--sysbuild**, that builds mcuboot
    * Example: build XIP application \
    `west build -b grinreflex_n6_dk3//app --sysbuild u5/gf_n6_dk3/` \
That board pretty much derived from [Nucleo](https://docs.zephyrproject.org/latest/boards/st/nucleo_n657x0_q/doc/index.html), so you can use their tutorial to obtain information on how to build/run/debug

## Configuration options
* **CONFIG_OPENCV_LIB** - if **y**, adds OpenCv library, please see `u5/lib/cv/CMakeLists.txt` to see which modules are actually built
* **CONFIG_TFLITE_LIB** - if **y**, adds TfLite framework, the performance remains poor as no optimizations are done yet
* **CONFIG_QUIRC_LIB** - say **y** to Add [this](https://github.com/dlbeer/quirc)
* **CONFIG_NEMA** - enables **NEMA** or stm32's **GPU2.5**, no modifications required to **lvgl** (Yes, that works only as a part of lvgl so far), that sometimes significantly speeds up graphic manipulations
* **CONFIG_STM32_JPEG** - pretty much **rough** stm32's jpeg driver, tested only on **u5** family so far


### Note: while playing yo may face weird issues, where configuration is not suppoorted or missing, please feel free to submit an issue; Maintainers are very welcome