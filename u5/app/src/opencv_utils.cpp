

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>

#include "opencv_utils.hpp"

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

bool loadCascade(cv::CascadeClassifier &cascade, const uint8_t *buffer, size_t len) {
    cv::FileStorage fs(std::string((const char *)buffer, len), cv::FileStorage::READ | cv::FileStorage::MEMORY);
    return loadCascade(cascade, fs);
}

void detectObjects(cv::CascadeClassifier &cascade, cv::Mat &gray, std::vector<cv::Rect> &objects) {
    const double scaleFactor = 1.1;
    const int minNeighbors = 3;
    const int flags = 0;

    cascade.detectMultiScale(gray, objects, scaleFactor, minNeighbors,
                                flags);
}

void detectFaces(cv::CascadeClassifier &cascade, cv::Mat &gray, std::vector<cv::Rect> &faces) {
    detectObjects(cascade, gray, faces);
}

cv::Mat cv_preprocessForQR(const cv::Mat& bgr) {
    cv::Mat gray, up, eq, bin;
    if (bgr.channels() == 3) cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    else gray = bgr;

    cv::equalizeHist(gray, eq);
    cv::adaptiveThreshold(eq, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 5);
    return bin;
}

std::vector<cv::Rect> detectFaceAndSmile(
    cv::CascadeClassifier &faceCascade,
    cv::CascadeClassifier &smileCascade,
    cv::Mat &grayScreen,
    cv::Mat &grayFull,
    cv::Rect &faceROIMax) {

    std::vector<cv::Rect> outROIs;
    std::vector<cv::Rect> faceROIs;
    std::vector<cv::Rect> smileROIs;

    const double scaleFactor = 1.05;
    const int minNeighbors = 3;
    const int flags = 0;

    faceCascade.detectMultiScale(grayScreen, faceROIs, scaleFactor, minNeighbors,
                                flags);

    for (auto & faceROI : faceROIs) {
        outROIs.push_back(faceROI);

        try {

            cv::Rect faceROI_Orig = remapROI(faceROI, grayScreen, grayFull);

            /* Get lower half/third of face ROI + some gap where the smile is most likelly to be */
            const uint32_t offset = (faceROI_Orig.height * 2) / 3;
            faceROI_Orig.y = faceROI_Orig.y + offset;
            //faceROI_Orig.height = faceROI_Orig.height - faceROI_Orig.height / 3;
            if (faceROI_Orig.height + faceROI_Orig.y > grayFull.rows) {
                faceROI_Orig.height = grayFull.rows - faceROI_Orig.y;
            }

            float sx = 1.0f;
            float sy = 1.0f;

            cv::Mat faceROIFrame = grayFull(faceROI_Orig);
            cv::Mat faceROIFrameResized;

            // Get area with minimum number of pixels
            if (faceROIMax.width * faceROIMax.height < faceROI_Orig.width * faceROI_Orig.height) {
                sx = (float)faceROIMax.width / (float)faceROI_Orig.width;
                sy = (float)faceROIMax.height / (float)faceROI_Orig.height;
            }
            cv::resize(faceROIFrame, faceROIFrameResized, cv::Size(), sx, sy, cv::INTER_LINEAR);

            const double scaleFactor = 1.05;
            const int minNeighbors = 3;
            const int flags = 0;

            smileCascade.detectMultiScale(faceROIFrameResized, smileROIs, scaleFactor, minNeighbors,
                                        flags);

            for (auto & smileROI : smileROIs) {
                cv::Rect smileROIRemapped = translateScaleROI(smileROI, 1.0f / sx, 1.0f / sy, faceROI_Orig.x, faceROI_Orig.y);

                float ssx_inv = (float)grayScreen.cols / (float)grayFull.cols;
                float ssy_inv = (float)grayScreen.rows / (float)grayFull.rows;

                smileROIRemapped = translateScaleROI(smileROIRemapped, ssx_inv, ssy_inv, 0.0f, 0.0f);

                outROIs.push_back(smileROIRemapped);
            }

        } catch (cv::Exception &ce) {
            std::cerr << "Exception: " << ce.what() << std::endl;
        } catch ( ... ) {
            std::cerr << "Unknown Exception";
        }
    }

    return outROIs;
}

cv::Rect remapROI(cv::Rect &ROI, cv::Mat &inFrame, cv::Mat &outFrame) {
	float sx = (float)outFrame.cols / (float)inFrame.cols;
	float sy = (float)outFrame.rows / (float)inFrame.rows;
	return translateScaleROI(ROI, sx, sy, 0.0f, 0.0f);
}

cv::Rect translateScaleROI(cv::Rect &ROI, float sx, float sy, float tx, float ty) {
	cv::Rect OutROI;

	float inROIVecData[6] = {
		(float)ROI.x, (float)(ROI.x + ROI.width),
		(float)ROI.y, (float)(ROI.y + ROI.height),
		1.0f, 1.0f, 
	};

	float scaleVecData[9] = {
		sx, 0.0f, tx,
		0.0f, sy, ty,
		0.0f, 0.0f, 1.0f
	};

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
