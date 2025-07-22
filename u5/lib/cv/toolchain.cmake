set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER arm-zephyr-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-zephyr-eabi-g++)
set(CMAKE_ASM_COMPILER arm-zephyr-eabi-gcc)

set(CMAKE_CXX_STANDARD 20)

set(ZEPHYR_BASE $ENV{ZEPHYR_BASE})

if(NOT ZEPHYR_BASE)
  message(FATAL_ERROR "ZEPHYR_BASE env variable is empty, do : source path/to/zephyr/zephyr-env.sh")
endif()

set(ZEPHYR_COMM_FLAGS "-fno-strict-aliasing \
  -Os \
  -mlong-calls \
  -Wno-frame-address \
  -fcheck-new \
  -std=c++${CMAKE_CXX_STANDARD} \
  -Wno-register \
  -Wno-volatile \
  -fno-common \
  -g \
  -gdwarf-4 \
  -fdiagnostics-color=always \
  -mcpu=cortex-m33 \
  -mthumb \
  -mabi=aapcs \
  -mfp16-format=ieee \
  -Wall \
  -Wformat \
  -Wformat-security \
  -Wno-format-zero-length \
  -Wpointer-arith \
  -Wexpansion-to-defined \
  -Wno-unused-but-set-variable \
  -fno-pic \
  -fno-pie \
  -fno-asynchronous-unwind-tables \
  -fno-reorder-functions \
  --param=min-pagesize=0 \
  -fno-defer-pop \
  -ffunction-sections \
  -fdata-sections\
  -imacros ${ZEPHYR_BASE}/include/zephyr/toolchain/zephyr_stdint.h")

#TODO : drag proper toolchain !
set(ZEPHYR_CXX_FLAGS "-std=c++${CMAKE_CXX_STANDARD} \
  -Wno-deprecated-enum-enum-conversion \
  -Wno-register \
  -Wno-volatile")

set(ZEPHYR_C_FLAGS "")

#-DSIZEOF_SIZE_T=4
set(COMM_FLAGS "-DSIZEOF_SIZE_T=4")

set(CMAKE_C_FLAGS "${ZEPHYR_COMM_FLAGS} ${ZEPHYR_C_FLAGS} ${COMM_FLAGS}" CACHE STRING "C Compiler Base Flags")
set(CMAKE_CXX_FLAGS "${ZEPHYR_COMM_FLAGS} ${ZEPHYR_CXX_FLAGS} ${COMM_FLAGS}" CACHE STRING "C++ Compiler Base Flags")

# Can be removed after gcc 5.2.0 support is removed (ref GCC_NOT_5_2_0)
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib" CACHE STRING "Linker Base Flags")