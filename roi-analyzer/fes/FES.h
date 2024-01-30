#pragma once

#include <opencv2/opencv.hpp>
#include "../ROIDetector.h"

#define FES_WIDTH 171
#define FES_HEIGHT 128

class FES : public ROIDetector {
    // 1-Prior
    cv::Mat p0;
    // Prior
    cv::Mat p1;

    cv::Mat computeFinalSaliency(const cv::Mat& img, std::vector<int> pScale, std::vector<float> sScale, float alpha, float sigma0, float sigma1);
    cv::Mat calculateImageSaliency(const cv::Mat& img, const cv::Mat& imgT, int nSample, float radius, float sigma0, float sigma1);

    void printMat(cv::Mat mat, std::string name);
    void printVec(cv::Mat vec, int rows);

    public:
        FES();
        virtual void computeROI(cv::Mat& img, cv::Mat& roiMap);
};