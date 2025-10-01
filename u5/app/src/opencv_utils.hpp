
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
 * @param thumbnailFrame frame to detect face in, thumbnail, small enough to save memory
 * @param fullFrame frame to detect face in, roi is cropped from to have finer frained area
 * @param faceROIMax maximum faceROI to be resized to if detected ROI is largen than this
 * @param faces vector with faces rects
 * @param smiles vector with sniles rects
 * @return std::vector<cv::Rect> all ROI's ever considered during run
 */
std::vector<cv::Rect> detectFaceAndSmile(
    cv::CascadeClassifier &faceCascade,
    cv::CascadeClassifier &smileCascade,
    cv::Mat &thumbnailFrame,
    cv::Mat &fullFrame,
    cv::Rect &faceROIMax,
    std::vector<cv::Rect> &faces,
    std::vector<cv::Rect> &smiles
);

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

