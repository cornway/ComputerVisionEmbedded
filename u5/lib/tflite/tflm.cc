


#include <zephyr/kernel.h>

#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include "utils.h"

#include "models/pnet_1.c"
#include "models/pnet_2.c"
#include "models/pnet_3.c"
#include "models/rnet.c"
#include "models/onet.c"


#define SCRATH_BUFFER_SIZE (39 * 1024)
#define TENSOR_ARENA_SIZE (110 * 1024 + SCRATH_BUFFER_SIZE)

static tflite::MicroInterpreter * pnet_1_interpreter = nullptr;
static tflite::MicroInterpreter * pnet_2_interpreter = nullptr;
static tflite::MicroInterpreter * pnet_3_interpreter = nullptr;
static tflite::MicroInterpreter * rnet_interpreter = nullptr;
static tflite::MicroInterpreter * onet_interpreter = nullptr;

void tflm_init(void) {
//	tflite::ErrorReporter* error_reporter = nullptr;
//  tflite::MicroErrorReporter micro_error_reporter;
//  error_reporter = &micro_error_reporter;

  /* Map the model into a usable data structure */
  const tflite::Model * pnet_1_model = tflite::GetModel(pnet_1_model_data);
  if (pnet_1_model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
    		"version %d.", pnet_1_model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  const tflite::Model * pnet_2_model = tflite::GetModel(pnet_2_model_data);
  if (pnet_2_model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
    		"version %d.", pnet_2_model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  const tflite::Model * pnet_3_model = tflite::GetModel(pnet_3_model_data);
  if (pnet_3_model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
    		"version %d.", pnet_3_model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  const tflite::Model * rnet_model = tflite::GetModel(rnet_model_data);
  if (rnet_model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
    		"version %d.", rnet_model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  const tflite::Model * onet_model = tflite::GetModel(onet_model_data);
  if (rnet_model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
    		"version %d.", onet_model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  /* Reserve memory */
  uint8_t * tensor_arena = (uint8_t *) malloc(TENSOR_ARENA_SIZE);

  if (tensor_arena == NULL) {
    printf("Couldn't allocate memory of %d bytes\n", TENSOR_ARENA_SIZE);
    return;
  }

  /* Pull in only the operation implementations we need. This relies on a
   * complete list of all the ops needed by this graph. An easier approach is to
   * just use the AllOpsResolver, but this will incur some penalty in code space
   * for op implementations that are not needed by this graph
   *
   */
  static tflite::MicroMutableOpResolver<10> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddPrelu();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddTranspose();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddDequantize();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();

  /* Build an interpreter to run the model with */
  static tflite::MicroInterpreter static_pnet_1_interpreter(
  		pnet_1_model, micro_op_resolver, tensor_arena, TENSOR_ARENA_SIZE);
  pnet_1_interpreter = &static_pnet_1_interpreter;

  static tflite::MicroInterpreter static_pnet_2_interpreter(
  		pnet_2_model, micro_op_resolver, tensor_arena, TENSOR_ARENA_SIZE);
  pnet_2_interpreter = &static_pnet_2_interpreter;

  static tflite::MicroInterpreter static_pnet_3_interpreter(
    		pnet_3_model, micro_op_resolver, tensor_arena, TENSOR_ARENA_SIZE);
    pnet_3_interpreter = &static_pnet_3_interpreter;

  static tflite::MicroInterpreter static_rnet_interpreter(
  		rnet_model, micro_op_resolver, tensor_arena, TENSOR_ARENA_SIZE);
  rnet_interpreter = &static_rnet_interpreter;

  static tflite::MicroInterpreter static_onet_interpreter(
  		onet_model, micro_op_resolver, tensor_arena, TENSOR_ARENA_SIZE);
  onet_interpreter = &static_onet_interpreter;

  /* Allocate memory from the tensor_arena for the model's tensors */
  TfLiteStatus allocate_status = pnet_1_interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  allocate_status = pnet_2_interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  allocate_status = pnet_3_interpreter->AllocateTensors();
	if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
		return;
	}

  allocate_status = rnet_interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
  	MicroPrintf("AllocateTensors() failed");
    return;
  }

  allocate_status = onet_interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
  	MicroPrintf("AllocateTensors() failed");
    return;
  }
}

void inference_task(void *frame) {
	/* Convert to RGB888 and return frame buffer*/
    uint8_t * rgb888_image = (uint8_t *)frame;

    /* Run P-Net for all scales */
    candidate_windows_t pnet_candidate_windows;
    pnet_candidate_windows.candidate_window = NULL;
    pnet_candidate_windows.len = 0;

    run_pnet(&pnet_candidate_windows, pnet_1_interpreter, rgb888_image, IMG_W, IMG_H, PNET_1_SCALE);
    run_pnet(&pnet_candidate_windows, pnet_2_interpreter, rgb888_image, IMG_W, IMG_H, PNET_2_SCALE);
    run_pnet(&pnet_candidate_windows, pnet_3_interpreter, rgb888_image, IMG_W, IMG_H, PNET_3_SCALE);
    nms(&pnet_candidate_windows, NMS_THRESHOLD, IOU_MODE);

    bboxes_t * pnet_bboxes;
    get_calibrated_boxes(&pnet_bboxes, &pnet_candidate_windows);
    free(pnet_candidate_windows.candidate_window);

    square_boxes(pnet_bboxes);
    correct_boxes(pnet_bboxes, IMG_W, IMG_H);

    candidate_windows_t rnet_candidate_windows;
    rnet_candidate_windows.candidate_window = NULL;
    rnet_candidate_windows.len = 0;

    run_rnet(&rnet_candidate_windows, rnet_interpreter, rgb888_image, IMG_W, IMG_H, pnet_bboxes);
    nms(&rnet_candidate_windows, NMS_THRESHOLD, IOU_MODE);

    bboxes_t * rnet_bboxes;
    get_calibrated_boxes(&rnet_bboxes, &rnet_candidate_windows);
    free(rnet_candidate_windows.candidate_window);

    square_boxes(rnet_bboxes);
    correct_boxes(rnet_bboxes, IMG_W, IMG_H);

    candidate_windows_t onet_candidate_windows;
    onet_candidate_windows.candidate_window = NULL;
    onet_candidate_windows.len = 0;

    run_onet(&onet_candidate_windows, onet_interpreter, rgb888_image, IMG_W, IMG_H, rnet_bboxes);
    nms(&onet_candidate_windows, NMS_THRESHOLD, IOU_MODE);

    bboxes_t * onet_bboxes;
    get_calibrated_boxes(&onet_bboxes, &onet_candidate_windows);
    free(onet_candidate_windows.candidate_window);

    square_boxes(onet_bboxes);
    correct_boxes(onet_bboxes, IMG_W, IMG_H);

    if (onet_bboxes->len > 0) {
        /* Print MTCNN times and bboxes */
        printf("P-Net bboxes : %d\n", pnet_bboxes->len);
        printf("R-Net bboxes : %d\n", rnet_bboxes->len);
        printf("O-Net bboxes : %d\n", onet_bboxes->len);
        printf("MTCNN bboxes : %d\r\n", onet_bboxes->len);

        /* Open file */
        //FILE * f = fopen("/spiffs/faces.jpg", "wb");
        //size_t cnv_buf_len;
        //uint8_t * cnv_buf = NULL;

        /* Draw bboxes and save the file */
        //draw_rectangle_rgb888(rgb888_image, onet_bboxes, IMG_W);
        //fmt2jpg(rgb888_image, IMG_W * IMG_H, IMG_W, IMG_H, PIXFORMAT_RGB888, 80, &cnv_buf, &cnv_buf_len);
        //fwrite(cnv_buf, cnv_buf_len, 1, f);

        /* Close and free */
        //fclose(f);
        //free(cnv_buf);
    }

    /* Free all the bboxes */
    free(pnet_bboxes->bbox);
    free(pnet_bboxes);
    free(rnet_bboxes->bbox);
    free(rnet_bboxes);
    free(onet_bboxes->bbox);
    free(onet_bboxes);
}