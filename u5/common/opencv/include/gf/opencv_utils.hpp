
#pragma once

#include <vector>

#include "opencv2/opencv.hpp"

namespace gf_cv {

bool loadCascade(cv::CascadeClassifier &cascade, const char *path);
bool loadCascade(cv::CascadeClassifier &cascade, cv::FileStorage &fs);
bool loadCascade(cv::CascadeClassifier &cascade, const uint8_t *buffer,
                 size_t len);

template <class T> struct Range {
  static_assert(std::is_arithmetic<T>::value,
                "struct Range: T must be arithmetic");

  T start, stop, step;

  struct sentinel {};

  struct iterator {
    T value, stop, step;

    T operator*() const { return value; };
    iterator &operator++() {
      value += step;
      return *this;
    }
    bool operator!=(sentinel) { return value <= stop; }
    bool operator==(sentinel) { return value > stop; }
  };

  iterator begin() const {
    assert(step > T(0) && "Range: step must be > 0");
    return {start, stop, step};
  }

  sentinel end() const { return {}; }
};

struct ROIKernelParamsSweep {
  struct Clahe {
    Range<float> clipLimit;
    Range<int> tileSize;
  } clahe;
  struct BiFilter {
    Range<int> d;
    Range<float> sigmaColor;
    Range<float> sigmaSpace;
  } biFilter;
  struct Gamma {
    Range<float> gamma;
  } gamma;
  struct Blur {
    Range<float> sigmaX;
  } blur;
};

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
std::vector<cv::Rect>
detectFaceAndSmile(cv::CascadeClassifier &faceCascade,
                   cv::CascadeClassifier &smileCascade, cv::Mat &thumbnailFrame,
                   cv::Mat &fullFrame, cv::Rect &faceROIMax,
                   ROIKernelParamsSweep &roIKernelParamsSweep);

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