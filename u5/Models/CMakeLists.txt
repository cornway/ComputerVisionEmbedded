

dt_nodelabel(xspi2 NODELABEL "xspi2" REQUIRED)
dt_reg_addr(xspi2_mmap_addr PATH ${xspi2} INDEX 1 REQUIRED)
set(WEIGHTS_BASE_ADDR ${xspi2_mmap_addr})

dt_nodelabel(model_weights_partition NODELABEL "model_weights_partition" REQUIRED)
dt_reg_addr(model_weights_partition_addr PATH ${model_weights_partition})
math(EXPR WEIGHTS_BASE_ADDR "${WEIGHTS_BASE_ADDR} + ${model_weights_partition_addr}")
math(EXPR WEIGHTS_BASE_ADDR_HEX "${WEIGHTS_BASE_ADDR}" OUTPUT_FORMAT HEXADECIMAL)


set (MODEL_DIRS
    yolov8_obj_det
)


foreach(model ${MODEL_DIRS})
    string(TOUPPER "${model}" model_upper)
    if(DEFINED CONFIG_${model_upper} AND CONFIG_${model_upper})
        add_subdirectory(${model})

        message(STATUS "WEIGHTS_BASE_ADDR next = ${WEIGHTS_BASE_ADDR_HEX}")
    else()
        message(STATUS "CONFIG_${model_upper} is not enabled")
    endif()
endforeach()
