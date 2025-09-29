#include <opencv2/opencv.hpp>
#include <iostream>

#include "HaarPath.hpp"

#include "opencv_utils.hpp"

cv::String haarPath = HAAR_PATH;
cv::String face_cascade_name = haarPath + "lbpcascades/lbpcascade_frontalface_improved.xml";
cv::String smile_cascade_name = haarPath + "haarcascades/haarcascade_smile.xml";
cv::CascadeClassifier face_cascade;
cv::CascadeClassifier smile_cascade;
cv::String window_name = "Capture - Face detection";

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
	while (camera.read(frameOriginal)) {

		if (frameOriginal.empty())
		{
			printf(" --(!) No captured frame -- Break!");
			break;
		}

		cv::Mat frameOriginalGray;
		cv::cvtColor(frameOriginal, frameOriginalGray, cv::COLOR_BGR2GRAY);
		cv::equalizeHist(frameOriginalGray, frameOriginalGray);

		cv::Rect faceROIMax{};
		faceROIMax.width = 80;
		faceROIMax.height = 80;

		cv::resize(frameOriginalGray, frameScreenGray, cv::Size(faceROIMax.width, faceROIMax.height), 0.0f, 0.0f, cv::INTER_LINEAR);

		std::vector<cv::Rect> ROIs = detectFaceAndSmile(
			face_cascade,
			smile_cascade,
			frameScreenGray,
			frameOriginalGray,
			faceROIMax
		);

		for (const auto &ROI : ROIs) {
			rectangle(frameScreenGray, ROI, cv::Scalar(0, 0, 255), 2, 8, 0);
		}

		cv::imshow(window_name, frameScreenGray);
		int c = cv::waitKey(100);
    	if ((char)c == 27) { return 0; }
    }

    return 0;
}

// CMD to generate executable: 
// g++ webcam_opencv.cpp -o webcam_demo -I/usr/include/opencv4 -lopencv_core -lopencv_videoio -lopencv_highgui

// Note: check your opencv hpp files - for many users it is at /usr/local/include/opencv4
// Add more packages during compilation from the list obtained by $ pkg-config --cflags --libs opencv4
