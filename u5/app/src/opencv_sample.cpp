


#if defined(CONFIG_OPENCV_LIB)

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include <iostream>
#include <cstring>

#include <sys/time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

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

#endif