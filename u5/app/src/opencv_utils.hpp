
#pragma once

#include "opencv2/opencv.hpp"

namespace Ocv {

cv::Mat objdetect_prepare(const char *path, cv::ImreadModes mode);
}