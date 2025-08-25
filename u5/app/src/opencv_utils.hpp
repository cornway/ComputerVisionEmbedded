
#pragma once

#include "opencv2/opencv.hpp"

#define OCV_SC_FACE_DETECT      0
#define OCV_SC_QRCODE           1
#if defined(CONFIG_QUIRC_LIB)
#define OCV_SC_QRCODE_QUIRC     2
#endif

bool loadCascade(cv::CascadeClassifier &cascade, const char *path);

bool detectFaces(cv::CascadeClassifier &cascade, cv::Mat &gray, std::vector<cv::Rect> &faces);

cv::Mat cv_preprocessForQR(const cv::Mat& bgr);
