

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>

#include <sys/time.h>

#include "opencv_utils.hpp"

bool loadCascade(cv::CascadeClassifier &cascade, const char *path) {
    uint32_t start_ms = k_uptime_get_32();

    if (!cascade.load(path)) {
        std::cerr << "Cannot load cascade: " << path << "\n";
        return false;
    }

    uint32_t end_ms = k_uptime_get_32();
    uint32_t elapsed = end_ms - start_ms;

    std::cout << "cascade.load() Took : " << elapsed << " ms"  << std::endl;
    return true;
}

bool detectFaces(cv::CascadeClassifier &cascade, cv::Mat &gray, std::vector<cv::Rect> &faces) {
    const cv::Size minSize(24, 24);
    const cv::Size maxSize;
    const double scaleFactor = 1.04;
    const int minNeighbors = 1;
    const int flags = 0;

    uint32_t start_ms = k_uptime_get_32();

    cascade.detectMultiScale(gray, faces, scaleFactor, minNeighbors,
                                flags, minSize, maxSize);

    uint32_t end_ms = k_uptime_get_32();
    uint32_t elapsed = end_ms - start_ms;

    bool facesFound = ! faces.empty();

    if (facesFound) {
      printf("detectMultiScale : %d ms\n", elapsed);
    }

    return facesFound;
}

cv::Mat cv_preprocessForQR(const cv::Mat& bgr) {
    cv::Mat gray, up, eq, bin;
    if (bgr.channels() == 3) cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    else gray = bgr;

    cv::equalizeHist(gray, eq);
    cv::adaptiveThreshold(eq, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 5);
    return bin;
}