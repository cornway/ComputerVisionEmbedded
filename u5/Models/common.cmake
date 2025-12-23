function(process_model model weights_base_addr model_path mpool_file_in user_json_in)
  set(STEDGEAI_TOOL stedgeai)

  if (NOT model)
    message(FATAL_ERROR "model is not defined")
  endif()

  if (NOT DEFINED weights_base_addr)
    message(FATAL_ERROR "weights_base_addr is not defined")
  endif()

  if (NOT model_path)
    message(FATAL_ERROR "model_path is not defined")
  endif()

  if (NOT mpool_file_in)
    message(FATAL_ERROR "mpool_file_in is not defined")
  endif()

  if (NOT user_json_in)
    message(FATAL_ERROR "user_json_in is not defined")
  endif()

  set(SOC_SERIES "${CONFIG_SOC_SERIES}")
  # TODO: consider something else for other SOC's ?
  string(REGEX REPLACE "x$" "" SOC_SERIES "${SOC_SERIES}")

  math(EXPR WEIGHTS_BASE_ADDR_HEX "${weights_base_addr}" OUTPUT_FORMAT HEXADECIMAL)

  set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${model})

  set(MPOOL_FILE_IN ${mpool_file_in})
  set(MPOOL_FILE_OUT ${OUTPUT_DIR}/${SOC_SERIES}.mpool)

  configure_file(${MPOOL_FILE_IN} ${MPOOL_FILE_OUT} @ONLY)
  configure_file(${user_json_in} ${OUTPUT_DIR}/user_neuralart.json @ONLY)

  set(ST_AI_WS_DIR ${OUTPUT_DIR}/st_ai_ws)
  set(ST_AI_OUT_DIR ${OUTPUT_DIR}/st_ai_output)

  add_custom_command(
    OUTPUT
      ${ST_AI_OUT_DIR}/${model}.stamp
    COMMAND ${STEDGEAI_TOOL}
      generate
      --workspace ${ST_AI_WS_DIR}
      --output ${ST_AI_OUT_DIR}
      --name ${model}
      --no-inputs-allocation
      --model ${model_path}
      --target ${SOC_SERIES}
      --st-neural-art default@${OUTPUT_DIR}/user_neuralart.json
    COMMAND ${CMAKE_OBJCOPY}
      -I binary ${ST_AI_OUT_DIR}/${model}_atonbuf.xSPI2.raw
      --change-addresses ${WEIGHTS_BASE_ADDR_HEX}
      -O ihex  ${CMAKE_BINARY_DIR}/${model}_data.hex
    COMMAND ${CMAKE_COMMAND} -E touch ${ST_AI_OUT_DIR}/${model}.stamp
    DEPENDS
      ${OUTPUT_DIR}/user_neuralart.json
      ${MPOOL_FILE_OUT}
      ${model_path}
    BYPRODUCTS
      ${ST_AI_OUT_DIR}/${model}.h
      ${ST_AI_OUT_DIR}/${model}.c
      ${ST_AI_OUT_DIR}/${model}_ecblobs.h
      ${CMAKE_BINARY_DIR}/${model}_data.hex
    VERBATIM
  )

  add_custom_target(${model} ALL
    DEPENDS
      ${ST_AI_OUT_DIR}/${model}.stamp
      ${ST_AI_OUT_DIR}/${model}.h
      ${ST_AI_OUT_DIR}/${model}.c
      ${ST_AI_OUT_DIR}/${model}_ecblobs.h
      ${CMAKE_BINARY_DIR}/${model}_data.hex
  )

  add_dependencies(app ${model})

  target_sources(app PRIVATE ${ST_AI_OUT_DIR}/${model}.c)
  target_include_directories(app PRIVATE ${ST_AI_OUT_DIR})

  file(SIZE ${model_path} MODEL_BIN_SIZE)

  set(ALIGN 0x10000)
  math(EXPR weights_base_addr "(${weights_base_addr} + ${MODEL_BIN_SIZE} + (${ALIGN}-1)) & ~(${ALIGN}-1)")
  set(weights_base_addr ${weights_base_addr} PARENT_SCOPE)
endfunction()
