#include "FES.h"

using namespace cv;
using namespace std;

FES::FES() {
    string priorPath  = "../prior.yml";
    FileStorage fs( priorPath, FileStorage::READ);
    fs["p1"] >> prior;
    fs.release();
}

void FES::computeROI(Mat& img, Mat& roiMap) {
    Size roiMapSize = img.size() / 16;

    Mat resizedImg;
    resize(img, resizedImg, Size(171, 128));

    Mat floatImg;
    resizedImg.convertTo(floatImg, CV_32F, 1/255.0);

    Mat labImg;
    labImg.create(img.size(), CV_32FC3);
    cvtColor(floatImg, labImg, COLOR_BGR2Lab);

    roiMap = computeFinalSaliency(labImg, {8,8,8}, {13, 25, 28}, 30, 10, 1, this->prior);
    
    resize(roiMap, roiMap, roiMapSize);

    roiMap.convertTo(roiMap, CV_8U, 255);
    threshold(roiMap, roiMap, 100, 255, THRESH_BINARY);

    Mat kernelFES = getStructuringElement(MORPH_RECT, Size(5, 5));
    morphologyEx(roiMap, roiMap, MORPH_ERODE, kernelFES);
    kernelFES = getStructuringElement(MORPH_RECT, Size(10, 10));
    morphologyEx(roiMap, roiMap, MORPH_DILATE, kernelFES);
}

// compute multi scale saliency over an image 
//
// @input
//   img - a given image to process
//   pScale - precission scale (number of samples) [1xn vector]
//   sScale - size scale (sampling raduis) [1xn vector]
//   alpha - attenuation factor [1x1 variable]
//   sigma0 : standard deviation of kernels in surround [1x1 variable]
//   sigma1 : standard deviation of kernel in center [1x1 variable]
//   p1 - P(1|x) [128x171 matrix]
// @output
//   saliency - saliency of inputed image
// 
// please refer to the following paper for details
// Rezazadegan Tavakoli H, Rahtu E & Heikkil? J, 
// "Fast and efficient saliency detection using sparse sampling and kernel density estimation."
// Proc. Scandinavian Conference on Image Analysis (SCIA 2011), 2011, Ystad, Sweden.
//
// The code has been tested on Matlab 2010a (32-bit) running windows. 
// This code is publicly available for demonstration and educational
// purposes, any commercial use without permission is strictly prohibited.  
//
// Please contact the author in case of any questions, comments, or Bug
// reports
//
// @CopyRight: Hamed Rezazadegan Tavakoli
// @Contact Email: hrezazad@ee.oulu.fi
// @date  : 2010
// @version: 0.1
Mat FES::computeFinalSaliency(const Mat& img, vector<int> pScale, vector<float> sScale, float alpha, float sigma0, float sigma1, const Mat& p1) {
    Size imgSize = img.size();

    // Mat resizedImg;
    // resize(img, resizedImg, Size(171, 128));

    if (p1.rows != 128 || p1.cols != 171 || p1.channels() != 1) {
        cout << "p1 must be 128x171x1" << endl;
        exit(1);
    }

    int n = pScale.size();

    vector<Mat> saliencies;

    for (int i = 0; i < n; ++i) {
        Mat temp = calculateImageSaliency(img, pScale[i], sScale[i], sigma0, sigma1, p1);
        
        saliencies.push_back(temp);
    }

    // Combine channels
    Mat saliency;
    merge(saliencies, saliency);
    
    // Gaussian blur
    GaussianBlur(saliency, saliency, Size(25, 25), 0.2*26);

    // Raise to power alpha
    pow(saliency, alpha, saliency);
 
    // Mean over channels
    transform(saliency, saliency, cv::Matx13f(1,1,1)); // sum over scales
    saliency = saliency / n;
  
    // Normalize
    double minVal, maxVal;
    minMaxLoc(saliency, &minVal, &maxVal);
    saliency = (saliency - minVal) / (maxVal - minVal);
 
    // Resize to original size
    // resize(saliency, saliency, imgSize);
 
    return saliency;
}

// calculates the saliency of an image at a given scale
//
// @param
//   img : an image
//   nSample : number of samples in the center
//   radius : radius of the circular samples are going to be take from
//   sigma0 : standard deviation of kernels in surround
//   sigma1 : standard deviation of kernel in center
//   ph1 : P(1|x)
// 
// please refer to the following paper for details
// Rezazadegan Tavakoli H, Rahtu E & Heikkil� J, 
// "Fast and efficient saliency detection using sparse sampling and kernel density estimation."
// Proc. Scandinavian Conference on Image Analysis (SCIA 2011), 2011, Ystad, Sweden.
//
// The code has been tested on Matlab 2010a (32-bit) running windows. 
// This code is publicly available for demonstration and educational
// purposes, any commercial use without permission is strictly prohibited.  
//
// Please contact the author in case of any questions, comments, or Bug
// reports
//
// @CopyRight: Hamed Rezazadegan Tavakoli
// @Contact Email: hrezazad@ee.oulu.fi
// @date  : 2010
// @version: 0.1
Mat FES::calculateImageSaliency(const Mat& img, int nSample, float radius, float sigma0, float sigma1, const Mat& ph1) {
    int nrow = img.rows;
    int ncol = img.cols;
    int nChannel = img.channels();

    if (nChannel != 3) {
        std::cerr << "This works only on images of 3 channels, preferably LAB color space needed" << std::endl;
        // Handle the error appropriately, e.g., return an empty Mat or throw an exception
    }

    vector<int> new_shape {nrow * ncol, nChannel};
    // We want to concat cols not the rows, so we transpose the image
    Mat fMat = img.t(); // checked // only transpose once
    fMat = fMat.reshape(nChannel, {nrow * ncol, 1}); // Build feature matrix, type: 32FC3

    // Get center coordinates
    Mat yc; // 1...12...23...ncol, every number nrow times, checked
    Mat xc; // 1...nrow repeated ncol times, checked

    vector<int> xc_vec;
    for(int i = 0; i < nrow; i++){
        xc_vec.push_back(i);
    }
    xc = repeat(Mat(xc_vec), ncol, 1);

    vector<int> yc_vec;
    for(int i = 0; i < ncol; i++){
        yc_vec.push_back(i);
    }

    yc = repeat(Mat(yc_vec), 1, nrow);
    yc = yc.reshape(1, nrow * ncol);

    // Process each sample
    std::vector<int> x, y; // both checked
    for (int i = 1; i <= nSample; ++i) {
        x.push_back(static_cast<int>(radius * cos(2.0 * CV_PI * i / nSample)));
        y.push_back(static_cast<int>(-radius * sin(2.0 * CV_PI * i / nSample)));
    }

    Mat LxcH0 = Mat::zeros(nrow * ncol, 1, CV_32F); // checked

    for (int i = 0; i < nSample; ++i) {
        Mat fx = min(max(xc + x[i], 1), nrow); // type: 32SC1
        Mat fy = min(max(yc + y[i], 1), ncol); // type: 32SC1
 
        Mat indY = fx + (fy-1) * nrow;          // type: 32SC1
        // indY.convertTo(indY, CV_32F);           // type: 32FC1

        Mat indX = Mat::zeros(nrow * ncol, 1, CV_32S); // type: 32SC1
        // indX.convertTo(indX, CV_32F);          // type: 32FC1

        Mat ind;
        merge(vector<Mat>{indX, indY}, ind);  // type: 32SC2
        ind.convertTo(ind, CV_16S);           // type: 16SC2

        indX.convertTo(indX, CV_32F);          // type: 32FC1
        indY.convertTo(indY, CV_32F);           // type: 32FC1

        Mat fMatc;
        remap(fMat, fMatc, indX, indY, INTER_LINEAR);
        Mat fMatc2;
        // remap(fMat, fMatc2, ind, Mat(), INTER_LINEAR);

        // cout << "fMatc: " << fMatc.at<Vec3f>(0) << ", " << fMatc.at<Vec3f>(1) << ", " << fMatc.at<Vec3f>(2) << endl;
        // cout << "fMatc2: " << fMatc2.at<Vec3f>(0) << ", " << fMatc2.at<Vec3f>(1) << ", " << fMatc2.at<Vec3f>(2) << endl;

        // cout << (fMatc.at<Vec3f>(0) == fMatc2.at<Vec3f>(0) ? "Passed" : "Failed") << endl;

        Mat temp = fMatc - fMat;
        temp = temp.reshape(1, {nrow * ncol, nChannel});

        temp = temp.mul(temp);              // element-wise square
        reduce(temp, temp, 1, REDUCE_SUM);  // sum over channels
        temp = temp / (2 * sigma0 * sigma0);
        exp(-temp, temp);                   // element-wise exp
        LxcH0 += temp;
    }

    Mat ph0 = 1 - ph1; // CV_32FC1, checked

    Mat ph0_vec = ph0.t(); // only transpose once
    ph0_vec = ph0_vec.reshape(1, nrow*ncol);     // CV_32FC1
    Mat ph1_vec = ph1.t(); // only transpose once
    ph1_vec = ph1_vec.reshape(1, nrow*ncol);     // CV_32FC1

    Mat constMat = (ph0_vec * sigma1) / (ph1_vec * nSample * sigma0); // checked

    // Compute final posterior
    Mat PH1xc = 1 / (1 + constMat.mul(LxcH0)); // checked

    Mat saliency = PH1xc.reshape(1, {ncol, nrow});
    saliency = saliency.t();    // only transpose once

    return saliency;
}

void FES::printVec(Mat vec, int rows){
    for(int i = 0; i < rows; i++){
        cout << vec.at<int>(i) << " ";
    }
    cout << endl;
}

void FES::printMat(Mat mat, string name){
    cout << name << ": ",
    cout << "size: " << mat.rows << "x" << mat.cols << "x" << mat.channels();
    cout << ", type: " << mat.type() <<  endl;

    switch(mat.type()){
        case CV_8UC1:
            cout << mat.at<uint8_t>(0,0) << endl;
            break;
        case CV_8UC3:
            cout << mat.at<Vec3b>(0,0) << endl;
            break;
        case CV_32FC3:
            cout << mat.at<Vec3f>(0,0) << endl;
            break;
        case CV_32FC1:
            cout << mat.at<float>(0,0) << endl;
            break;
        case CV_32SC1:
            cout << mat.at<int>(0,0) << endl;
            break;
        default:
            cout << "unknown type" << endl;
    }
}