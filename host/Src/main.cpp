#include <iostream>
#include <opencv2/opencv.hpp>

#include "HaarPath.hpp"

#include "gf/opencv_utils.hpp"

cv::String haarPath = HAAR_PATH;
cv::String face_cascade_name =
    haarPath + "lbpcascades/lbpcascade_frontalface_improved.xml";
cv::String smile_cascade_name = haarPath + "haarcascades/haarcascade_smile.xml";
cv::CascadeClassifier face_cascade;
cv::CascadeClassifier smile_cascade;
cv::String window_name = "Capture - Face detection";
cv::String window_name_smile = "Smile";

using namespace gf_cv;

bool checkFrameSizeSupported(cv::Mat &gray) {
  if (gray.cols == 640 && gray.rows == 480) {
    return true;
  }

  if (gray.cols == 320 && gray.rows == 240) {
    return true;
  }
  return false;
}

int main(int, char **) {
  // open the first webcam plugged in the computer
  cv::VideoCapture camera(0); // in linux check $ ls /dev/video0
  if (!camera.isOpened()) {
    std::cerr << "ERROR: Could not open camera" << std::endl;
    return 1;
  }

  // array to hold image
  cv::Mat frameOriginal;
  cv::Mat frameScreenGray;

  if (!face_cascade.load(face_cascade_name)) {
    printf("--(!)Error loading face cascade\n");
    return -1;
  };
  if (!smile_cascade.load(smile_cascade_name)) {
    printf("--(!)Error loading eyes cascade\n");
    return -1;
  };

  cv::namedWindow(window_name, cv::WND_PROP_ASPECT_RATIO);
  // display the frame until you press a key

  std::cout << "Start capturing" << std::endl;
  std::map<std::string, uint32_t> smileROIKernelParamsScore;
  while (camera.read(frameOriginal)) {

    if (frameOriginal.empty()) {
      printf(" --(!) No captured frame -- Break!");
      break;
    }

    cv::Mat frameOriginalGray;
    cv::cvtColor(frameOriginal, frameOriginalGray, cv::COLOR_BGR2GRAY);
    // cv::equalizeHist(frameOriginalGray, frameOriginalGray);

    if (false == checkFrameSizeSupported(frameOriginalGray)) {
      std::cerr << "Not supported frame size" << std::endl;
      exit(-1);
    }

    cv::Mat frameCroppedGray;

    if (frameOriginalGray.cols == 640 && frameOriginalGray.rows == 480) {
      cv::Rect crop;
      crop.width = 320;
      crop.height = 240;
      crop.x = (frameOriginalGray.cols - crop.width) / 2;
      crop.y = (frameOriginalGray.rows - crop.height) / 2;
      frameCroppedGray = frameOriginalGray(crop);
    } else {
      frameCroppedGray = frameOriginalGray;
    }

    cv::Rect faceROIMax{};
    faceROIMax.width = 64;
    faceROIMax.height = 40;

    cv::resize(frameCroppedGray, frameScreenGray, cv::Size(80, 80), 0.0f, 0.0f,
               cv::INTER_NEAREST);

    std::vector<cv::Rect> faces, smiles;

    auto sigmaX = Range<float>{0.82, 0.87, 0.01};
    BlurStage blurStage = BlurStage(sigmaX);

    auto gamma = Range<float>{0.44, 0.50, 0.02};
    GammaStage gammaStage = GammaStage(gamma);

    auto d = Range<int>{3, 3, 1};
    auto sigmaColor = Range<float>{10.0, 10.0, 1.0};
    auto sigmaSpace = Range<float>{50.0, 50.0, 1.0};
    BilateralStage bilateralStage = BilateralStage(d, sigmaColor, sigmaSpace);

    auto clipLimit = Range<float>{4.0, 4.0, 1.0};
    auto tileSize = Range<int>{4, 4, 1};
    ClaheStage claheStage = ClaheStage(clipLimit, tileSize);

    claheStage.setNextStage(&bilateralStage);
    bilateralStage.setNextStage(&gammaStage);
    gammaStage.setNextStage(nullptr);
    // blurStage.setNextStage(nullptr);

    auto rects =
        detectFaceAndSmile(face_cascade, smile_cascade, frameScreenGray,
                           frameCroppedGray, faceROIMax, claheStage);

    for (const auto &rect : rects) {
      rectangle(frameScreenGray, rect, cv::Scalar(0, 0, 255), 1, 1, 0);
    }

    cv::imshow(window_name, frameScreenGray);

    int c = cv::waitKey(1);
    if ((char)c == 27) {
      break;
    }

    continue;

#if 0
		if (faces.empty()) {
			continue;
		}

		const double scaleFactor = 1.1;
		const int minNeighbors = 3;
		const int flags = cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;

		std::vector<float> claheClipLimit_V({4.0});
		std::vector<int> claheTileSize_V({4});
		std::vector<int> biFilterD_V({3});
		std::vector<int> biFilterSigmaColor_V({20, 22, 24, 26, 28, 20});
		std::vector<int> biFilterSigmaSpace_V({50});
		std::vector<float> gammaGamma_V({0.45, 0.47, 0.49, 0.50, 0.52, 0.54, 0.56, 0.58, 0.60, 0.62, 0.65});
		std::vector<float> blurSigmaX_V({0.85, 0.86, 0.87, 0.88, 0.89, 0.90, 0.91, 0.92, 0.93, 0.94, 0.95});

		for (auto claheClipLimit : claheClipLimit_V) {
			for (auto claheTileSize : claheTileSize_V) {
				for (auto biFilterD : biFilterD_V) {
					for (auto biFilterSigmaColor : biFilterSigmaColor_V) {
						for (auto biFilterSigmaSpace : biFilterSigmaSpace_V) {
							for (auto gammaGamma : gammaGamma_V) {
								for (auto blurSigmaX : blurSigmaX_V) {
									smileROIKernelParams.clahe.clipLimit = claheClipLimit;
									smileROIKernelParams.clahe.tileSize = claheTileSize;
									smileROIKernelParams.biFilter.d = biFilterD;
									smileROIKernelParams.biFilter.sigmaColor = biFilterSigmaColor;
									smileROIKernelParams.biFilter.sigmaSpace = biFilterSigmaSpace;
									smileROIKernelParams.gamma.gamma = gammaGamma;
									smileROIKernelParams.blur.sigmaX = blurSigmaX;

									smileROIMat = preprocessSmileROI(smileROIMat, smileROIKernelParams);
									std::vector<cv::Rect> smiles;
									smile_cascade.detectMultiScale(smileROIMat, smiles, scaleFactor, minNeighbors,
																flags);

									if (!smiles.empty()) {
										std::string key;
										key += "clahe.clipLimit=" + std::to_string(smileROIKernelParams.clahe.clipLimit) + ":";
										key += "clahe.tileSize=" + std::to_string(smileROIKernelParams.clahe.tileSize) + ":";
										key += "biFilter.d=" + std::to_string(smileROIKernelParams.biFilter.d) + ":";
										key += "biFilter.sigmaColor=" + std::to_string(smileROIKernelParams.biFilter.sigmaColor) + ":";
										key += "biFilter.sigmaSpace=" + std::to_string(smileROIKernelParams.biFilter.sigmaSpace) + ":";
										key += "gamma.gamma=" + std::to_string(smileROIKernelParams.gamma.gamma) + ":";
										key += "blur.sigmaX=" + std::to_string(smileROIKernelParams.blur.sigmaX);

										auto it = smileROIKernelParamsScore.try_emplace(key, 1);
										if (!it.second) {
											auto &node = *it.first;
											node.second += 1;
										}

										//cv::imshow(window_name_2, smileROIMat);
										//cv::waitKey(100);
										break;
									}

								}
							}
						}
					}
				}
			}
		}
#endif
  }

  for (auto &[key, score] : smileROIKernelParamsScore) {
    std::cout << "key = " << key << ", score = " << score << std::endl;
  }

  return 0;
}

// CMD to generate executable:
// g++ webcam_opencv.cpp -o webcam_demo -I/usr/include/opencv4 -lopencv_core
// -lopencv_videoio -lopencv_highgui

// Note: check your opencv hpp files - for many users it is at
// /usr/local/include/opencv4 Add more packages during compilation from the list
// obtained by $ pkg-config --cflags --libs opencv4
