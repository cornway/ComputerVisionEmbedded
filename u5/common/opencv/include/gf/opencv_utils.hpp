
#pragma once

#include <vector>

#include "gf/opencv_pipeline.hpp"

namespace gf_cv {

bool loadCascade(cv::CascadeClassifier &cascade, const char *path);
bool loadCascade(cv::CascadeClassifier &cascade, cv::FileStorage &fs);
bool loadCascade(cv::CascadeClassifier &cascade, const uint8_t *buffer,
                 size_t len);

/**
 * @brief
 *
 * @param faceCascade
 * @param smileCascade
 * @param thumbnailFrame frame to detect face in, thumbnail, small enough to
 * save memory
 * @param fullFrame frame to detect face in, roi is cropped from to have finer
 * grained area
 * @param faceROIMax maximum faceROI to be resized to if detected ROI is larger
 * than this
 * @param roIKernelParamsSweep
 * @return vector of cv::Rects for detected objects
 */
std::vector<cv::Rect> detectFaceAndSmile(cv::CascadeClassifier &faceCascade,
                                         cv::CascadeClassifier &smileCascade,
                                         cv::Mat &thumbnailFrame,
                                         cv::Mat &fullFrame,
                                         cv::Rect &faceROIMax, Stage &pipeline);

/**
 * @brief
 *
 * @param faceCascade
 * @param thumbnailFrame
 * @return std::vector<cv::Rect>
 */
std::vector<cv::Rect> detectFace(cv::CascadeClassifier &faceCascade,
                                 cv::Mat &thumbnailFrame);

/**
 * @brief
 *
 * @param bgr
 * @return cv::Mat
 */
cv::Mat cv_preprocessForQR(const cv::Mat &bgr);

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
cv::Rect translateScaleROI(cv::Rect &ROI, float sx, float sy, float tx,
                           float ty);

} /* namespace gf_cv */