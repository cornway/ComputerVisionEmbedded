
zephyr_library()

zephyr_library_sources(
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/vision_models_pp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/vision_models_pp_maxi_if32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/vision_models_pp_maxi_is8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/vision_models_pp_maxi_iu8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/od_pp_yolov2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/od_pp_yolov5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/od_pp_yolov8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Src/od_pp_st_yolox.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_osal_zephyr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_debug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_lib_sw_operators.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_rt_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_runtime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_aton_util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_sw_float.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ll_sw_integer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton/ecloader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/Devices/STM32N6XX/npu_cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/Devices/STM32N6XX/mcu_cache.c
)

zephyr_compile_definitions(
    LL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6
    LL_ATON_OSAL=LL_ATON_OSAL_ZEPHYR
    LL_ATON_RT_MODE=LL_ATON_RT_ASYNC
    LL_ATON_SW_FALLBACK
    LL_ATON_DBG_BUFFER_INFO_EXCLUDED=1
    APP_HAS_PARALLEL_NETWORKS=0
)

zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/ll_aton
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Npu/Devices/STM32N6XX
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/lib_vision_models_pp/lib_vision_models_pp/Inc/
)

function(add_prebuilt_library lib_name prebuilt_path)
    add_library(${lib_name} STATIC IMPORTED GLOBAL)
    set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${BLOBS_DIR}/${prebuilt_path}
    )
    zephyr_link_libraries(${lib_name})
endfunction()

add_prebuilt_library(libnetworkruntime1020cm55gcc ${CMAKE_CURRENT_SOURCE_DIR}/Lib/AI_Runtime/Lib/GCC/ARMCortexM55/NetworkRuntime1020_CM55_GCC.a)