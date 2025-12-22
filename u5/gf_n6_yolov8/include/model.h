/**
 ******************************************************************************
 * @file    model.h
 * @author  GPM Application Team
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */


#ifndef __MODEL__
#define __MODEL__

struct dbox {
	/* upper left coord */
	int x;
	int y;
	int w;
	int h;
};

void model_thread_ep(void *arg1, void *arg2, void *arg3);
int model_get_boxes(struct dbox *boxes, int boxes_nb);
int model_get_latest_inference_time(void);

#endif
