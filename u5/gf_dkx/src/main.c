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

#define CONFIG_VIDEO_WIDTH 160
#define CONFIG_VIDEO_HEIGHT 120

#include "grinreflex.h"
#include "version.h"

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

#if !DT_HAS_CHOSEN(zephyr_camera)
#error No camera chosen in devicetree. Missing "--shield" or "--snippet video-sw-generator" flag?
#endif

#if !DT_HAS_CHOSEN(zephyr_display)
#error No display chosen in devicetree. Missing "--shield" flag?
#endif

int main(void) {
  LOG_INF("Git revision : %s", GIT_REVISION);
  init();

  while (1) {
    loop();
  }
}
