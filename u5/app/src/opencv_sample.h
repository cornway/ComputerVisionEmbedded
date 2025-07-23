

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_OPENCV_LIB)
void opencv_test ();

void edge_detect(const uint8_t *rgb_in,
                 uint8_t *edges_out,
                 int width, int height,
                 double low_thresh,
                 double high_thresh);


const uint8_t* image_data(void);
size_t image_size(void);

int edge_demo(const char *path,
                 uint8_t *edges_out,
                 int *width, int *height,
                 double low_thresh,
                 double high_thresh);

int object_detect (const char *img_path, const char *cascade_path);
          
int object_detect_2(const char* img_path,
                             const char* cascade_path,
                             uint8_t**   out_buf,     // RGB888 buffer you can feed LVGL
                             int*        out_w,
                             int*        out_h,
                             size_t*     out_data_sz);

#endif

#ifdef __cplusplus
}
#endif