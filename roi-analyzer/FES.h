#pragma once

#include <stdio.h>
#include <opencv2/opencv.hpp>

void printMat(cv::Mat mat, std::string name);
cv::Mat computeFinalSaliency(const cv::Mat& img, std::vector<int> pScale, std::vector<float> sScale, float alpha, float sigma0, float sigma1, const cv::Mat& p1);
cv::Mat calculateImageSaliency(const cv::Mat& img, int nSample, float radius, float sigma0, float sigma1, const cv::Mat& ph1);
void printVec(cv::Mat vec, int rows);