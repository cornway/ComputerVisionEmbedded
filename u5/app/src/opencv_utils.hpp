
#pragma once

#include "opencv2/imgcodecs.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

namespace Ocv {

cv::Mat objdetect_prepare(const char *path, cv::ImreadModes mode);
}