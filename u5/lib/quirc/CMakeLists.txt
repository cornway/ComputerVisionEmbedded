
zephyr_library()
zephyr_library_sources(
    quirc/lib/decode.c
    quirc/lib/identify.c
    quirc/lib/quirc.c
    quirc/lib/version_db.c

)

zephyr_include_directories(quirc/lib)