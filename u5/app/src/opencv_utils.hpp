
#pragma once

#include "opencv2/opencv.hpp"

#define OCV_SC_FACE_DETECT      0
#define OCV_SC_QRCODE           1
#if defined(CONFIG_QUIRC_LIB)
#define OCV_SC_QRCODE_QUIRC     2
#endif

bool loadCascade(cv::CascadeClassifier &cascade, const char *path);
bool loadCascade(cv::CascadeClassifier &cascade, cv::FileStorage &fs);
bool loadCascade(cv::CascadeClassifier &cascade, const uint8_t *buffer, size_t len);
/**
 * @brief 
 * 
 * @param faceCascade
 * @param smileCascade
 * @param grayScreen this is the frame to be displayed on the screen, usually smaller that original one
 * @param grayFull this is the original frame, to crop from to detect smile
 * @param faceROIMax maximum faceROI to be resized to if detected ROI is largen than this
 * @return std::vector<cv::Rect> bunch of ROI's to be drawn on the screen (grayScreen)
 */
std::vector<cv::Rect> detectFaceAndSmile(
    cv::CascadeClassifier &faceCascade,
    cv::CascadeClassifier &smileCascade,
    cv::Mat &grayScreen,
    cv::Mat &grayFull,
    cv::Rect &faceROIMax);

cv::Mat cv_preprocessForQR(const cv::Mat& bgr);

/**
 * @brief Remaps ROI that was defined withing inFrame to the outFrame
 * 
 * @param ROI original ROI rect
 * @param inFrame input frame
 * @param outFrame output frame
 * @return cv::Rect remapped ROI rect
 */
cv::Rect remapROI(cv::Rect &ROI, cv::Mat &inFrame, cv::Mat &outFrame);

/**
 * @brief Translate and scale ROI using homogenous transformations
 * 
 * @param ROI 
 * @param sx 
 * @param sy 
 * @param tx 
 * @param ty 
 * @return cv::Rect 
 */
cv::Rect translateScaleROI(cv::Rect &ROI, float sx, float sy, float tx, float ty);

