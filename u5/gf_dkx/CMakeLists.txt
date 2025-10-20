#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(EXTRA_CONF_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl.conf;
)

if (CONFIG_VARIANT_RELEASE)
set(DTC_OVERLAY_FILE ${DTC_OVERLAY_FILE};${CMAKE_CURRENT_SOURCE_DIR}/overlays/release.overlay CACHE STRING "" FORCE)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(gf_dkx)

target_include_directories(app PRIVATE
    src
)

target_sources(app PRIVATE
    src/main.c
)

include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)

if (CONFIG_OPENCV_LIB)
target_sources(app PRIVATE
    src/opencv_fd_app.cpp
)
else()
target_sources(app PRIVATE
    src/lvgl_video_app.cpp
)
endif()

execute_process(
    COMMAND git describe --always --dirty
    OUTPUT_VARIABLE GIT_REVISION
)

string(REGEX REPLACE "\n$" "" GIT_REVISION "${GIT_REVISION}")

configure_file(
    src/version.h.in
    src/version.h
    @ONLY
)

target_include_directories(app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")
