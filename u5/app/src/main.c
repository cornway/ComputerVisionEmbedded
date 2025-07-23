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
#include <stdio.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_msc.h>

#include <sample_usbd.h>
#include <lvgl_fs.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#if defined(CONFIG_TFLITE_LIB)
#include <app/lib/tflm.h>
#endif

#if defined(CONFIG_OPENCV_LIB)
#include <opencv_sample.h>
#endif

#define AUTOMOUNT_NODE DT_NODELABEL(ffs2)
FS_FSTAB_DECLARE_ENTRY(AUTOMOUNT_NODE);

static uint32_t count;
static struct usbd_context *sample_usbd;

USBD_DEFINE_MSC_LUN(flash, "NAND", "Zephyr", "FlashDisk", "0.00");

static int enable_usb_device_next(void)
{
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

static void lv_btn_click_callback(lv_event_t *e)
{
	ARG_UNUSED(e);

	count = 0;
}

void fs_example ()
{
	/* You can get direct mount point to automounted node too */
	struct fs_mount_t *mp = &FS_FSTAB_ENTRY(AUTOMOUNT_NODE);
	struct fs_dir_t dir;
	struct fs_statvfs sbuf;
	int rc;

	fs_dir_t_init(&dir);
	/* Allow log messages to flush to avoid interleaved output */
	k_sleep(K_MSEC(50));

	printk("Mount %s\n", mp->mnt_point);

	rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc < 0) {
		printk("FAIL: statvfs: %d\n", rc);
		return;
	}

	printk("%s: bsize = %lu ; frsize = %lu ;"
	       " blocks = %lu ; bfree = %lu\n",
	       mp->mnt_point,
	       sbuf.f_bsize, sbuf.f_frsize,
	       sbuf.f_blocks, sbuf.f_bfree);

	rc = fs_opendir(&dir, mp->mnt_point);
	printk("%s opendir: %d\n", mp->mnt_point, rc);

	if (rc < 0) {
		LOG_ERR("Failed to open directory");
	}

	while (rc >= 0) {
		struct fs_dirent ent = { 0 };

		rc = fs_readdir(&dir, &ent);
		if (rc < 0) {
			LOG_ERR("Failed to read directory entries");
			break;
		}
		if (ent.name[0] == 0) {
			printk("End of files\n");
			break;
		}
		printk("  %c %u %s\n",
		       (ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
		       ent.size,
		       ent.name);
	}

	(void)fs_closedir(&dir);

	return;
}

int main(void)
{
	char count_str[64] = {0};
	const struct device *display_dev;
	lv_obj_t *count_label;

#if defined(CONFIG_OPENCV_LIB)
	opencv_test();
#endif

	fs_example();

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
		lv_obj_align(hello_world_button, LV_ALIGN_TOP_LEFT, 0, -15);
		lv_obj_add_event_cb(hello_world_button, lv_btn_click_callback, LV_EVENT_CLICKED,
				    NULL);
		hello_world_label = lv_label_create(hello_world_button);
	} else {
		hello_world_label = lv_label_create(lv_screen_active());
	}

	lv_label_set_text(hello_world_label, "Hello world!");
	lv_obj_align(hello_world_label, LV_ALIGN_TOP_LEFT, 0, 0);

	count_label = lv_label_create(lv_screen_active());
	lv_obj_align(count_label, LV_ALIGN_BOTTOM_MID, 0, 0);

#if defined(CONFIG_OPENCV_LIB)
#if 0
	#define IMG_WIDTH       320
	#define IMG_HEIGHT      240
	#define IMG_BPP         3
	#define IMG_SIZE        (IMG_WIDTH * IMG_HEIGHT * IMG_BPP)

	static uint8_t edges_buf[IMG_SIZE/IMG_BPP];

	uint32_t start_ms = k_uptime_get_32();
	//edge_detect(image_data(), edges_buf, IMG_WIDTH, IMG_HEIGHT, 50.0, 150.0);

	int width, height;
	if (0 == edge_demo("/NAND:/sample2.png", edges_buf, &width, &height, 50.0, 150.0))
	{

		uint32_t end_ms = k_uptime_get_32();
		uint32_t elapsed = end_ms - start_ms;

		printf("Routine took %u ms\n", elapsed);

		static lv_img_dsc_t img_dsc;
		img_dsc.header.w = IMG_WIDTH;
		img_dsc.header.h = IMG_HEIGHT;
		img_dsc.header.cf = LV_COLOR_FORMAT_A8; //
		img_dsc.data_size = IMG_SIZE/IMG_BPP;
		img_dsc.data = edges_buf;

		lv_obj_t *img = lv_img_create(lv_screen_active());

		lv_img_set_src(img, &img_dsc);
		lv_obj_center(img);
	}
	
#else 

	// When you’re done with the image (and no longer show it), free the buffer:
	//free((void*)img_dsc.data);

#endif
#else
	#define IMG_WIDTH       96
	#define IMG_HEIGHT      96
	#define IMG_BPP         3
	#define IMG_SIZE        (IMG_WIDTH * IMG_HEIGHT * IMG_BPP)

	static uint8_t img_buf[IMG_SIZE];

	static uint8_t edges_buf[IMG_SIZE/IMG_BPP];
	
	if (lvgl_fs_load_raw("/NAND:/test.bin", img_buf, IMG_SIZE))
	{

		static lv_img_dsc_t img_dsc;
		img_dsc.header.w = IMG_WIDTH;
		img_dsc.header.h = IMG_HEIGHT;
		img_dsc.header.cf = LV_COLOR_FORMAT_A8; //
		img_dsc.data_size = IMG_SIZE/IMG_BPP;
		img_dsc.data = edges_buf;

		lv_obj_t *img = lv_img_create(lv_screen_active());

#if defined(CONFIG_TFLITE_LIB)
		tflm_init();
		inference_task(img_buf);
#endif

		lv_img_set_src(img, &img_dsc);
		lv_obj_center(img);
	}
#endif

	lv_timer_handler();
	display_blanking_off(display_dev);

	uint32_t prev_count = count;
	while (1) {
		sprintf(count_str, "%d", count);
		lv_label_set_text(count_label, count_str);
		lv_timer_handler();

		if (count == 0 && prev_count != 0)
		{
#if defined(CONFIG_OPENCV_LIB)
			//int faces = object_detect("/NAND:/face.png", "/NAND:/alt.xml");

			uint8_t *out_buf;
			int out_w;
			int out_h;
			int out_data_sz;

			int faces = object_detect_2(
				"/NAND:/face.png", "/NAND:/alt.xml",
				&out_buf, &out_w, &out_h, &out_data_sz
			);

			printf("out_w = %u out_h = %u out_data_sz = %u\n", out_w, out_h, out_data_sz);

			static lv_img_dsc_t img_dsc;
			img_dsc.header.w  = out_w;
			img_dsc.header.h  = out_h;
			img_dsc.header.cf = LV_COLOR_FORMAT_RGB888;   // RGB888
			img_dsc.data_size = out_data_sz;
			img_dsc.data      = out_buf;

			lv_obj_t* img = lv_img_create(lv_screen_active());
			lv_img_set_src(img, &img_dsc);
			lv_obj_center(img);
#endif
		}

		prev_count = count;

		++count;
		k_sleep(K_MSEC(100));
	}
}
