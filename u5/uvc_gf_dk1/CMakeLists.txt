cmake_minimum_required(VERSION 3.13.1)

set(EXTRA_CONF_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl.conf;
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(uvc_gf_dk1)
include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)

target_include_directories(app PRIVATE
    src
)

target_sources(app PRIVATE
    src/main.c
)

if (NOT CONFIG_BOARD_GRINREFLEX_DK1)
message(FATAL "only CONFIG_BOARD_GRINREFLEX_DK1 is supported")
endif()