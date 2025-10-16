

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>

#include "gf/opencv_utils.hpp"

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

static cv::Mat preprocessSmileROI(const cv::Mat& grayROI, float gamma) {
    // 1) CLAHE for local contrast (great in dim light)
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(5.0, cv::Size(4,4));
    cv::Mat claheROI, bfROI;
    clahe->apply(grayROI, claheROI);

    // 2) Denoise but keep edges
    cv::bilateralFilter(claheROI, bfROI, 3, 20, 50);

    // 3) Gamma lift (power-law)
    bfROI.convertTo(bfROI, CV_32FC1, 1.0/255.0);
    cv::pow(bfROI, gamma, bfROI);
    bfROI.convertTo(bfROI, CV_8UC1, 255.0);

    // 4) Light unsharp mask to emphasize edges
    cv::Mat blurImg;
    cv::GaussianBlur(bfROI, blurImg, cv::Size(0,0), 1.0);
    cv::addWeighted(bfROI, 1.5, blurImg, -0.5, 0, bfROI);

    return bfROI;
}

cv::Mat detectFaceAndSmile(
    cv::CascadeClassifier &faceCascade,
    cv::CascadeClassifier &smileCascade,
    cv::Mat &thumbnailFrame,
    cv::Mat &fullFrame,
    cv::Rect &faceROIMax,
    std::vector<cv::Rect> &faceROIs,
    std::vector<cv::Rect> &smileROIs,
    float mouthGamma) {

    const double scaleFactor = 1.04;
    const int minNeighbors = 3;
    const int flags = cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;
    cv::Mat smilePostProc;

    //cv::equalizeHist(thumbnailFrame, thumbnailFrame);
    faceCascade.detectMultiScale(thumbnailFrame, faceROIs, scaleFactor, minNeighbors,
                                flags);

    for (auto & faceROI : faceROIs) {

        try {

            cv::Rect faceROIFull = remapROI(faceROI, thumbnailFrame, fullFrame);

            /* Get lower half/third of face ROI + some gap where the smile is most likelly to be */
            const uint32_t offset_y = (faceROIFull.height * 2) / 3;

            faceROIFull.y = faceROIFull.y + offset_y - 10;
            faceROIFull.height = faceROIFull.height - faceROIFull.height / 2;
            if (faceROIFull.height + faceROIFull.y > fullFrame.rows) {
                faceROIFull.height = fullFrame.rows - faceROIFull.y;
            }

            cv::Rect roiThumb = remapROI(faceROIFull, fullFrame, thumbnailFrame);

            float sx = 1.0f;
            float sy = 1.0f;

            cv::Mat faceROIFrame = fullFrame(faceROIFull);
            cv::Mat faceROIFrameResized;

            // Get area with minimum number of pixels
            if (faceROIMax.width * faceROIMax.height < faceROIFull.width * faceROIFull.height) {
                sx = (float)faceROIMax.width / (float)faceROIFull.width;
                sy = (float)faceROIMax.height / (float)faceROIFull.height;
                cv::resize(faceROIFrame, faceROIFrameResized, cv::Size(), sx, sy, cv::INTER_LINEAR);
            } else {
                faceROIFrameResized = faceROIFrame;
            }

            const double scaleFactor = 1.1;
            const int minNeighbors = 3;
            const int flags = cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;

            //cv::equalizeHist(faceROIFrameResized, faceROIFrameResized);
            faceROIFrameResized = preprocessSmileROI(faceROIFrameResized, mouthGamma);

            smilePostProc = faceROIFrameResized.clone();
            smileCascade.detectMultiScale(faceROIFrameResized, smileROIs, scaleFactor, minNeighbors,
                                        flags);

            float ssx_inv = (float)thumbnailFrame.cols / (float)fullFrame.cols;
            float ssy_inv = (float)thumbnailFrame.rows / (float)fullFrame.rows;

            for (auto & smileROI : smileROIs) {
                cv::Rect smileROIRemapped = translateScaleROI(smileROI, 1.0f / sx, 1.0f / sy, faceROIFull.x, faceROIFull.y);
                smileROIRemapped = translateScaleROI(smileROIRemapped, ssx_inv, ssy_inv, 0.0f, 0.0f);

                smileROI = smileROIRemapped;
            }

        } catch (cv::Exception &ce) {
            std::cerr << "Exception: " << ce.what() << std::endl;
        } catch ( ... ) {
            std::cerr << "Unknown Exception";
        }
    }

    return smilePostProc;
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
