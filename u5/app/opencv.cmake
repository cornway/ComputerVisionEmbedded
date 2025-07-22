

zephyr_compile_options(-DOPENCV_DISABLE_THREAD_SUPPORT)

set(CV_LIB_PATH ${CMAKE_CURRENT_LIST_DIR}/../lib/cv)
set(CV_MODULES_PATH ${CV_LIB_PATH}/opencv/modules)
set(CV_LIBA_PATH ${CV_LIB_PATH}/build/opencv/lib)
set(CV_SAMPLE_IMAGE "${CMAKE_CURRENT_LIST_DIR}/../../assets/sample.bin")

target_include_directories(app PRIVATE
    ${CV_LIB_PATH}/build
    ${CV_LIB_PATH}/opencv/include
    ${CV_MODULES_PATH}/core/include
    ${CV_MODULES_PATH}/imgproc/include
    ${CV_MODULES_PATH}/imgcodecs/include
)

target_compile_definitions(app PRIVATE CV_SAMPLE_IMAGE=${CV_SAMPLE_IMAGE})

target_link_libraries(app PRIVATE
    ${CV_LIBA_PATH}/libopencv_imgproc.a
    ${CV_LIBA_PATH}/libopencv_core.a
)

target_sources(app PRIVATE
    src/opencv_sample.cpp
)