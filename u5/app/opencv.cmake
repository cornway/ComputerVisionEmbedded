

zephyr_compile_options(-DOPENCV_DISABLE_THREAD_SUPPORT)

set(CV_LIB_PATH ${CMAKE_CURRENT_LIST_DIR}/../lib/cv)
set(CV_MODULES_PATH ${CV_LIB_PATH}/opencv/modules)
set(CV_LIBA_PATH ${CV_LIB_PATH}/build/opencv/lib)
set(CV_LIBA_3rdparty_PATH ${CV_LIB_PATH}/build/opencv/3rdparty/lib)

set(CV_SAMPLE_IMAGE "${CMAKE_CURRENT_LIST_DIR}/../../assets/sample.bin")

target_include_directories(app PRIVATE
    ${CV_LIB_PATH}/build
    ${CV_LIB_PATH}/opencv/include
    ${CV_MODULES_PATH}/core/include
    ${CV_MODULES_PATH}/imgproc/include
    ${CV_MODULES_PATH}/imgcodecs/include
    ${CV_MODULES_PATH}/objdetect/include
    ${CV_MODULES_PATH}/calib3d/include
    ${CV_MODULES_PATH}/features2d/include
    ${CV_MODULES_PATH}/flann/include
)

target_compile_definitions(app PRIVATE CV_SAMPLE_IMAGE=${CV_SAMPLE_IMAGE})

target_link_libraries(app PRIVATE
    ${CV_LIBA_PATH}/libopencv_imgcodecs.a
    ${CV_LIBA_PATH}/libopencv_imgproc.a
    ${CV_LIBA_PATH}/libopencv_calib3d.a
    ${CV_LIBA_PATH}/libopencv_features2d.a
    ${CV_LIBA_PATH}/libopencv_flann.a
    ${CV_LIBA_PATH}/libopencv_objdetect.a
    ${CV_LIBA_PATH}/libopencv_core.a
    ${CV_LIBA_3rdparty_PATH}/liblibpng.a
    ${CV_LIBA_3rdparty_PATH}/liblibopenjp2.a
    ${CV_LIBA_3rdparty_PATH}/libzlib.a
    ${CV_LIBA_3rdparty_PATH}/liblibjpeg-turbo.a
)

target_sources(app PRIVATE
    src/opencv_sample.cpp
    src/opencv_utils.cpp
)
