#include <opencv2/opencv.hpp>
#include <iostream>

#include "HaarPath.hpp"

#include "gf/opencv_utils.hpp"

cv::String haarPath = HAAR_PATH;
cv::String face_cascade_name = haarPath + "lbpcascades/lbpcascade_frontalface_improved.xml";
cv::String smile_cascade_name = haarPath + "haarcascades/haarcascade_smile.xml";
cv::CascadeClassifier face_cascade;
cv::CascadeClassifier smile_cascade;
cv::String window_name = "Capture - Face detection";
cv::String window_name_2 = "Capture - Smile detection";


bool checkFrameSizeSupported(cv::Mat &gray) {
	if (gray.cols == 640 && gray.rows == 480) {
		return true;
	}

	if (gray.cols == 320 && gray.rows == 240) {
		return true;
	}
	return false;
}

int main(int, char**) {
    // open the first webcam plugged in the computer
    cv::VideoCapture camera(0); // in linux check $ ls /dev/video0
    if (!camera.isOpened()) {
        std::cerr << "ERROR: Could not open camera" << std::endl;
        return 1;
    }

    // array to hold image
    cv::Mat frameOriginal;
	cv::Mat frameScreenGray;

    if (!face_cascade.load(face_cascade_name))
	{
		printf("--(!)Error loading face cascade\n"); 
		return -1;
	};
	if (!smile_cascade.load(smile_cascade_name)) 
	{
		printf("--(!)Error loading eyes cascade\n"); 
		return -1;
	};
    
    cv::namedWindow(window_name, 2);
    // display the frame until you press a key

	std::cout << "Start capturing" << std::endl;
	float gamma = 0.1;
	std::map<float, uint32_t> gammaScore;
	while (camera.read(frameOriginal)) {

		if (frameOriginal.empty())
		{
			printf(" --(!) No captured frame -- Break!");
			break;
		}

		cv::Mat frameOriginalGray;
		cv::cvtColor(frameOriginal, frameOriginalGray, cv::COLOR_BGR2GRAY);
		//cv::equalizeHist(frameOriginalGray, frameOriginalGray);

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
		faceROIMax.height = 64;

		cv::resize(frameCroppedGray, frameScreenGray, cv::Size(80, 80), 0.0f, 0.0f, cv::INTER_NEAREST );

		std::vector<cv::Rect> faces, smiles;

		cv::Mat smilePostProc = detectFaceAndSmile(
			face_cascade,
			smile_cascade,
			frameScreenGray,
			frameCroppedGray,
			faceROIMax,
			faces,
			smiles,
			gamma
		);

		if (!faces.empty() && !smiles.empty()) {
			if (gammaScore.count(gamma)) {
				gammaScore[gamma] = gammaScore[gamma] + 1;
			} else {
				gammaScore[gamma] = 1;
			}
		}

		if (!faces.empty() && smiles.empty()) {
			gamma += 0.1;
			if (gamma >= 1.0) {
				gamma = 0.1;
			}
		}

		for (const auto &ROI : faces) {
			rectangle(frameScreenGray, ROI, cv::Scalar(0, 0, 255), 1, 1, 0);
		}
		for (const auto &ROI : smiles) {
			rectangle(frameScreenGray, ROI, cv::Scalar(0, 0, 255), 1, 1, 0);
		}

		if (smilePostProc.cols && smilePostProc.rows) {
			cv::imshow(window_name_2, smilePostProc);
		}
		cv::imshow(window_name, frameScreenGray);

		int c = cv::waitKey(100);
    	if ((char)c == 27) { break; }
    }

	for (auto & [gamma, score] : gammaScore) {
		std::cout << "gamma = " << gamma << ", score = " << score << std::endl;
	}

    return 0;
}

// CMD to generate executable: 
// g++ webcam_opencv.cpp -o webcam_demo -I/usr/include/opencv4 -lopencv_core -lopencv_videoio -lopencv_highgui

// Note: check your opencv hpp files - for many users it is at /usr/local/include/opencv4
// Add more packages during compilation from the list obtained by $ pkg-config --cflags --libs opencv4
