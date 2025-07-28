/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lvgl.h>
#include <lvgl_input_device.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include <stdio.h>
#include <zephyr/fs/fs.h>

#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

#include <lvgl_fs.h>
#include <sample_usbd.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#if defined(CONFIG_TFLITE_LIB)
#include <app/lib/tflm.h>
#endif

#if defined(CONFIG_OPENCV_LIB)
#include <opencv_sample.h>
#endif

#include "application.h"

#define AUTOMOUNT_NODE DT_NODELABEL(ffs2)
FS_FSTAB_DECLARE_ENTRY(AUTOMOUNT_NODE);

static uint32_t count;
static struct usbd_context *sample_usbd;

USBD_DEFINE_MSC_LUN(flash, "NAND", "Zephyr", "FlashDisk", "0.00");

static int enable_usb_device_next(void) {
  int err;

  sample_usbd = sample_usbd_init_device(NULL);
  if (sample_usbd == NULL) {
    LOG_ERR("Failed to initialize USB device");
    return -ENODEV;
  }

  err = usbd_enable(sample_usbd);
  if (err) {
    LOG_ERR("Failed to enable device support");
    return err;
  }

  LOG_DBG("USB device support enabled");

  return 0;
}

static void lv_btn_click_callback(lv_event_t *e) {
  ARG_UNUSED(e);

  count = 0;
}

int main(void) {
  char count_str[64] = {0};
  const struct device *display_dev;
  lv_obj_t *count_label;

  // fs_example();

  int ret = 0;
  ret = enable_usb_device_next();

  if (ret != 0) {
    LOG_ERR("Failed to enable USB");
  }
  display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display_dev)) {
    LOG_ERR("Device not ready, aborting test");
    return 0;
  }

  lvgl_fs_sample();

  lv_obj_t *hello_world_label;
  if (IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
    lv_obj_t *hello_world_button;

    hello_world_button = lv_button_create(lv_screen_active());
    lv_obj_align(hello_world_button, LV_ALIGN_TOP_MID, 0, -15);
    lv_obj_add_event_cb(hello_world_button, lv_btn_click_callback,
                        LV_EVENT_CLICKED, NULL);
    hello_world_label = lv_label_create(hello_world_button);
  } else {
    hello_world_label = lv_label_create(lv_screen_active());
  }

  lv_label_set_text(hello_world_label, "Hello world!");
  lv_obj_align(hello_world_label, LV_ALIGN_TOP_MID, 0, 0);

  count_label = lv_label_create(lv_screen_active());
  lv_obj_align(count_label, LV_ALIGN_BOTTOM_MID, 0, 0);

  lv_timer_handler();
  display_blanking_off(display_dev);

  if (init()) {
    return -1;
  }

  uint32_t fps_stop = 0;
  uint32_t frames = 0;

  fps_stop = k_uptime_get_32() + 1000;

  while (1) {
    // sprintf(count_str, "%d", count);
    // lv_label_set_text(count_label, count_str);
    lv_timer_handler();

#if 0
    if (count == 0 && prev_count != 0) {
      loop();
    }
#endif
    loop();

    k_sleep(K_MSEC(1));

    frames++;
    if (k_uptime_get_32() > fps_stop) {
      int fps = (frames * 1000) / (1000 + k_uptime_get_32() - fps_stop);
      sprintf(count_str, "FPS: %d", fps);
      lv_label_set_text(count_label, count_str);
      fps_stop = k_uptime_get_32() + 1000;
      frames = 0;
    }
  }
}
