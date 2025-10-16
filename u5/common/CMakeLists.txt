

if (CONFIG_OPENCV_LIB)
add_subdirectory(opencv)
endif()

if (CONFIG_NEMA)
add_subdirectory(nema)
endif()

target_include_directories(app PUBLIC include)

target_sources(app PRIVATE
    src/lvgl_utils.cpp
    src/video.cpp
    src/utils.cpp
)

dt_has_chosen(USART2 PROPERTY "zephyr_usart2")

if (USART2)
target_sources(app PRIVATE
    src/uart.cpp
)
endif()
