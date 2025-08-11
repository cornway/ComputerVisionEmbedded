

zephyr_compile_options(-DOPENCV_DISABLE_THREAD_SUPPORT)
zephyr_compile_options(-DOPENCV_FLANN_USE_STD_RAND)

target_sources(app PRIVATE
    src/opencv_utils.cpp
)

target_link_libraries(app PUBLIC opencv_lib)
