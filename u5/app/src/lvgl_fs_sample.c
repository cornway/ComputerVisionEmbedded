//#include <lv_fs.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lvgl_fs_sample_config);

#include <lvgl.h>

#define MOUNT_POINT "/NAND:"
#define MAX_PATH 128

// File structure wrapper
typedef struct {
    struct fs_file_t file;
} lvgl_fs_file_t;

// Directory structure wrapper
typedef struct {
    struct fs_dir_t dir;
    struct fs_dirent entry;
} lvgl_fs_dir_t;

static bool _ready_cb(struct _lv_fs_drv_t *drv);
static void *_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t _close_cb(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t _read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t _write_cb(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static lv_fs_res_t _seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t _tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);
static void *_dir_open_cb(lv_fs_drv_t *drv, const char *path);
static lv_fs_res_t _dir_read_cb(lv_fs_drv_t *drv, void *rdir_p, char *fn, uint32_t fn_len);
static lv_fs_res_t _dir_close_cb(lv_fs_drv_t *drv, void *rdir_p);

void lvgl_fs_sample ()
{
    static lv_fs_drv_t drv;                   /*Needs to be static or global*/
    lv_fs_drv_init(&drv);                     /*Basic initialization*/

    drv.letter = 'S';                         /*An uppercase letter to identify the drive */
    drv.ready_cb = _ready_cb;                 /*Callback to tell if the drive is ready to use */
    drv.open_cb = _open_cb;                   /*Callback to open a file */
    drv.close_cb = _close_cb;                 /*Callback to close a file */
    drv.read_cb = _read_cb;                   /*Callback to read a file */
    drv.write_cb = _write_cb;                 /*Callback to write a file */
    drv.seek_cb = _seek_cb;                   /*Callback to seek in a file (Move cursor) */
    drv.tell_cb = _tell_cb;                   /*Callback to tell the cursor position  */

    drv.dir_open_cb = _dir_open_cb;           /*Callback to open directory to read its content */
    drv.dir_read_cb = _dir_read_cb;           /*Callback to read a directory's content */
    drv.dir_close_cb = _dir_close_cb;         /*Callback to close a directory */

    drv.user_data = NULL;             /*Any custom data if required*/

    lv_fs_drv_register(&drv);                 /*Finally register the drive*/
}


static bool _ready_cb(struct _lv_fs_drv_t *drv)
{
    return true;  // For most cases, if mounted properly
}

static void *_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    lvgl_fs_file_t *f = (lvgl_fs_file_t *)malloc(sizeof(lvgl_fs_file_t));

    if (!f) return NULL;

    fs_file_t_init(&f->file);

    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), MOUNT_POINT"%s", path);

    int flags = 0;
    if (mode == LV_FS_MODE_WR)
        flags = FS_O_WRITE | FS_O_CREATE;
    else if (mode == LV_FS_MODE_RD)
        flags = FS_O_READ;
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
        flags = FS_O_READ | FS_O_WRITE | FS_O_CREATE;

    int ret = fs_open(&f->file, full_path, flags);
    if (ret < 0) {
        free(f);
        return NULL;
    }

    return f;
}

static lv_fs_res_t _close_cb(lv_fs_drv_t *drv, void *file_p)
{
    lvgl_fs_file_t *f = file_p;

    fs_close(&f->file);
    free(f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t _read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    lvgl_fs_file_t *f = file_p;

    ssize_t r = fs_read(&f->file, buf, btr);
    if (r < 0) return LV_FS_RES_UNKNOWN;
    *br = r;
    return LV_FS_RES_OK;
}

static lv_fs_res_t _write_cb(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    lvgl_fs_file_t *f = file_p;
    ssize_t w = fs_write(&f->file, buf, btw);
    if (w < 0) return LV_FS_RES_UNKNOWN;
    *bw = w;
    return LV_FS_RES_OK;
}

static lv_fs_res_t _seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    lvgl_fs_file_t *f = file_p;

    int z_whence;
    switch (whence) {
        case LV_FS_SEEK_SET: z_whence = FS_SEEK_SET; break;
        case LV_FS_SEEK_CUR: z_whence = FS_SEEK_CUR; break;
        case LV_FS_SEEK_END: z_whence = FS_SEEK_END; break;
        default: return LV_FS_RES_INV_PARAM;
    }

    int ret = fs_seek(&f->file, pos, z_whence);
    return (ret < 0) ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

static lv_fs_res_t _tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    lvgl_fs_file_t *f = file_p;

    off_t pos = fs_tell(&f->file);
    if (pos < 0) return LV_FS_RES_UNKNOWN;
    *pos_p = pos;
    return LV_FS_RES_OK;
}

static void *_dir_open_cb(lv_fs_drv_t *drv, const char *path)
{
    lvgl_fs_dir_t *d = (lvgl_fs_dir_t *)malloc(sizeof(lvgl_fs_dir_t));

    if (!d) return NULL;

    fs_dir_t_init(&d->dir);

    char full_path[128];
    snprintf(full_path, sizeof(full_path), MOUNT_POINT"/%s", path);

    if (fs_opendir(&d->dir, full_path) < 0) {
        free(d);
        return NULL;
    }

    return d;
}

static lv_fs_res_t _dir_read_cb(lv_fs_drv_t *drv, void *rdir_p, char *fn, uint32_t fn_len)
{
    lvgl_fs_dir_t *d = rdir_p;
    int ret = fs_readdir(&d->dir, &d->entry);
    if (ret < 0 || d->entry.name[0] == 0) {
        fn[0] = '\0'; // No more files
        return LV_FS_RES_OK;
    }

    strncpy(fn, d->entry.name, LV_FS_MAX_FN_LENGTH);
    return LV_FS_RES_OK;
}

static lv_fs_res_t _dir_close_cb(lv_fs_drv_t *drv, void *rdir_p)
{
    lvgl_fs_dir_t *d = rdir_p;
    fs_closedir(&d->dir);
    free(d);
    return LV_FS_RES_OK;
}

uint8_t *lvgl_fs_load_raw (const char *path, uint8_t *buffer, size_t size)
{
    struct fs_file_t file;
    fs_file_t_init(&file);

    int ret = fs_open(&file, path, FS_O_READ);
    if (ret < 0) {
        LOG_ERR("Failed to open %s: %d\n", path, ret);
        return NULL;
    }

    ssize_t read_bytes = fs_read(&file, buffer, size);
    fs_close(&file);

    if (read_bytes != size) {
        LOG_ERR("Read %d bytes, expected %d\n", (int)read_bytes, (int)size);
        return NULL;
    }

    return buffer;
}