# TODO: below two lines are unclear
set(NO_THREADSAFE_STATICS $<TARGET_PROPERTY:compiler-cpp,no_threadsafe_statics>)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:${NO_THREADSAFE_STATICS}>)

zephyr_compile_options(-DOPENCV_DISABLE_THREAD_SUPPORT)
zephyr_compile_options(-DOPENCV_FLANN_USE_STD_RAND)

target_sources(app PRIVATE
    src/opencv_utils.cpp
    src/opencv_pipeline.cpp
)

set(OPENCV_LBP_CASCADES ${OPENCV_INSTALL_DIR}/share/opencv4/lbpcascades)
set(OPENCV_HAAR_CASCADES ${OPENCV_INSTALL_DIR}/share/opencv4/haarcascades)

set(OPENCV_CASCADE_FACE_PATH ${OPENCV_LBP_CASCADES}/lbpcascade_frontalface_improved.xml)
set(OPENCV_CASCADE_SMILE_PATH ${OPENCV_HAAR_CASCADES}/haarcascade_smile.xml)

configure_file(
    src/cascades.h.in
    src/cascades.h
    @ONLY
)

target_include_directories(app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")

target_include_directories(app PRIVATE include)

target_link_libraries(app PUBLIC opencv_lib)