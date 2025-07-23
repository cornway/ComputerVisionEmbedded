

if (NOT CONFIG_FPU)
    message(FATAL_ERROR "FPU must be enabled: CONFIG_FPU=y")
endif()

set(LVGL_LIB_PATH ${CMAKE_CURRENT_LIST_DIR}/../../modules/lib/gui/lvgl)
set(LVGL_NEMA_PATH ${LVGL_LIB_PATH}/libs/nema_gfx)
set(LVGL_NEMA_LIBA_PATH ${LVGL_NEMA_PATH}/lib/core/cortex_m33_revC/gcc)


zephyr_include_directories(
    ${LVGL_NEMA_PATH}/include
    ${LVGL_LIB_PATH}/src/draw/nema_gfx
)

if (CONFIG_FP_SOFTABI)
target_link_libraries(app PRIVATE
    ${LVGL_NEMA_LIBA_PATH}/libnemagfx.a
    ${LVGL_NEMA_LIBA_PATH}/libnemagfx-wc16.a
)
else()
target_link_libraries(app PRIVATE
    ${LVGL_NEMA_LIBA_PATH}/libnemagfx-float-abi-hard.a
    ${LVGL_NEMA_LIBA_PATH}/libnemagfx-float-abi-hard-wc16.a
)
endif() # CONFIG_FP_SOFTABI

file(GLOB_RECURSE SOURCES ${LVGL_LIB_PATH}/src/draw/nema_gfx/*.c)
zephyr_library_sources(${SOURCES})
