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

message(EXTRA_DTC_OVERLAY_FILE=${EXTRA_DTC_OVERLAY_FILE})


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
