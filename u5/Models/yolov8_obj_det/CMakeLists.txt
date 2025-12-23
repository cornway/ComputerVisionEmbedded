
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

set(SOC_SERIES "${CONFIG_SOC_SERIES}")
# TODO: consider something else for other SOC's ?
string(REGEX REPLACE "x$" "" SOC_SERIES "${SOC_SERIES}")

set(model_file ${CMAKE_CURRENT_LIST_DIR}/yolov8n_100e_int8.tflite)
set(mpool_file_in ${CMAKE_CURRENT_LIST_DIR}/../mpools/${SOC_SERIES}.mpool.in)
set(user_json_in ${CMAKE_CURRENT_LIST_DIR}/user_neuralart.json.in)

process_model(${model} ${WEIGHTS_BASE_ADDR} ${model_file} ${mpool_file_in} ${user_json_in})
set(WEIGHTS_BASE_ADDR ${weights_base_addr} PARENT_SCOPE)