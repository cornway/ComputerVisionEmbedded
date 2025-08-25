# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_TFLITE_LIB tflite)
add_subdirectory_ifdef(CONFIG_OPENCV_LIB cv)
add_subdirectory_ifdef(CONFIG_QUIRC_LIB quirc)