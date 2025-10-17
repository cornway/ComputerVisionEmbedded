

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>

#include "gf/opencv_pipeline.hpp"
#include "gf/opencv_utils.hpp"

namespace gf_cv {

bool loadCascade(cv::CascadeClassifier &cascade, const char *path) {
  if (!cascade.load(path)) {
    std::cerr << "Cannot load cascade: " << path << "\n";
    return false;
  }
  return true;
}

bool loadCascade(cv::CascadeClassifier &cascade, cv::FileStorage &fs) {
  if (!cascade.read(fs.getFirstTopLevelNode())) {
    std::cerr << "Cannot load cascade\n";
    return false;
  }
  return true;
}

bool loadCascade(cv::CascadeClassifier &cascade, const uint8_t *buffer,
                 size_t len) {
  cv::FileStorage fs(std::string((const char *)buffer, len),
                     cv::FileStorage::READ | cv::FileStorage::MEMORY);
  return loadCascade(cascade, fs);
}

void detectObjects(cv::CascadeClassifier &cascade, cv::Mat &gray,
                   std::vector<cv::Rect> &objects) {
  const double scaleFactor = 1.1;
  const int minNeighbors = 3;
  const int flags = 0;

  cascade.detectMultiScale(gray, objects, scaleFactor, minNeighbors, flags);
}

void detectFaces(cv::CascadeClassifier &cascade, cv::Mat &gray,
                 std::vector<cv::Rect> &faces) {
  detectObjects(cascade, gray, faces);
}

cv::Mat cv_preprocessForQR(const cv::Mat &bgr) {
  cv::Mat gray, up, eq, bin;
  if (bgr.channels() == 3)
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
  else
    gray = bgr;

  cv::equalizeHist(gray, eq);
  cv::adaptiveThreshold(eq, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                        cv::THRESH_BINARY, 31, 5);
  return bin;
}

static bool detectSmile(cv::CascadeClassifier &cascade, cv::Mat &in,
                        std::vector<cv::Rect> &rects) {

  const double scaleFactor = 1.1;
  const int minNeighbors = 3;
  const int flags =
      cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;

  cascade.detectMultiScale(in, rects, scaleFactor, minNeighbors, flags);
  return !rects.empty();
}

std::vector<cv::Rect>
detectFaceAndSmile(cv::CascadeClassifier &faceCascade,
                   cv::CascadeClassifier &smileCascade, cv::Mat &thumbnailFrame,
                   cv::Mat &fullFrame, cv::Rect &faceROIMax, Stage &pipeline) {

  std::vector<cv::Rect> rects;

  const double scaleFactor = 1.04;
  const int minNeighbors = 3;
  const int flags =
      cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;

  // cv::equalizeHist(thumbnailFrame, thumbnailFrame);
  std::vector<cv::Rect> faceRects;
  faceCascade.detectMultiScale(thumbnailFrame, faceRects, scaleFactor,
                               minNeighbors, flags);

  for (auto &faceRect : faceRects) {

    rects.push_back(faceRect);

    try {

      cv::Rect faceRectFull = remapROI(faceRect, thumbnailFrame, fullFrame);

      /* Get lower half/third of face ROI + some gap where the smile is most
       * likelly to be */
      const uint32_t offset_y = (faceRectFull.height * 2) / 3;

      faceRectFull.y = faceRectFull.y + offset_y - 10;
      faceRectFull.height = faceRectFull.height - faceRectFull.height / 2;
      if (faceRectFull.height + faceRectFull.y > fullFrame.rows) {
        faceRectFull.height = fullFrame.rows - faceRectFull.y;
      }

      cv::Rect roiThumb = remapROI(faceRectFull, fullFrame, thumbnailFrame);

      float sx = 1.0f;
      float sy = 1.0f;

      cv::Mat faceROIFrame = fullFrame(faceRectFull);
      cv::Mat faceROIFrameResized;

      // Get area with minimum number of pixels
      if (faceROIMax.width * faceROIMax.height <
          faceRectFull.width * faceRectFull.height) {
        sx = (float)faceROIMax.width / (float)faceRectFull.width;
        sy = (float)faceROIMax.height / (float)faceRectFull.height;
        cv::resize(faceROIFrame, faceROIFrameResized, cv::Size(), sx, sy,
                   cv::INTER_LINEAR);
      } else {
        faceROIFrameResized = faceROIFrame;
      }

      std::vector<cv::Rect> smileRects;

      detectSmile(smileCascade, faceROIFrameResized, smileRects);

      // pipeline.invoke(faceROIFrameResized, [&](cv::Mat &in) -> bool {
      //   return detectSmile(smileCascade, in, smileRects);
      // });

      // detectSmile(smileCascade, faceROIFrameResized, smileRects,
      //             roIKernelParamsSweep);
      //  detectSmile(smileCascade, faceROIFrameResized, smileRects);

      float ssx_inv = (float)thumbnailFrame.cols / (float)fullFrame.cols;
      float ssy_inv = (float)thumbnailFrame.rows / (float)fullFrame.rows;

      for (auto &smileRect : smileRects) {
        cv::Rect smileROIRemapped = translateScaleROI(
            smileRect, 1.0f / sx, 1.0f / sy, faceRectFull.x, faceRectFull.y);
        smileROIRemapped =
            translateScaleROI(smileROIRemapped, ssx_inv, ssy_inv, 0.0f, 0.0f);
        rects.push_back(smileROIRemapped);
      }

    } catch (cv::Exception &ce) {
      std::cerr << "Exception: " << ce.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown Exception";
    }
  }

  return rects;
}

cv::Rect remapROI(cv::Rect &ROI, cv::Mat &inFrame, cv::Mat &outFrame) {
  float sx = (float)outFrame.cols / (float)inFrame.cols;
  float sy = (float)outFrame.rows / (float)inFrame.rows;
  return translateScaleROI(ROI, sx, sy, 0.0f, 0.0f);
}

cv::Rect translateScaleROI(cv::Rect &ROI, float sx, float sy, float tx,
                           float ty) {
  cv::Rect OutROI;

  float inROIVecData[6] = {
      (float)ROI.x, (float)(ROI.x + ROI.width),
      (float)ROI.y, (float)(ROI.y + ROI.height),
      1.0f,         1.0f,
  };

  float scaleVecData[9] = {sx, 0.0f, tx, 0.0f, sy, ty, 0.0f, 0.0f, 1.0f};

  cv::Mat scaleMat(3, 3, CV_32FC1, &scaleVecData[0]);
  cv::Mat inROIMat(3, 2, CV_32FC1, &inROIVecData[0]);

  float outROIVecData[6]{};
  cv::Mat outROIMat(3, 2, CV_32FC1, &outROIVecData[0]);

  cv::gemm(scaleMat, inROIMat, 1.0f, cv::Mat(), 1.0f, outROIMat);

  OutROI.x = outROIVecData[0];
  OutROI.y = outROIVecData[2];
  OutROI.width = outROIVecData[1] - outROIVecData[0];
  OutROI.height = outROIVecData[3] - outROIVecData[2];

  return OutROI;
}

} /* namespace gf_cv */