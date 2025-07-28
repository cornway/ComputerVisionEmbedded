
#include <iostream>

#include "lvgl.h"

#include "gfx_utils.hpp"
#if defined(CONFIG_OPENCV_LIB)
#include "opencv_utils.hpp"
#endif

#include "uart.hpp"
#include <app/drivers/jpeg.h>

#include "application.h"

#define JPEG_DEVICE_NODE DT_CHOSEN(zephyr_jpeg)

#define NAND_PATH(_path) ("/NAND:" _path)

#if defined(CONFIG_OPENCV_LIB)
static cv::CascadeClassifier faceCascade;
#endif

#define UART_RING_BUF_SIZE 8192
static uint8_t uart_ring_buf[UART_RING_BUF_SIZE];

static lv_obj_t *canvas;
static lv_obj_t *jpeg_img;

UartEx uartEx = UartEx(uart_ring_buf, sizeof(uart_ring_buf));

const struct device *jpeg_dev = DEVICE_DT_GET(JPEG_DEVICE_NODE);

int init() {

#if defined(CONFIG_OPENCV_LIB)
  const char *faceCascadePath = NAND_PATH("/alt.xml");

  cv::setNumThreads(1);

  uint32_t start_ms = k_uptime_get_32();

  if (!faceCascade.load(faceCascadePath)) {
    std::cerr << "Cannot load cascade: " << faceCascadePath << "\n";
    return -3;
  }

  uint32_t end_ms = k_uptime_get_32();
  uint32_t elapsed = end_ms - start_ms;

  std::cout << "faceCascade.load : " << elapsed << std::endl;
#endif

  canvas = lv_canvas_create(lv_screen_active());
  lv_canvas_fill_bg(canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
  lv_obj_center(canvas);

  jpeg_img = lv_img_create(lv_screen_active());

  uartEx.init();
  uartEx.trigger();

  if (!device_is_ready(jpeg_dev)) {
    printf("%s JPEG device not ready", jpeg_dev->name);
    return -ENODEV;
  }
  return 0;
}

#define TMP_BUF_WIDTH 400
#define TMP_BUF_HEIGHT 400

static uint8_t tmp_buf[TMP_BUF_WIDTH * TMP_BUF_HEIGHT];

int loop() {
  uint32_t jpeg_buf_len = uartEx.tryReadHeader();

  if (0 == jpeg_buf_len) {
    return 0;
  }

  // memset(tmp_buf, 0, sizeof(tmp_buf));

  static uint8_t uart_buf[8192];
  static uint8_t yuyv_buf[80 * 80 * 3];
  static uint8_t rgb_buf[80 * 80 * 3];
  struct jpeg_out_prop jpeg_prop;

  jpeg_buf_len = uartEx.tryRead(uart_buf, jpeg_buf_len);

  uartEx.trigger();

  jpeg_buf_len = ROUND_UP(jpeg_buf_len, 4);

  jpeg_hw_decode(jpeg_dev, uart_buf, jpeg_buf_len, yuyv_buf);

  jpeg_hw_poll(jpeg_dev, 0, &jpeg_prop);

  printf("JPEG done w = %d, h = %d, color = %d chroma = %d\n", jpeg_prop.width,
         jpeg_prop.height, jpeg_prop.color_space, jpeg_prop.chroma);

  jpeg_color_convert_helper(jpeg_dev, &jpeg_prop, yuyv_buf, rgb_buf);

  // memset(rgb_buf, 0xff, sizeof(rgb_buf));
  uint32_t rgb_size = 80 * 80 * 3;

  cv::Mat mat_rgb(jpeg_prop.height, jpeg_prop.width, CV_8UC3, rgb_buf);
  cv::Mat mat_gray;

  cv::cvtColor(mat_rgb, mat_gray, cv::COLOR_RGB2GRAY);

  const cv::Size minSize(20, 20);
  const cv::Size maxSize(mat_gray.cols, mat_gray.rows);
  const double scaleFactor = 1.3;
  const int minNeighbors = 3;
  const int flags = 0;

  std::vector<cv::Rect> faces;

  uint32_t start_ms = k_uptime_get_32();

  faceCascade.detectMultiScale(mat_gray, faces, scaleFactor, minNeighbors,
                               flags, minSize, maxSize);

  uint32_t end_ms = k_uptime_get_32();
  uint32_t elapsed = end_ms - start_ms;

  printf("detectMultiScale : %d ms\n", elapsed);

  for (const auto &r : faces) {
    cv::rectangle(mat_gray, r, cv::Scalar(255, 255, 255), 2, cv::LINE_8);
    std::cout << "detectMultiScale : " << elapsed << std::endl;
    printf("faces = %d, %d %d\n", faces.size(), mat_gray.cols, mat_gray.rows);
  }

  static Gfx::GfxBuffer src;
  static Gfx::GfxBuffer dst;

  src.buf = mat_gray.data;
  src.width = mat_gray.cols;
  src.height = mat_gray.rows;
  src.stride = mat_gray.cols;
  src.cf = LV_COLOR_FORMAT_L8;
  src.size = mat_gray.rows * mat_gray.cols;

  dst.buf = tmp_buf;
  dst.width = TMP_BUF_WIDTH;
  dst.height = TMP_BUF_HEIGHT;
  dst.stride = TMP_BUF_WIDTH;
  dst.size = sizeof(tmp_buf);
  dst.cf = LV_COLOR_FORMAT_L8;

  Gfx::fit(canvas, src, dst);

  return 0;
}

/////////////////// Examples /////////////////////////

#if 0

TFLite:

#define IMG_WIDTH 96
#define IMG_HEIGHT 96
#define IMG_BPP 3
#define IMG_SIZE (IMG_WIDTH * IMG_HEIGHT * IMG_BPP)

  static uint8_t img_buf[IMG_SIZE];

  static uint8_t edges_buf[IMG_SIZE / IMG_BPP];

  if (lvgl_fs_load_raw("/NAND:/test.bin", img_buf, IMG_SIZE)) {

    static lv_img_dsc_t img_dsc;
    img_dsc.header.w = IMG_WIDTH;
    img_dsc.header.h = IMG_HEIGHT;
    img_dsc.header.cf = LV_COLOR_FORMAT_A8; //
    img_dsc.data_size = IMG_SIZE / IMG_BPP;
    img_dsc.data = edges_buf;

    lv_obj_t *img = lv_img_create(lv_screen_active());

#if defined(CONFIG_TFLITE_LIB)
    tflm_init();
    inference_task(img_buf);
#endif

    lv_img_set_src(img, &img_dsc);
    lv_obj_center(img);
  }

Edges

#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define IMG_BPP 3
#define IMG_SIZE (IMG_WIDTH * IMG_HEIGHT * IMG_BPP)

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

void fs_example() {
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
         mp->mnt_point, sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks,
         sbuf.f_bfree);

  rc = fs_opendir(&dir, mp->mnt_point);
  printk("%s opendir: %d\n", mp->mnt_point, rc);

  if (rc < 0) {
    LOG_ERR("Failed to open directory");
  }

  while (rc >= 0) {
    struct fs_dirent ent = {0};

    rc = fs_readdir(&dir, &ent);
    if (rc < 0) {
      LOG_ERR("Failed to read directory entries");
      break;
    }
    if (ent.name[0] == 0) {
      printk("End of files\n");
      break;
    }
    printk("  %c %u %s\n", (ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
           ent.size, ent.name);
  }

  (void)fs_closedir(&dir);

  return;
}

#endif