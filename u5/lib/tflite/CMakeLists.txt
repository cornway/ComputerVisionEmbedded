

zephyr_library()
zephyr_library_sources(
    tflm.cc
    utils.cc
)

#target_include_directories(tflite PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_SOURCE_DIR}/models
#)