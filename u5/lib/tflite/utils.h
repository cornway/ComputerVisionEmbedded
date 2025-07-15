#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include <tensorflow/lite/micro/micro_interpreter.h>
#include "models_settings.h"

/* Exported macro ------------------------------------------------------------*/
#define PNET_SIZE				12
#define RNET_SIZE				24
#define ONET_SIZE				48

#define PNET_THRESHOLD  0.5
#define RNET_THRESHOLD  0.7
#define ONET_THRESHOLD  0.8

#define NMS_THRESHOLD   0.35

#define IMG_MIN(A, B) ((A) < (B) ? (A) : (B))
#define IMG_MAX(A, B) ((A) < (B) ? (B) : (A))

/* Exported typedef ----------------------------------------------------------*/
typedef enum {
    MIN_MODE = 0,
    IOU_MODE
} nms_mode_t;

typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;
} coordinates_t;

typedef struct {
    float score;
    coordinates_t window;
    coordinates_t offsets;
} candidate_window_t;

typedef struct {
    uint8_t len;
    candidate_window_t * candidate_window;
} candidate_windows_t;

typedef struct {
    uint8_t len;
    coordinates_t * bbox;
} bboxes_t;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void print_candidate_windows(candidate_windows_t * candidate_windows);
void get_pnet_boxes(bboxes_t ** bboxes, uint16_t width, uint16_t height, float scale);
void add_candidate_windows(candidate_windows_t * candidate_windows, float * probs, float * offsets, bboxes_t * bboxes, float threshold);
void nms(candidate_windows_t * candidate_windows, float threshold, nms_mode_t mode);
void print_bboxes(bboxes_t * bboxes);
void get_calibrated_boxes(bboxes_t ** bboxes, candidate_windows_t * candidate_windows);
void square_boxes(bboxes_t * bboxes);
void correct_boxes(bboxes_t * bboxes, uint16_t w, uint16_t h);
void draw_rectangle_rgb888(uint8_t * buf, bboxes_t * bboxes, int width);
void print_rgb888(uint8_t * img, int width, int height);
void crop_rgb888_img(uint8_t * src, uint8_t * dst, uint16_t width, coordinates_t * coordinates);
void image_zoom_in_twice(uint8_t * dimage, int dw, int dh, int dc, uint8_t * simage, int sw,int sc);
void image_resize_linear(uint8_t * dst_image, uint8_t * src_image, int dst_w, int dst_h, int dst_c, int src_w, int src_h);

/* MTCNN */
void run_pnet(candidate_windows_t * candidate_windows, tflite::MicroInterpreter * interpreter, uint8_t * img, uint16_t w, uint16_t h, float scale);
void run_rnet(candidate_windows_t * candidate_windows, tflite::MicroInterpreter * interpreter, uint8_t * img, uint16_t w, uint16_t h, bboxes_t * bboxes);
void run_onet(candidate_windows_t * candidate_windows, tflite::MicroInterpreter * interpreter, uint8_t * img, uint16_t w, uint16_t h, bboxes_t * bboxes);

#endif /* UTILS_H_ */