#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

message(FATAL_ERROR This application is deprecated, please use uvc_gf_dk1 / gf_dkx / ...)

set(EXTRA_CONF_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl.conf;
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app)

# TODO: below two lines are unclear
set(NO_THREADSAFE_STATICS $<TARGET_PROPERTY:compiler-cpp,no_threadsafe_statics>)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:${NO_THREADSAFE_STATICS}>)

if (CONFIG_OPENCV_LIB)
    include(opencv.cmake)
endif()

if (CONFIG_NEMA)
    include(nema.cmake)
endif()

target_include_directories(app PRIVATE
    src
)

target_sources(app PRIVATE
    src/main.c
    src/gfx_utils.cpp
    src/video.cpp
)

if (CONFIG_BOARD_GRINREFLEX_DK1 OR CONFIG_BOARD_GRINREFLEX_DK2)
target_sources(app PRIVATE
    src/grinreflex_utils.cpp
)
endif()

if (CONFIG_BOARD_GRINREFLEX_DK1)
target_sources(app PRIVATE
    src/sample_usbd_init.c
    src/lvgl_fs_sample.c
    src/uart.cpp
    src/grinreflex.cpp
)
endif()

if (CONFIG_BOARD_GRINREFLEX_DK2)
target_sources(app PRIVATE
    src/grinreflex_v2.cpp
)
endif()
