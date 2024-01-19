#pragma once

#include "../ROIDetector.h"
#include <opencv2/opencv.hpp>
#include <opencv2/saliency.hpp>
#include <numeric>

enum SaliencyType {
    STATIC_FINE_GRAINED_SALIENCY,
    STATIC_SPECTRAL_RESIDUAL_SALIENCY,
    MOTION,
    OBJECTNESS
};

class CVSaliency : public ROIDetector {
    cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;
    bool fristFrame = true;
  
    public:
        CVSaliency(SaliencyType type);
        virtual void computeROI(cv::Mat& img, cv::Mat& roiMap);
};