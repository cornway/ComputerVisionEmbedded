
#include <functional>

#include "opencv2/opencv.hpp"

#pragma once

namespace gf_cv {

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

class Stage {
public:
  using InvokeCallback = std::function<bool(cv::Mat &)>;
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

  Stage &setNextStage(Stage *nextStage);
  bool invoke(cv::Mat &in, InvokeCallback callback);
  void setInput(cv::Mat &in);

protected:
  Stage *nextStage{nullptr};
  cv::Mat in;
};

class DummyStage : public Stage {
public:
  explicit DummyStage() = default;
  ~DummyStage() = default;
  cv::Mat get() override { return cv::Mat(); }
  void next() override {}
  bool hasNext() override { return false; }
};

/* CLAHE */
class ClaheStage : public Stage {
public:
  explicit ClaheStage(Range<float> &clipLimit, Range<int> &tileSize);

  ~ClaheStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  Range<float> &clipLimit;
  Range<int> &tileSize;

  Range<float>::iterator clipLimitIt;
  Range<int>::iterator tileSizeIt;
};

class BilateralStage : public Stage {
public:
  explicit BilateralStage(Range<int> &d, Range<float> &sigmaColor,
                          Range<float> &sigmaSpace);
  ~BilateralStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  Range<int> &d;
  Range<float> &sigmaColor;
  Range<float> &sigmaSpace;

  Range<int>::iterator dIt;
  Range<float>::iterator sigmaColorIt;
  Range<float>::iterator sigmaSpaceIt;
};

class GammaStage : public Stage {
public:
  explicit GammaStage(Range<float> &gamma);
  ~GammaStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  Range<float> &gamma;

  Range<float>::iterator gammaIt;
};

class BlurStage : public Stage {
public:
  explicit BlurStage(Range<float> &sigmaX);
  ~BlurStage() = default;

  cv::Mat get() override;
  void next() override;
  bool hasNext() override;

private:
  Range<float> &sigmaX;

  Range<float>::iterator sigmaXIt;
};

} // namespace gf_cv