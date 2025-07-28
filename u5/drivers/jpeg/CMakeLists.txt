
zephyr_library()
zephyr_library_sources_ifdef(CONFIG_STM32_JPEG
    jpeg.c
    jpeg_utils.c
)
zephyr_library_include_directories(./)