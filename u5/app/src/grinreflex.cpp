

#include <iostream>
#include <vector>

#if defined(CONFIG_OPENCV_LIB)
#include "opencv2/opencv.hpp"
#endif 

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
#include <sample_usbd.h>

#include <lvgl_fs.h>
#include <sample_usbd.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(grinreflex_app);

#include "grinreflex.h"
#include "video.hpp"
#include "gfx_utils.hpp"

#include <app/drivers/jpeg.h>

#if defined(CONFIG_OPENCV_LIB)
#include "opencv_utils.hpp"
#endif

#if defined(CONFIG_QUIRC_LIB)
#include "quirc.h"
#endif

#define AUTOMOUNT_NODE DT_NODELABEL(ffs2)
FS_FSTAB_DECLARE_ENTRY(AUTOMOUNT_NODE);

static struct usbd_context *sample_usbd;
static lv_obj_t *vid_canvas;

#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
static uint8_t jpeg_frame_buffer[CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888)];
static uint8_t rgb_frame_buffer[CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888)];
#endif

#if defined(CONFIG_OPENCV_LIB)
#define GRAY_FRAME_WIDTH 80
#define GRAY_FRAME_HEIGHT 80
#else
#define GRAY_FRAME_WIDTH 160
#define GRAY_FRAME_HEIGHT 80
#endif

static uint8_t gray_frame_buffer[GRAY_FRAME_WIDTH * GRAY_FRAME_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8)];

USBD_DEFINE_MSC_LUN(flash, "NAND", "Zephyr", "FlashDisk", "0.00");

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
const struct device *jpeg_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_jpeg));

#if defined(CONFIG_OPENCV_LIB)
static lv_obj_t *RedGreenDot;
static lv_obj_t *FaceRect;
static cv::CascadeClassifier faceCascade;
static cv::QRCodeDetector qrCodeDetector;

#define OPECV_SHOWCASE OCV_SC_FACE_DETECT

#endif

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

static lv_img_dsc_t video_img;
static lv_obj_t *screen;

int init()
{
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return -ENODEV;
	}

    if (!device_is_ready(jpeg_dev)) {
        printf("%s JPEG device not ready", jpeg_dev->name);
        return -ENODEV;
    }

	if (0 && enable_usb_device_next() != 0) {
		LOG_ERR("Failed to enable USB");
        return -ENODEV;
    }

#if defined(CONFIG_OPENCV_LIB)

    cv::setNumThreads(1);
    cv::setUseOptimized(false);

#if (OPECV_SHOWCASE == OCV_SC_FACE_DETECT)
    const char *faceCascadePath = NAND_PATH("/cascface.xml");
    if (false == loadCascade(faceCascade, faceCascadePath)) {
        return -3;
    }
#endif
#endif

    Video::setup();

    vid_canvas = lv_canvas_create(lv_screen_active());
    lv_canvas_fill_bg(vid_canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
    lv_obj_center(vid_canvas);

    display_blanking_off(display_dev);

    video_img.header.w = CONFIG_GRINREFLEX_VIDEO_WIDTH;
	video_img.header.h = CONFIG_GRINREFLEX_VIDEO_HEIGHT;
#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
	video_img.data_size = CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888);
	video_img.header.cf = LV_COLOR_FORMAT_RGB888;
#else
	video_img.data_size = CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_NATIVE);
	video_img.header.cf = LV_COLOR_FORMAT_NATIVE;
#endif
    screen = lv_img_create(lv_scr_act());

#if defined(CONFIG_OPENCV_LIB)
    RedGreenDot = lv_obj_create(lv_scr_act());
    lv_obj_set_size(RedGreenDot, 30, 30);                    // dot size
    lv_obj_set_style_radius(RedGreenDot, LV_RADIUS_CIRCLE, 0); // make it round
    lv_obj_set_style_border_width(RedGreenDot, 0, 0);
    lv_obj_set_style_bg_opa(RedGreenDot, LV_OPA_COVER, 0);
    lv_obj_clear_flag(RedGreenDot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(RedGreenDot, LV_ALIGN_LEFT_MID, 4, 0);      // 4px from left
    lv_obj_move_foreground(RedGreenDot);                     // keep it on top

    FaceRect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(FaceRect, 0, 0);
    lv_obj_set_size(FaceRect, 1, 1);

    // Unfilled, just a thick border
    lv_obj_set_style_bg_opa(FaceRect, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(FaceRect, 4, LV_PART_MAIN);
    lv_obj_set_style_border_opa(FaceRect, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(FaceRect, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_obj_set_style_radius(FaceRect, 0, LV_PART_MAIN);      // 0 = sharp corners
    lv_obj_set_style_pad_all(FaceRect, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(FaceRect, LV_OPA_TRANSP, LV_PART_MAIN); // no focus outline
    lv_obj_set_style_shadow_opa(FaceRect, LV_OPA_TRANSP, LV_PART_MAIN);  // no shadow
    lv_obj_move_foreground(FaceRect);
    lv_obj_add_flag(FaceRect, LV_OBJ_FLAG_HIDDEN);

#endif
    return 0;
}

int loop()
{
    struct video_buffer vbuf{};
    struct video_buffer *vbuf_ptr = &vbuf;
    struct jpeg_out_prop jpeg_prop;

    vbuf_ptr->type = VIDEO_BUF_TYPE_OUTPUT;
    
    int err = video_dequeue(video_dev, &vbuf_ptr, K_FOREVER);
    if (err) {
        LOG_ERR("Unable to dequeue video buf");
        return 0;
    }

#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
    jpeg_hw_decode(jpeg_dev, (uint8_t *)vbuf_ptr->buffer, vbuf_ptr->bytesused, jpeg_frame_buffer);

    jpeg_hw_poll(jpeg_dev, 0, &jpeg_prop);

    //printf("JPEG done w = %d, h = %d, color = %d chroma = %d\n", jpeg_prop.width,
    //        jpeg_prop.height, jpeg_prop.color_space, jpeg_prop.chroma);

    jpeg_color_convert_helper(jpeg_dev, &jpeg_prop, jpeg_frame_buffer, rgb_frame_buffer);

    Gfx::GfxBuffer src{};
    Gfx::GfxBuffer dst{};

    src.buf = rgb_frame_buffer;
    src.width = CONFIG_GRINREFLEX_VIDEO_WIDTH;
    src.height = CONFIG_GRINREFLEX_VIDEO_HEIGHT;
    src.stride = CONFIG_GRINREFLEX_VIDEO_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888);
    src.cf = LV_COLOR_FORMAT_RGB888;
    src.size = sizeof(rgb_frame_buffer);

    dst.buf = gray_frame_buffer;
    dst.width = GRAY_FRAME_WIDTH;
    dst.height = GRAY_FRAME_HEIGHT;
    dst.stride = GRAY_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8);
    dst.cf = LV_COLOR_FORMAT_L8;
    dst.size = sizeof(gray_frame_buffer);

    Gfx::fit(vid_canvas, src, dst);
    lv_task_handler();
#else
    video_img.data = (uint8_t *)vbuf_ptr->buffer;
    lv_img_set_src(screen, &video_img);
	lv_obj_align(screen, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_task_handler();
#endif
    err = video_enqueue(video_dev, vbuf_ptr);
    if (err) {
        LOG_ERR("Unable to requeue video buf");
        return 0;
    }

#if defined(CONFIG_OPENCV_LIB)
#if (OPECV_SHOWCASE == OCV_SC_FACE_DETECT)
    cv::Mat mat_gray(GRAY_FRAME_WIDTH, GRAY_FRAME_HEIGHT, CV_8UC1, gray_frame_buffer);

    std::vector<cv::Rect> faces;

    bool facesFound = detectFaces(faceCascade, mat_gray, faces);

    lv_color_t c = facesFound ? lv_palette_main(LV_PALETTE_GREEN)
                                : lv_palette_main(LV_PALETTE_RED);
    lv_obj_set_style_bg_color(RedGreenDot, c, 0);

    if (facesFound) lv_obj_remove_flag(FaceRect, LV_OBJ_FLAG_HIDDEN);
    else         lv_obj_add_flag(FaceRect, LV_OBJ_FLAG_HIDDEN);

    for (const auto &r : faces) {
        //cv::rectangle(mat_gray, r, cv::Scalar(255, 255, 255), 2, cv::LINE_8);
        printf("faces = %d, %d %d\n", faces.size(), r.x, r.y);

        lv_obj_align_to(FaceRect, vid_canvas, LV_ALIGN_TOP_LEFT, r.x, r.y);
        lv_obj_set_size(FaceRect, r.width, r.height);
    }
#elif (OPECV_SHOWCASE == OCV_SC_QRCODE)
    cv::Mat gray(GRAY_FRAME_WIDTH, GRAY_FRAME_HEIGHT, CV_8UC1, gray_frame_buffer);
    cv::Mat bgr(jpeg_prop.width, jpeg_prop.height, CV_8UC3, rgb_frame_buffer);
    
    try {
        cv::Mat bin = cv_preprocessForQR(gray);

        cv::Mat ptsSingle;
        uint32_t start_ms = k_uptime_get_32();

        std::string text = qrCodeDetector.detectAndDecode(bin, ptsSingle);
        uint32_t end_ms = k_uptime_get_32();
        uint32_t elapsed = end_ms - start_ms;
        
        if (!text.empty()) {
            std::cout << "Decoded text : " << text << std::endl;
            std::cout << "Time taken " << elapsed << " ms" << std::endl;
        }
    } catch (cv::Exception e) {
        std::cout << "Caught exception " << e.what() << std::endl;
    }
#endif

#if defined(CONFIG_QUIRC_LIB)
#if (OPECV_SHOWCASE == OCV_SC_QRCODE_QUIRC)
    cv::Mat gray(GRAY_FRAME_WIDTH, GRAY_FRAME_HEIGHT, CV_8UC1, gray_frame_buffer);
    struct quirc *qr;

    qr = quirc_new();
    if (!qr) {
        std::cout << "quirc: Failed to allocate memory" << std::endl;
        return -ENOMEM;
    }

    if (quirc_resize(qr, GRAY_FRAME_WIDTH, GRAY_FRAME_HEIGHT) < 0) {
        std::cout << "quirc: Failed to allocate video memory" << std::endl;
        return -ENOMEM;
    }

    uint8_t *image;
    int w, h;
    bool qrDecodedOk = false;

    uint32_t start_ms = k_uptime_get_32();

    image = quirc_begin(qr, &w, &h);

    std::memcpy(image, gray.data, (size_t)(w * h));
    quirc_end(qr);

    int num_codes;
    int i;

    num_codes = quirc_count(qr);
    for (i = 0; i < num_codes; i++) {
        struct quirc_code code;
        struct quirc_data data;
        quirc_decode_error_t err;

        quirc_extract(qr, i, &code);

        /* Decoding stage */
        err = quirc_decode(&code, &data);
        if (err) {
            printf("DECODE FAILED: %s\n", quirc_strerror(err));
            return -EPERM;
        } else {
            qrDecodedOk = true;
            printf("Data: %s\n", data.payload);
        }
    }

    quirc_destroy(qr);

    uint32_t end_ms = k_uptime_get_32();
    uint32_t elapsed = end_ms - start_ms;

    if (qrDecodedOk) {
        std::cout << "Time Taken : " << elapsed << " ms" << std::endl; 
    }

#endif /* (OPECV_SHOWCASE == OCV_SC_QRCODE_QUIRC)  */
#endif /* defined(CONFIG_QUIRC_LIB) */
#endif /* defined(CONFIG_OPENCV_LIB) */
    return 0;
}