
#include "gf/opencv_pipeline.hpp"

namespace gf_cv {

Stage &Stage::setNextStage(Stage *_nextStage) {
  nextStage = _nextStage;
  return *this;
}

bool Stage::invoke(cv::Mat &_in, InvokeCallback callback) {
  if (!nextStage) {
    return callback(_in);
  }
  setInput(_in);
  for (auto mat : *this) {
    if (nextStage->invoke(mat, callback)) {
      return true;
    }
  }
  return false;
}

void Stage::setInput(cv::Mat &_in) { in = _in; }

ClaheStage::ClaheStage(Range<float> &clipLimit, Range<int> &tileSize)
    : clipLimit(clipLimit), tileSize(tileSize) {

  clipLimitIt = clipLimit.begin();
  tileSizeIt = tileSize.begin();
}

cv::Mat ClaheStage::get() {
  cv::Ptr<cv::CLAHE> clahe =
      cv::createCLAHE(*clipLimitIt, cv::Size(*tileSizeIt, *tileSizeIt));
  cv::Mat out;
  clahe->apply(in, out);
  return out;
}

void ClaheStage::next() {
  clipLimitIt = ++clipLimitIt;
  if (clipLimitIt == clipLimit.end()) {
    clipLimitIt = clipLimit.begin();
    tileSizeIt = ++tileSizeIt;
  }
}

bool ClaheStage::hasNext() {
  return (clipLimitIt != clipLimit.end()) && (tileSizeIt != tileSize.end());
}

BilateralStage::BilateralStage(Range<int> &d, Range<float> &sigmaColor,
                               Range<float> &sigmaSpace)
    : d(d), sigmaColor(sigmaColor), sigmaSpace(sigmaSpace) {

  dIt = d.begin();
  sigmaColorIt = sigmaColor.begin();
  sigmaSpaceIt = sigmaSpace.begin();
}

cv::Mat BilateralStage::get() {
  cv::Mat out;
  cv::bilateralFilter(in, out, *dIt, *sigmaColorIt, *sigmaSpaceIt);
  return out;
}

void BilateralStage::next() {
  dIt = ++dIt;
  if (dIt == d.end()) {
    dIt = d.begin();
    sigmaColorIt = ++sigmaColorIt;
  }
  if (sigmaColorIt == sigmaColor.end()) {
    sigmaColorIt = sigmaColor.begin();
    sigmaSpaceIt = ++sigmaSpaceIt;
  }
}

bool BilateralStage::hasNext() {
  return (dIt != d.end()) && (sigmaColorIt != sigmaColor.end()) &&
         (sigmaSpaceIt != sigmaSpace.end());
}

GammaStage::GammaStage(Range<float> &gamma) : gamma(gamma) {
  gammaIt = gamma.begin();
}

cv::Mat GammaStage::get() {
  cv::Mat out = in.clone();
  out.convertTo(out, CV_32FC1, 1.0 / 255.0);
  cv::pow(out, *gammaIt, out);
  out.convertTo(out, CV_8UC1, 255.0);
  return out;
}

void GammaStage::next() { gammaIt = ++gammaIt; }

bool GammaStage::hasNext() { return (gammaIt != gamma.end()); }

BlurStage::BlurStage(Range<float> &sigmaX) : sigmaX(sigmaX) {
  sigmaXIt = sigmaX.begin();
}

cv::Mat BlurStage::get() {
  cv::Mat blurFrame;
  cv::Mat out = in.clone();
  cv::GaussianBlur(out, blurFrame, cv::Size(0, 0), *sigmaXIt);
  // cv::Laplacian(bfROI, blurImg, -1, 1, 1, 0, cv::BORDER_DEFAULT);
  cv::addWeighted(out, 1.5, blurFrame, -0.5, 0, out);
  return out;
}

void BlurStage::next() { sigmaXIt = ++sigmaXIt; }

bool BlurStage::hasNext() { return (sigmaXIt != sigmaX.end()); }

} // namespace gf_cv