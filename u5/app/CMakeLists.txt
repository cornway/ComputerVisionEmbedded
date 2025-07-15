#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(BOARDS ${CMAKE_CURRENT_LIST_DIR}/../boards)
set(OVERLAY_FILE ${BOARDS}/${BOARD}.overlay)

if(EXISTS ${OVERLAY_FILE})
    list(APPEND EXTRA_DTC_OVERLAY_FILE "${OVERLAY_FILE}")
else()
    message(FATAL_ERROR "${OVERLAY_FILE} not found")
endif()

set(EXTRA_CONF_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl.conf;${CMAKE_CURRENT_SOURCE_DIR}/usbd.conf
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app)

# TODO: below two lines are unclear
set(NO_THREADSAFE_STATICS $<TARGET_PROPERTY:compiler-cpp,no_threadsafe_statics>)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:${NO_THREADSAFE_STATICS}>)

target_include_directories(app PRIVATE src)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
    src/main.c
    src/sample_usbd_init.c
    src/lvgl_fs_sample.c
)
