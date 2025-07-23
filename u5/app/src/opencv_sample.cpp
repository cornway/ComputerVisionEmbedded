


#if defined(CONFIG_OPENCV_LIB)

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include <iostream>
#include <cstring>

#include <sys/time.h>

#include <opencv2/core.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#define STR_(x) #x
#define STR(x)  STR_(x)

extern const uint8_t _binary_image_raw_start[];
extern const uint8_t _binary_image_raw_end[];

__asm__ (
    ".section .rodata\n"
    ".global _binary_image_raw_start\n"
    "_binary_image_raw_start:\n"
    ".incbin \"" STR(CV_SAMPLE_IMAGE) "\"\n"
    ".global _binary_image_raw_end\n"
    "_binary_image_raw_end:\n"
    ".text\n"
);

extern "C" size_t image_size(void) {
    return _binary_image_raw_end - _binary_image_raw_start;
}

extern "C" const uint8_t* image_data(void) {
    return _binary_image_raw_start;
}

void printMat (cv::Mat &A)
{
    for (int r = 0; r < A.rows; ++r) {
        for (int c = 0; c < A.cols; ++c) {
            float v = A.at<float>(r,c);
            //std::cout << v << (c+1<A.cols ? ", " : "");

            printf(" %f", v);
        }
        if (r+1<A.rows) std::cout << std::endl;
    }
    std::cout << "\n\n";
}

extern "C" void opencv_test ()
{
    cv::Mat A = cv::Mat::eye(3, 3, CV_32F);
    printMat(A);

    // Scale it by 3
    cv::Mat B = A * 3.0f;
    printMat(B);


    // Compute determinant of A and B
    float detA = cv::determinant(A);
    float detB = cv::determinant(B);

    printf("det(A) = %f\n", detA);
    printf("det(B) = %f\n", detB);
}

#if 1
extern "C" void edge_detect(const uint8_t *rgb_in,
                 uint8_t *edges_out,
                 int width, int height,
                 double low_thresh = 50.0,
                 double high_thresh = 150.0)
{
    // 1) Wrap input buffer (no copy) as height×width CV_8UC3 Mat
    cv::Mat src(height, width, CV_8UC3, const_cast<uint8_t*>(rgb_in));

    // 2) Convert RGB → Grayscale (CV_8UC1)
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_RGB2GRAY);

    // 3) Run Canny edge detector (produces CV_8UC1 edges)
    cv::Mat edges;
    cv::Canny(gray, edges, low_thresh, high_thresh);

    // 4) Copy result back into your buffer (row‐major, one byte per pixel)
    std::memcpy(edges_out, edges.data, width * height);
}

extern "C" int edge_demo(const char *path,
                 uint8_t *edges_out,
                 int *width, int *height,
                 double low_thresh = 50.0,
                 double high_thresh = 150.0)
{
    cv::Mat src = cv::imread(path, cv::IMREAD_COLOR);
    if (src.empty()) {
        printf("Failed to read %s\n", path);
        return -1;
    }

    cv::Mat gray, edges;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);   // BGR if desktop; RGB if you built differently
    cv::Canny(gray, edges, 50.0, 150.0);

    std::memcpy(edges_out, edges.data, edges.total());
    *width = edges.cols;
    *height = edges.rows;

    return 0;
}

#else

extern "C" void edge_detect(const uint8_t *rgb_in,
                 uint8_t *edges_out,
                 int width, int height,
                 double low_thresh = 50.0,
                 double high_thresh = 150.0) {}

#endif

extern "C" int object_detect(const char* img_path, const char* cascade_path)
{
    // 0) Single-thread to save stack/heap on MCU
    cv::setNumThreads(1);

    // 1) Load directly as grayscale
    cv::Mat gray = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    if (gray.empty()) {
        std::cerr << "Cannot read image: " << img_path << "\n";
        return -1;
    }

    // Optional: light histogram equalization (cheap, but you can skip if CPU-tight)
    cv::equalizeHist(gray, gray);

    // 2) Load cascade
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(cascade_path)) {
        std::cerr << "Cannot load cascade: " << cascade_path << "\n";
        return -3;
    }

    // 3) Since the image is 40x40 and face ≈ 80% (~32x32):
    const cv::Size minSize(20, 20);   // don't waste time on tiny windows
    const cv::Size maxSize(40, 40);   // we know it's not bigger than the frame

    const double scaleFactor  = 1.1;  // fewer pyramid levels than 1.1
    const int    minNeighbors = 3;    // still reasonable quality
    const int    flags = 0;//cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;

    std::vector<cv::Rect> faces;
    faceCascade.detectMultiScale(gray, faces,
                                 scaleFactor, minNeighbors, flags,
                                 minSize, maxSize);

    // If you only need a yes/no, you could return (faces.empty() ? 0 : 1) to avoid vector copies
    std::cout << "Faces detected: " << faces.size() << "\n";
    return static_cast<int>(faces.size());
}

extern "C" int object_detect_2(const char* img_path,
                             const char* cascade_path,
                             uint8_t**   out_buf,     // RGB888 buffer you can feed LVGL
                             int*        out_w,
                             int*        out_h,
                             size_t*     out_data_sz) // bytes in buffer
{
    if (!out_buf || !out_w || !out_h || !out_data_sz) return -99;

    cv::setNumThreads(1);

    // Load color so we can draw; make a gray copy for detection.
    cv::Mat bgr = cv::imread(img_path, cv::IMREAD_COLOR);
    if (bgr.empty()) {
        std::cerr << "Cannot read image: " << img_path << "\n";
        return -1;
    }
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(cascade_path)) {
        std::cerr << "Cannot load cascade: " << cascade_path << "\n";
        return -3;
    }

    const cv::Size minSize(20, 20);
    const cv::Size maxSize(bgr.cols, bgr.rows);
    const double scaleFactor  = 1.3;
    const int    minNeighbors = 3;
    const int    flags        = 0;

    std::vector<cv::Rect> faces;

    uint32_t start_ms = k_uptime_get_32();

    faceCascade.detectMultiScale(gray, faces,
                                 scaleFactor, minNeighbors, flags,
                                 minSize, maxSize);

    uint32_t end_ms = k_uptime_get_32();
    uint32_t elapsed = end_ms - start_ms;

    printf("object_detect_2 took %u ms\n", elapsed);

    // Draw rectangles (green, 2 px)
    for (const auto& r : faces) {
        cv::rectangle(bgr, r, cv::Scalar(0,255,0), 2, cv::LINE_8);
    }

    // Convert BGR -> RGB (LVGL expects RGB order)
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);

    // Allocate a plain buffer for LVGL (RGB888)
    const int w = rgb.cols;
    const int h = rgb.rows;
    const size_t sz = static_cast<size_t>(w) * h * 3;
    uint8_t* buf = static_cast<uint8_t*>(malloc(sz));
    if (!buf) return -5;
    std::memcpy(buf, rgb.data, sz);

    *out_buf     = buf;
    *out_w       = w;
    *out_h       = h;
    *out_data_sz = sz;

    std::cout << "Faces detected: " << faces.size() << "\n";
    return static_cast<int>(faces.size());
}

#endif