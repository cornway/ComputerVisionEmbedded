/*
 * Copyright (c) 2024 Charles Dias <charlesdias.cd@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

#define CONFIG_VIDEO_WIDTH 160
#define CONFIG_VIDEO_HEIGHT 120

#include "grinreflex.h"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

#if !DT_HAS_CHOSEN(zephyr_camera)
#error No camera chosen in devicetree. Missing "--shield" or "--snippet video-sw-generator" flag?
#endif

#if !DT_HAS_CHOSEN(zephyr_display)
#error No display chosen in devicetree. Missing "--shield" flag?
#endif

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int main(void)
{
	struct video_buffer *vbuf = &(struct video_buffer){};
	enum video_buf_type type = VIDEO_BUF_TYPE_OUTPUT;
	int err;

	init();

	while (1)
	{
		loop(1);
	}

	lv_img_dsc_t video_img = {
		.header.w = CONFIG_VIDEO_WIDTH,
		.header.h = CONFIG_VIDEO_HEIGHT,
		.data_size = CONFIG_VIDEO_WIDTH * CONFIG_VIDEO_HEIGHT * sizeof(lv_color_t),
		.header.cf = LV_COLOR_FORMAT_NATIVE,
//		.data = (const uint8_t *)buffers[0]->buffer,
	};

	lv_obj_t *screen = lv_img_create(lv_scr_act());

	LOG_INF("- Capture started");

	/* Grab video frames */
	vbuf->type = type;
	while (1) {
		err = video_dequeue(video_dev, &vbuf, K_FOREVER);
		if (err) {
			LOG_ERR("Unable to dequeue video buf");
			return 0;
		}

		video_img.data = (uint8_t *)vbuf->buffer;
		lv_img_set_src(screen, &video_img);
		lv_obj_align(screen, LV_ALIGN_BOTTOM_LEFT, 0, 0);

		lv_task_handler();

		err = video_enqueue(video_dev, vbuf);
		if (err) {
			LOG_ERR("Unable to requeue video buf");
			return 0;
		}
	}
}
