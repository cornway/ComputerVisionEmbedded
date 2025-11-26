/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>

// #define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
// #include <zephyr/logging/log.h>
// LOG_MODULE_REGISTER(main);

// static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
	// if (!device_is_ready(video_dev)) {
	// 	LOG_ERR("%s device is not ready", video_dev->name);
	// }
	// LOG_DBG("%s", video_dev->name);
	return 0;
}
