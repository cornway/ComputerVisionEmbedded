

#include <zephyr/drivers/display.h>
#include <zephyr/drivers/video.h>

#include "gf/video.hpp"

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(video);

namespace Video {

#if defined(CONFIG_GRINREFLEX_VIDEO_WIDTH) &&                                  \
    defined(CONFIG_GRINREFLEX_VIDEO_HEIGHT)

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
static struct video_buffer *buffers[2];

#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
#define SECOND_BUFFER_SIZE                                                     \
  (CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * 2)
#else
#define SECOND_BUFFER_SIZE                                                     \
  (CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * 2)
#endif

static struct video_buffer second_buffer {};
__attribute__((
    aligned(32))) static uint8_t video_large_buffer[SECOND_BUFFER_SIZE];

void setup() {
  struct video_format fmt;
  struct video_caps caps;
  enum video_buf_type type = VIDEO_BUF_TYPE_OUTPUT;
  struct video_selection sel = {
      .type = VIDEO_BUF_TYPE_OUTPUT,
  };
  size_t bsize;
  int i = 0;
  int err;

  if (!device_is_ready(video_dev)) {
    LOG_ERR("%s device is not ready", video_dev->name);
    return;
  }

  LOG_INF("- Device name: %s", video_dev->name);

  /* Get capabilities */
  caps.type = type;
  if (video_get_caps(video_dev, &caps)) {
    LOG_ERR("Unable to retrieve video capabilities");
    return;
  }

  LOG_INF("- Capabilities:");
  while (caps.format_caps[i].pixelformat) {
    const struct video_format_cap *fcap = &caps.format_caps[i];
    /* four %c to string */
    LOG_INF("  %c%c%c%c width [%u; %u; %u] height [%u; %u; %u]",
            (char)fcap->pixelformat, (char)(fcap->pixelformat >> 8),
            (char)(fcap->pixelformat >> 16), (char)(fcap->pixelformat >> 24),
            fcap->width_min, fcap->width_max, fcap->width_step,
            fcap->height_min, fcap->height_max, fcap->height_step);
    i++;
  }

  /* Get default/native format */
  fmt.type = type;
  if (video_get_format(video_dev, &fmt)) {
    LOG_ERR("Unable to retrieve video format");
    return;
  }

  /* Set the crop setting if necessary */
#if CONFIG_VIDEO_SOURCE_CROP_WIDTH && CONFIG_VIDEO_SOURCE_CROP_HEIGHT
  sel.target = VIDEO_SEL_TGT_CROP;
  sel.rect.left = CONFIG_VIDEO_SOURCE_CROP_LEFT;
  sel.rect.top = CONFIG_VIDEO_SOURCE_CROP_TOP;
  sel.rect.width = CONFIG_VIDEO_SOURCE_CROP_WIDTH;
  sel.rect.height = CONFIG_VIDEO_SOURCE_CROP_HEIGHT;
  if (video_set_selection(video_dev, &sel)) {
    LOG_ERR("Unable to set selection crop");
    return;
  }
  LOG_INF("Selection crop set to (%u,%u)/%ux%u", sel.rect.left, sel.rect.top,
          sel.rect.width, sel.rect.height);
#endif

  /* Set format */
  fmt.width = CONFIG_GRINREFLEX_VIDEO_WIDTH;
  fmt.height = CONFIG_GRINREFLEX_VIDEO_HEIGHT;
#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
  fmt.pixelformat = VIDEO_PIX_FMT_JPEG;
#else
  fmt.pixelformat = VIDEO_PIX_FMT_RGB565;
#endif
  /*
   * Check (if possible) if targeted size is same as crop
   * and if compose is necessary
   */
  sel.target = VIDEO_SEL_TGT_CROP;
  err = video_get_selection(video_dev, &sel);
  if (err < 0 && err != -ENOSYS) {
    LOG_ERR("Unable to get selection crop");
    return;
  }

  if (err == 0 &&
      (sel.rect.width != fmt.width || sel.rect.height != fmt.height)) {
    sel.target = VIDEO_SEL_TGT_COMPOSE;
    sel.rect.left = 0;
    sel.rect.top = 0;
    sel.rect.width = fmt.width;
    sel.rect.height = fmt.height;
    err = video_set_selection(video_dev, &sel);
    if (err < 0 && err != -ENOSYS) {
      LOG_ERR("Unable to set selection compose");
      return;
    }
  }

  if (video_set_format(video_dev, &fmt)) {
    LOG_ERR("Unable to set up video format");
    return;
  }

  LOG_INF("- Format: %c%c%c%c %ux%u %u", (char)fmt.pixelformat,
          (char)(fmt.pixelformat >> 8), (char)(fmt.pixelformat >> 16),
          (char)(fmt.pixelformat >> 24), fmt.width, fmt.height, fmt.pitch);

#ifdef LINE_COUNT_HEIGHT
  if (caps.min_line_count != LINE_COUNT_HEIGHT) {
    LOG_ERR("Partial framebuffers not supported by this sample");
    return;
  }
#endif
  /* Size to allocate for each buffer */
  bsize = fmt.pitch * fmt.height;

  bsize = CONFIG_VIDEO_BUFFER_POOL_SZ_MAX;

  buffers[0] = video_buffer_aligned_alloc(bsize, CONFIG_VIDEO_BUFFER_POOL_ALIGN,
                                          K_FOREVER);
  if (buffers[0] == NULL) {
    LOG_ERR("Unable to alloc video buffer");
    return;
  }
  buffers[0]->type = type;
  video_enqueue(video_dev, buffers[0]);

  buffers[1] = &second_buffer;

  buffers[1]->type = type;
  buffers[1]->buffer = video_large_buffer;
  buffers[1]->size = sizeof(video_large_buffer);

  video_enqueue(video_dev, buffers[1]);

  /* Set controls */
  struct video_control ctrl = {.id = VIDEO_CID_HFLIP, .val = 1};

  if (IS_ENABLED(CONFIG_VIDEO_HFLIP)) {
    video_set_ctrl(video_dev, &ctrl);
  }

  if (IS_ENABLED(CONFIG_VIDEO_VFLIP)) {
    ctrl.id = VIDEO_CID_VFLIP;
    video_set_ctrl(video_dev, &ctrl);
  }

#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
  ctrl.id = VIDEO_CID_JPEG_COMPRESSION_QUALITY;
  ctrl.val = CONFIG_GRINREFLEX_JPEG_VIDEO_QUALITY;
  video_set_ctrl(video_dev, &ctrl);
#endif

  /* Start video capture */
  if (video_stream_start(video_dev, type)) {
    LOG_ERR("Unable to start capture (interface)");
    return;
  }

  LOG_INF("- Capture started");
}

#endif /*defined(CONFIG_GRINREFLEX_VIDEO_WIDTH) &&                             \
    defined(CONFIG_GRINREFLEX_VIDEO_HEIGHT)*/

} // namespace Video