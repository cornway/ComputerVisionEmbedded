/**
  ******************************************************************************
  * @file           : mtcnn.c
  * @author         : Mauricio Barroso Benavides
  * @date           : Oct 2, 2022
  * @brief          : todo: write brief
  ******************************************************************************
  * @attention
  *
  * MIT License
  *
  * Copyright (c) 2022 Mauricio Barroso Benavides
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to
  * deal in the Software without restriction, including without limitation the
  * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  * sell copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  * IN THE SOFTWARE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "utils.h"

/* Private macro -------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static uint16_t get_width(coordinates_t * coordinates);
static uint16_t get_height(coordinates_t * coordinates);
static int compare_scores(const void * n1vp, const void * n2vp);
/* Exported functions --------------------------------------------------------*/
void print_candidate_windows(candidate_windows_t * candidate_windows) {
    for(uint8_t i = 0; i < candidate_windows->len; i++) {
      printf("%d: ", i);
        printf("[ ");
        printf("score:%f ", (double)candidate_windows->candidate_window[i].score);
        printf("] ");

        printf("[ ");
        printf("x1:%f ", (double)candidate_windows->candidate_window[i].window.x1);
        printf("y1:%f ", (double)candidate_windows->candidate_window[i].window.y1);
        printf("x2:%f ", (double)candidate_windows->candidate_window[i].window.x2);
        printf("y2:%f ", (double)candidate_windows->candidate_window[i].window.y2);
        printf("] ");

        printf("[ ");
        printf("x1:%f ", (double)candidate_windows->candidate_window[i].offsets.x1);
        printf("y1:%f ", (double)candidate_windows->candidate_window[i].offsets.y1);
        printf("x2:%f ", (double)candidate_windows->candidate_window[i].offsets.x2);
        printf("y2:%f ", (double)candidate_windows->candidate_window[i].offsets.y2);
        printf("]\n");
    }
}

void get_pnet_boxes(bboxes_t ** bboxes, uint16_t width, uint16_t height, float scale) {
    /* Allocate memory for the bboxes structure */
    * bboxes = (bboxes_t *)(malloc(sizeof(bboxes_t)));

    if(bboxes == NULL) {
        // return NULL;
    }

    (* bboxes)->len = 0;

    /* Calculate the number of boxes */
    uint8_t cols = (uint8_t)((((float)width * scale - 12.0f) / 2.0f) + 1.0f);
    uint8_t rows = (uint8_t)((((float)height * scale - 12.0f) / 2.0f) + 1.0f);

    /* Allocate memory for all bboxes */
    (* bboxes)->bbox = (coordinates_t *)(malloc(cols * rows * sizeof(coordinates_t)));

    if((* bboxes)->bbox == NULL) {
        // return NULL;
    }

    (* bboxes)->len = cols * rows;

//    printf("Scale: %f\n", scale);
//    printf("Len: %d\n", cols * rows);

    for(uint16_t i = 0; i < (* bboxes)->len; i++) {
			(* bboxes)->bbox[i].x1 = ceil((float)(2 * (i % cols) + 1) / scale);
			(* bboxes)->bbox[i].y1 = ceil((float)(2 * (i / cols) + 1) / scale);
			(* bboxes)->bbox[i].x2 = ceil((float)(2 * (i % cols) + 1 + 12) / scale);
			(* bboxes)->bbox[i].y2 = ceil((float)(2 * (i / cols) + 1 + 12) / scale);

			/* todo: test */
//			printf("x1:%f, y1:%f\n", (* bboxes)->bbox[i].x1, (* bboxes)->bbox[i].y1);
//			printf("x2:%f, y2:%f\r\n", (* bboxes)->bbox[i].x2, (* bboxes)->bbox[i].y2);
    }

//    printf("\r\n");
}

void add_candidate_windows(candidate_windows_t * candidate_windows, float * scores, float * offsets, bboxes_t * bboxes, float threshold) {
	for(uint8_t i = 0; i < bboxes->len; i++) {
		/* Check if the score value is greater than threshold value */
		if(scores[(i * 2) + 1] < threshold) {
			continue;
		}

		/* Allocate memory for the new candidate window */
		candidate_windows->candidate_window = (candidate_window_t *)realloc(candidate_windows->candidate_window, ++(candidate_windows->len) * sizeof(candidate_window_t));

		/* If the candidate_windows pointer is null continue the loop */
		if(candidate_windows->candidate_window == NULL) {
			continue;
		}

		/* Fill data members */
		candidate_windows->candidate_window[candidate_windows->len - 1].score = scores[(i * 2) + 1];
		candidate_windows->candidate_window[candidate_windows->len - 1].window.x1 = bboxes->bbox[i].x1;
		candidate_windows->candidate_window[candidate_windows->len - 1].window.y1 = bboxes->bbox[i].y1;
		candidate_windows->candidate_window[candidate_windows->len - 1].window.x2 = bboxes->bbox[i].x2;
		candidate_windows->candidate_window[candidate_windows->len - 1].window.y2 = bboxes->bbox[i].y2;

		candidate_windows->candidate_window[candidate_windows->len - 1].offsets.x1 = offsets[0 + (i * 4)];
		candidate_windows->candidate_window[candidate_windows->len - 1].offsets.y1 = offsets[1 + (i * 4)];
		candidate_windows->candidate_window[candidate_windows->len - 1].offsets.x2 = offsets[2 + (i * 4)];
		candidate_windows->candidate_window[candidate_windows->len - 1].offsets.y2 = offsets[3 + (i * 4)];
	}
}

void nms(candidate_windows_t * candidate_windows, float threshold, nms_mode_t mode) {
    /* Return NULL if there are no candidate windows */
    if(candidate_windows->len == 0) {
        // return NULL;	/* Return with error */
    }

    /* Sort descending by score value */
    qsort(candidate_windows->candidate_window, candidate_windows->len, sizeof(candidate_window_t), compare_scores);

    /* Calculate candidate_windows areas */
    uint16_t areas[candidate_windows->len];

    for(uint8_t i = 0; i < candidate_windows->len; i++) {
        areas[i] = get_width(&candidate_windows->candidate_window[i].window) * get_height(&candidate_windows->candidate_window[i].window);
    }

    for(uint8_t i = 0; i < candidate_windows->len; i++) {
        uint8_t counter = i + 1;

        for(uint8_t j = i + 1; j < candidate_windows->len; j++) {
            uint8_t ix1 = IMG_MAX(candidate_windows->candidate_window[i].window.x1 , candidate_windows->candidate_window[j].window.x1);
            uint8_t iy1 = IMG_MAX(candidate_windows->candidate_window[i].window.y1 , candidate_windows->candidate_window[j].window.y1);
            uint8_t ix2 = IMG_MIN(candidate_windows->candidate_window[i].window.x2 , candidate_windows->candidate_window[j].window.x2);
            uint8_t iy2 = IMG_MIN(candidate_windows->candidate_window[i].window.y2 , candidate_windows->candidate_window[j].window.y2);

            /* Calculate intersection area */
            uint16_t iarea = IMG_MAX(0.0, ix2 - ix1 + 1.0) * IMG_MAX(0.0, iy2 - iy1 + 1.0);

            /* Calculate ovelap between windows */
            float overlap;

            if(mode == MIN_MODE) {
                overlap = (float)iarea / (float)IMG_MIN(areas[i], areas[j]);
            }
            else {
                overlap = (float)iarea / (float)(areas[i] + areas[j] - iarea);
            }

            /* Update candidate windows if overlap is less than threshold*/
            if(overlap < threshold) {
                memcpy(&(candidate_windows->candidate_window)[counter++], &candidate_windows->candidate_window[j], sizeof(candidate_window_t));
            }
        }
        /* Update candidate_windows lenght */
        candidate_windows->candidate_window = (candidate_window_t *)realloc(candidate_windows->candidate_window, counter * sizeof(candidate_window_t));
        candidate_windows->len = counter;
    }
}

void print_bboxes(bboxes_t * bboxes) {
    for(uint8_t i = 0; i < bboxes->len; i++) {
        printf("%d: ", i);
        printf("[ ");
        printf("x1:%.0f ", (double)bboxes->bbox[i].x1);
        printf("y1:%.0f ", (double)bboxes->bbox[i].y1);
        printf("x2:%.0f ", (double)bboxes->bbox[i].x2);
        printf("y2:%.0f ", (double)bboxes->bbox[i].y2);
        printf("]\n");
    }
}

void get_calibrated_boxes(bboxes_t ** bboxes, candidate_windows_t * candidate_windows) {
    // * bboxes = get_bboxes(candidate_windows);

    /* Allocate memory for the bboxes structure */
    * bboxes = (bboxes_t *)malloc(sizeof(bboxes_t));

    if(bboxes == NULL) {
        // return NULL; /* Return with error */
    }

    /* Allocate memory for all bboxes */
    (* bboxes)->len = candidate_windows->len;
    (* bboxes)->bbox = (coordinates_t *)malloc((* bboxes)->len * sizeof(coordinates_t));

    if((* bboxes)->bbox == NULL) {
        // return NULL;    /* Return with errors */
    }

    /* Fill coordinates */
    for(uint8_t i = 0; i < (* bboxes)->len; i++) {
        (* bboxes)->bbox[i] = candidate_windows->candidate_window[i].window;
    }

    /* Fill offsets */
    for(uint8_t i = 0; i < (* bboxes)->len; i++) {
        uint16_t w = get_width(&(* bboxes)->bbox[i]);
        uint16_t h = get_height(&(* bboxes)->bbox[i]);

        if (candidate_windows->candidate_window[i].offsets.x1 > 1.0f) {
        	candidate_windows->candidate_window[i].offsets.x1 = 0;
        }
        if (candidate_windows->candidate_window[i].offsets.y1 > 1.0f) {
        	candidate_windows->candidate_window[i].offsets.y1 = 0;
        }
        if (candidate_windows->candidate_window[i].offsets.x2 > 1.0f) {
        	candidate_windows->candidate_window[i].offsets.x2 = 0;
        }
        if (candidate_windows->candidate_window[i].offsets.y2 > 1.0f) {
        	candidate_windows->candidate_window[i].offsets.y2 = 0;
        }

			(* bboxes)->bbox[i].x1 += w * candidate_windows->candidate_window[i].offsets.x1;
			(* bboxes)->bbox[i].y1 += h * candidate_windows->candidate_window[i].offsets.y1;
			(* bboxes)->bbox[i].x2 += w * candidate_windows->candidate_window[i].offsets.x2;
			(* bboxes)->bbox[i].y2 += h * candidate_windows->candidate_window[i].offsets.y2;


    }

}

void square_boxes(bboxes_t * bboxes) {
    for(uint8_t i = 0; i < bboxes->len; i++) {
        uint16_t w = get_width(&bboxes->bbox[i]);
        uint16_t h = get_height(&bboxes->bbox[i]);
        uint16_t max_side = IMG_MAX(h, w);

        bboxes->bbox[i].x1 += (w * 0.5f) - (max_side * 0.5f);
        bboxes->bbox[i].y1 += (h * 0.5f) - (max_side * 0.5f);
        bboxes->bbox[i].x2 = bboxes->bbox[i].x1 + max_side - 1.0f;
        bboxes->bbox[i].y2 = bboxes->bbox[i].y1 + max_side - 1.0f;

        /* todo: erase? */
				if (bboxes->bbox[i].x1 > IMG_W) {
					printf("Square\n");
					printf("error x1:%f\n", (double)bboxes->bbox[i].x1);
				}
				if (bboxes->bbox[i].y1 > IMG_H) {
					printf("Square\n");
					printf("error y1:%f\n", (double)bboxes->bbox[i].y1);
				}
    }
}

void correct_boxes(bboxes_t * bboxes, uint16_t w, uint16_t h) {
    for(uint8_t i = 0; i < bboxes->len; i++) {
        /* If box's bottom right corner is too far right */
        if(bboxes->bbox[i].x2 > w - 1) {
            bboxes->bbox[i].x2 = w - 1;
        }

        /* If box's bottom right corner is too low */
        if(bboxes->bbox[i].y2 > h - 1) {
            bboxes->bbox[i].y2 = h - 1;
        }

        /* If box's top left corner is too far left */
        if(bboxes->bbox[i].x1 < 0) {
            bboxes->bbox[i].x1 = 0;
        }

        /* If box's top left corner is too high */
        if(bboxes->bbox[i].y1 < 0) {
            bboxes->bbox[i].y1 = 0;
        }

        /* todo: erase? */
				if ((uint16_t)bboxes->bbox[i].x1 > IMG_W) {
					printf("Correct\n");
					printf("error x1:%f\n", (double)bboxes->bbox[i].x1);
				}
				if ((uint16_t)bboxes->bbox[i].y1 > IMG_H) {
					printf("Correct\n");
					printf("error y1:%f\n", (double)bboxes->bbox[i].y1);
				}
    }
}

void draw_rectangle_rgb888(uint8_t * buf, bboxes_t * bboxes, int width) {
	uint8_t p[4];
	for(int i = 0; i < bboxes->len; i++) {
		/* rectangle box */
		p[0] = (uint8_t)bboxes->bbox[i].x1;
		p[1] = (uint8_t)bboxes->bbox[i].y1;
		p[2] = (uint8_t)bboxes->bbox[i].x2;
		p[3] = (uint8_t)bboxes->bbox[i].y2;

		if ((p[2] < p[0]) || (p[3] < p[1])) {
			return;
		}

		#define DRAW_PIXEL_GREEN(BUF, X) \
		do {									\
			BUF[X + 0] = 0;			\
			BUF[X + 1] = 0xFF;	\
			BUF[X + 2] = 0;			\
		} while (0)

		// rectangle box
		for (int w = p[0]; w < p[2] + 1; w++) {
			int x1 = (p[1] * width + w) * 3;
			int x2 = (p[3] * width + w) * 3;
			DRAW_PIXEL_GREEN(buf, x1);
			DRAW_PIXEL_GREEN(buf, x2);
		}

		for (int h = p[1]; h < p[3] + 1; h++) {
			int y1 = (h * width + p[0]) * 3;
			int y2 = (h * width + p[2]) * 3;
			DRAW_PIXEL_GREEN(buf, y1);
			DRAW_PIXEL_GREEN(buf, y2);
		}
	}
}


void print_rgb888(uint8_t * img, int width, int height) {
	uint8_t * p = (uint8_t *)img;
	const char temp2char[17] = "@MNHQ&#UJ*x7^i;.";

	for(int j = 0; j < height; j++) {
		for(int i = 0; i < width; i++) {
			uint8_t * c = p + 3 * (j * width + i);
			uint8_t r = * c++;
			uint8_t g = * c++;
			uint8_t b = * c;
			uint32_t v = (r + g + b) / 3;
			v >>= 4;
			printf("%c", temp2char[15 - v]);
		}
		printf("\n");
	}
}

void crop_rgb888_img(uint8_t * src, uint8_t * dst, uint16_t width, coordinates_t * coordinates) {
	uint16_t x1 = (uint16_t)coordinates->x1;
	uint16_t y1 = (uint16_t)coordinates->y1;
	uint16_t y2 = (uint16_t)coordinates->y2;

  for(size_t i = y1; i < y2; i++) {
  	memcpy(dst + 3 * ((i - y1) * get_width(coordinates)), src + 3 * (i * width) + (x1 * 3), (get_width(coordinates) * 3) * sizeof(uint8_t));
	}
}

void image_zoom_in_twice(uint8_t *dimage, int dw, int dh, int dc, uint8_t *simage, int sw,int sc) {
	for(int dyi = 0; dyi < dh; dyi++) {
		int _di = dyi * dw;

		int _si0 = dyi * 2 * sw;
		int _si1 = _si0 + sw;

		for(int dxi = 0; dxi < dw; dxi++) {
			int di = (_di + dxi) * dc;
			int si0 = (_si0 + dxi * 2) * sc;
			int si1 = (_si1 + dxi * 2) * sc;

			if (1 == dc) {
				dimage[di] = (uint8_t)((simage[si0] + simage[si0 + 1] + simage[si1] + simage[si1 + 1]) >> 2);
			}
			else if(3 == dc) {
				dimage[di] = (uint8_t)((simage[si0] + simage[si0 + 3] + simage[si1] + simage[si1 + 3]) >> 2);
				dimage[di + 1] = (uint8_t)((simage[si0 + 1] + simage[si0 + 4] + simage[si1 + 1] + simage[si1 + 4]) >> 2);
				dimage[di + 2] = (uint8_t)((simage[si0 + 2] + simage[si0 + 5] + simage[si1 + 2] + simage[si1 + 5]) >> 2);
			}
			else {
				for(int dci = 0; dci < dc; dci++) {
					dimage[di + dci] = (uint8_t)((simage[si0 + dci] + simage[si0 + 3 + dci] + simage[si1 + dci] + simage[si1 + 3 + dci] + 2) >> 2);
				}
			}
		}
	}

	return;
}

void image_resize_linear(uint8_t * dst_image, uint8_t * src_image, int dst_w, int dst_h, int dst_c, int src_w, int src_h) {
    float scale_x = (float)src_w / dst_w;
    float scale_y = (float)src_h / dst_h;

    int dst_stride = dst_c * dst_w;
    int src_stride = dst_c * src_w;

    if(fabs(scale_x - 2) <= 1e-6 && fabs(scale_y - 2) <= 1e-6) {
        image_zoom_in_twice(
            dst_image,
            dst_w,
            dst_h,
            dst_c,
            src_image,
            src_w,
            dst_c);
    }
    else {
    	for(int y = 0; y < dst_h; y++) {
    		float fy[2];
            fy[0] = (float)((y + 0.5f) * scale_y - 0.5f);
            int src_y = (int)fy[0];
            fy[0] -= src_y;
            fy[1] = 1 - fy[0];
            src_y = IMG_MAX(0, src_y);
            src_y = IMG_MIN(src_y, src_h - 2);

            for(int x = 0; x < dst_w; x++) {
                float fx[2];
                fx[0] = (float)((x + 0.5f) * scale_x - 0.5f);
                int src_x = (int)fx[0];
                fx[0] -= src_x;

                if(src_x < 0) {
                    fx[0] = 0;
                    src_x = 0;
                }

                if(src_x > src_w - 2) {
                    fx[0] = 0;
                    src_x = src_w - 2;
                }

                fx[1] = 1 - fx[0];

                for(int c = 0; c < dst_c; c++) {
                    dst_image[y * dst_stride + x * dst_c + c] = round(src_image[src_y * src_stride + src_x * dst_c + c] * fx[1] * fy[1] + src_image[src_y * src_stride + (src_x + 1) * dst_c + c] * fx[0] * fy[1] + src_image[(src_y + 1) * src_stride + src_x * dst_c + c] * fx[1] * fy[0] + src_image[(src_y + 1) * src_stride + (src_x + 1) * dst_c + c] * fx[0] * fy[0]);
                }
            }
    	}
    }
}

void run_pnet(candidate_windows_t * candidate_windows, tflite::MicroInterpreter * interpreter, uint8_t * img, uint16_t w, uint16_t h, float scale) {
	uint16_t ws = uint16_t(w * scale);
	uint16_t hs = uint16_t(h * scale);

	uint8_t * imgs = (uint8_t *)malloc((ws * hs * 3) * sizeof(uint8_t));
	image_resize_linear(imgs, img, ws, hs, 3, w, h);


	/* Feed model */
	TfLiteTensor * input = interpreter->input(0);

	for(int i = 0; i < ws * hs * 3; i++) {
		input->data.int8[i] = ((uint8_t *) imgs)[i] ^ 0x80;
	}

	/* Free scaled image memory */
	free(imgs);


	/* Run the model on this input and make sure it succeeds */
	if (kTfLiteOk != interpreter->Invoke()) {
		/* Return with error */
		printf("Error\n");
	}

	TfLiteTensor * probs = interpreter->output(0);
	TfLiteTensor * offsets = interpreter->output(1);

	bboxes_t * bboxes = NULL;
	get_pnet_boxes(&bboxes, w, h, scale);

	add_candidate_windows(candidate_windows, probs->data.f, offsets->data.f, bboxes, PNET_THRESHOLD);
	free(bboxes->bbox);
	free(bboxes);

//	print_candidate_windows(candidate_windows);
}

void run_rnet(candidate_windows_t * candidate_windows, tflite::MicroInterpreter * interpreter, uint8_t * img, uint16_t w, uint16_t h, bboxes_t * bboxes) {
  float probs_buf[bboxes->len * 2];
  float offsets_buf[bboxes->len * 4];

	for(uint8_t i = 0; i < bboxes->len; i++) {
		/* Width and height for the crop image */
		uint16_t wc = get_width(&bboxes->bbox[i]);
		uint16_t hc = get_height(&bboxes->bbox[i]);



		uint8_t * imgc = (uint8_t *)malloc(wc * hc * 3 * sizeof(uint8_t));
		crop_rgb888_img(img, imgc, w, &bboxes->bbox[i]);

		/* Scale image for R-Net */
		uint8_t * rnet_image = (uint8_t *)malloc((RNET_SIZE * RNET_SIZE * 3) * sizeof(uint8_t));
		image_resize_linear(rnet_image, imgc, RNET_SIZE, RNET_SIZE, 3, wc, hc);
		free(imgc);

		/* Feed model */
		TfLiteTensor * input = interpreter->input(0);

		for(int i = 0; i < RNET_SIZE * RNET_SIZE * 3; i++) {
			input->data.int8[i] = ((uint8_t *) rnet_image)[i] ^ 0x80;
		}

		free(rnet_image);

		/* Run the model on this input and make sure it succeeds */
		if (kTfLiteOk != interpreter->Invoke()) {
			/* todo: return with error */
		}

		TfLiteTensor * probs = interpreter->output(0);
		TfLiteTensor * offsets = interpreter->output(1);

		for(uint8_t j = 0; j < 2; j++) {
			probs_buf[j + (i * 2)] = probs->data.f[j];
		}

		for(uint8_t j = 0; j < 4; j++) {
			offsets_buf[j + (i * 2)] = offsets->data.f[j];
		}

	}

	add_candidate_windows(candidate_windows, probs_buf, offsets_buf, bboxes, RNET_THRESHOLD);
}

void run_onet(candidate_windows_t * candidate_windows, tflite::MicroInterpreter * interpreter, uint8_t * img, uint16_t w, uint16_t h, bboxes_t * bboxes) {
  float probs_buf[bboxes->len * 2];
  float offsets_buf[bboxes->len * 4];

	for (uint8_t i = 0; i < bboxes->len; i++) {
		/* Width and height for the crop image */
		uint16_t wc = get_width(&bboxes->bbox[i]);
		uint16_t hc = get_height(&bboxes->bbox[i]);
//		printf("x1:%f, x2:%f, y1:%f, y2:%f\r\n", bboxes->bbox[i].x1, bboxes->bbox[i].x2, bboxes->bbox[i].y1, bboxes->bbox[i].y2);
//
//		printf("wc=%d, hc=%d\r\n", wc, hc);

		uint8_t * imgc = (uint8_t *)malloc(wc * hc * 3 * sizeof(uint8_t));
		crop_rgb888_img(img, imgc, w, &bboxes->bbox[i]);

		/* Scale image for R-Net */
		uint8_t * onet_image = (uint8_t *)malloc((ONET_SIZE * ONET_SIZE * 3) * sizeof(uint8_t));
		image_resize_linear(onet_image, imgc, ONET_SIZE, ONET_SIZE, 3, wc, hc);
		free(imgc);

		/* Feed model */
		TfLiteTensor * input = interpreter->input(0);

		for(int i = 0; i < ONET_SIZE * ONET_SIZE * 3; i++) {
			input->data.int8[i] = ((uint8_t *) onet_image)[i] ^ 0x80;
		}

		free(onet_image);

		/* Run the model on this input and make sure it succeeds */
		if (kTfLiteOk != interpreter->Invoke()) {
			/* todo: return with error */
		}

		TfLiteTensor * probs = interpreter->output(0);
		TfLiteTensor * offsets = interpreter->output(1);

		for(uint8_t j = 0; j < 2; j++) {
			probs_buf[j + (i * 2)] = probs->data.f[j];
		}

		for(uint8_t j = 0; j < 4; j++) {
			offsets_buf[j + (i * 2)] = offsets->data.f[j];
		}
	}

	add_candidate_windows(candidate_windows, probs_buf, offsets_buf, bboxes, ONET_THRESHOLD);
}

/* Private functions ---------------------------------------------------------*/
static uint16_t get_width(coordinates_t * coordinates) {
    return coordinates->x2 - coordinates->x1 + 1.0f;
}

static uint16_t get_height(coordinates_t * coordinates) {
    return coordinates->y2 - coordinates->y1 + 1.0f;
}

static int compare_scores(const void * n1vp, const void * n2vp) {
	const candidate_window_t * n1ptr = (const candidate_window_t *)n1vp;
	const candidate_window_t * n2ptr = (const candidate_window_t *)n2vp;

	if(n1ptr->score <= n2ptr->score) {
		return 1;
	}
	else if(n1ptr->score > n2ptr->score) {
		return -1;
	}

	return 0;
}
/***************************** END OF FILE ************************************/
