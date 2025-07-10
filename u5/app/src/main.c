/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>

#include <zephyr/fs/fs.h>
#include <ff.h>
#include <stdio.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#define AUTOMOUNT_NODE DT_NODELABEL(ffs2)
FS_FSTAB_DECLARE_ENTRY(AUTOMOUNT_NODE);

#define FILE_PATH "/FLASHDISK:/hello.txt"

static uint32_t count;

static void lv_btn_click_callback(lv_event_t *e)
{
	ARG_UNUSED(e);

	count = 0;
}

int fs_example ()
{
	struct fs_file_t file;
	int rc;
	static const char data[] = "Hello";
	/* You can get direct mount point to automounted node too */
	struct fs_mount_t *auto_mount_point = &FS_FSTAB_ENTRY(AUTOMOUNT_NODE);
	struct fs_dirent stat;

	fs_file_t_init(&file);

	rc = fs_open(&file, FILE_PATH, FS_O_CREATE | FS_O_WRITE);
	if (rc != 0) {
		LOG_ERR("Accessing filesystem failed");
		return rc;
	}

	rc = fs_write(&file, data, strlen(data));
	if (rc != strlen(data)) {
		LOG_ERR("Writing filesystem failed");
		return rc;
	}

	rc = fs_close(&file);
	if (rc != 0) {
		LOG_ERR("Closing file failed");
		return rc;
	}

	/* You can unmount the automount node */
	rc = fs_unmount(auto_mount_point);
	if (rc != 0) {
		LOG_ERR("Failed to do unmount");
		return rc;
	};

	/* And mount it back */
	rc = fs_mount(auto_mount_point);
	if (rc != 0) {
		LOG_ERR("Failed to remount the auto-mount node");
		return rc;
	}

	/* Is the file still there? */
	rc = fs_stat(FILE_PATH, &stat);
	if (rc != 0) {
		LOG_ERR("File status check failed %d", rc);
		return rc;
	}

	LOG_INF("Filesystem access successful");
	return rc;
}

int main(void)
{
	char count_str[11] = {0};
	const struct device *display_dev;
	lv_obj_t *hello_world_label;
	lv_obj_t *count_label;

	fs_example();

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	if (IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
		lv_obj_t *hello_world_button;

		hello_world_button = lv_button_create(lv_screen_active());
		lv_obj_align(hello_world_button, LV_ALIGN_CENTER, 0, -15);
		lv_obj_add_event_cb(hello_world_button, lv_btn_click_callback, LV_EVENT_CLICKED,
				    NULL);
		hello_world_label = lv_label_create(hello_world_button);
	} else {
		hello_world_label = lv_label_create(lv_screen_active());
	}

	lv_label_set_text(hello_world_label, "Hello world!");
	lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, 0);

	count_label = lv_label_create(lv_screen_active());
	lv_obj_align(count_label, LV_ALIGN_BOTTOM_MID, 0, 0);

	lv_timer_handler();
	display_blanking_off(display_dev);

	while (1) {
		if ((count % 100) == 0U) {
			sprintf(count_str, "%d", count/100U);
			lv_label_set_text(count_label, count_str);
		}
		lv_timer_handler();
		++count;
		k_sleep(K_MSEC(10));
	}
}
