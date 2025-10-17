/*
 * Copyright (c) 2024 Charles Dias <charlesdias.cd@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/drivers/video.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

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

int main(void) {
  init();

  while (1) {
    loop();
  }
}
