#pragma once

#include <opencv2/opencv.hpp>

class ROIDetector {
    public:
        virtual ~ROIDetector() {};
         /**
         * Computes the ROIs of an image.
         * Takes and image and returns a roi map.
         * Image must be of type CV_8UC3 and dimensions must be divisible by 16.
         * ROI map is of type CV_8UC1 and will be 1/16th the size of the image in both dimension.
         * Greater values in the ROI map indicate greater saliency.
         */
        virtual void computeROI(cv::Mat& img, cv::Mat& roiMap) = 0;
};