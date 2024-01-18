#pragma once

#include <opencv2/opencv.hpp>
#include "../ROIDetector.h"

#define FACE_DETECTION_SCALE 8
#define DETECT_BUFFER_SIZE 0x9000

class FaceDetection : public ROIDetector {
    public:
        virtual void computeROI(cv::Mat& img, cv::Mat& roiMap);
};