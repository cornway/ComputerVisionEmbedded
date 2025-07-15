#ifndef _LVGL_FS_H_
#define _LVGL_FS_H_

void lvgl_fs_sample ();

uint8_t *lvgl_fs_load_raw (const char *path, uint8_t *buffer, size_t size);

#endif /*_LVGL_FS_H_*/