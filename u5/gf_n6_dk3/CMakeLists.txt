# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gf_n6_dk3)

target_include_directories(app PRIVATE
src
)

target_sources(app PRIVATE
src/application.cpp
src/main.c)
