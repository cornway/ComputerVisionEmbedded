
#include "opencv_utils.hpp"

#pragma once

namespace gf_cv {

class Stage {
public:
  explicit Stage() = default;
  virtual ~Stage() = default;

  struct sentinel {};

  struct iterator {
    Stage &stage;

    cv::Mat operator*() { return stage.get(); }
    iterator &operator++() {
      stage.next();
      return *this;
    }
    bool operator!=(sentinel) { return stage.hasNext(); }
  };

  iterator begin() { return {*this}; }
  sentinel end() const { return {}; }

  virtual cv::Mat get() = 0;
  virtual void next() = 0;
  virtual bool hasNext() = 0;
};

class ClaheStage : public Stage {
public:
  explicit ClaheStage(cv::Mat &in, Range<float> &clipLimit,
                      Range<int> &tileSize);
  ~ClaheStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  cv::Mat &in;
  Range<float> &clipLimit;
  Range<int> &tileSize;

  Range<float>::iterator clipLimitIt;
  Range<int>::iterator tileSizeIt;
};

class BilateralStage : public Stage {
public:
  explicit BilateralStage(cv::Mat &in, Range<int> &d, Range<float> &sigmaColor,
                          Range<float> &sigmaSpace);
  ~BilateralStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  cv::Mat &in;
  Range<int> &d;
  Range<float> &sigmaColor;
  Range<float> &sigmaSpace;

  Range<int>::iterator dIt;
  Range<float>::iterator sigmaColorIt;
  Range<float>::iterator sigmaSpaceIt;
};

class GammaStage : public Stage {
public:
  explicit GammaStage(cv::Mat &in, Range<float> &gamma);
  ~GammaStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  cv::Mat &in;
  Range<float> &gamma;

  Range<float>::iterator gammaIt;
};

class BlurStage : public Stage {
public:
  explicit BlurStage(cv::Mat &in, Range<float> &sigmaX);
  ~BlurStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  cv::Mat &in;
  Range<float> &sigmaX;

  Range<float>::iterator sigmaXIt;
};

} // namespace gf_cv