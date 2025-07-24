

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>

#include <sys/time.h>

#include "opencv_utils.hpp"

namespace Ocv {

cv::Mat objdetect_prepare(const char *path, cv::ImreadModes mode) {
  cv::Mat mat = cv::imread(path, mode);

  cv::Mat gray;
  if (cv::IMREAD_COLOR == mode) {
    cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = mat;
  }

  cv::equalizeHist(gray, gray);

  return gray;
}

} // namespace Ocv