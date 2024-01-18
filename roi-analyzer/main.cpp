#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/saliency/saliencySpecializedClasses.hpp>
#include <opencv2/saliency.hpp>
#include <numeric>

#include <zmq.hpp>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <chrono>
#include <thread>

#include "fes/FES.h"
#include "facedetection/facedetection.h"

using namespace cv;
using namespace std;
#define ever (;;)

Mat readMatFromSocket(zmq::socket_t *sock) {
    int lines, width, stride;
    bool interleaved;

    zmq::message_t request;

    sock->recv (request, zmq::recv_flags::none);
    lines = *(int*)request.data();

    sock->recv (request, zmq::recv_flags::none);
    width = *(int*)request.data();

    sock->recv (request, zmq::recv_flags::none);
    stride = *(int*)request.data();

    sock->recv (request, zmq::recv_flags::none);
    interleaved = *(bool*)request.data();

    sock->recv (request, zmq::recv_flags::none);

    // std::cout << "Image size: " << width << "x" << lines << ", stride: " << stride << std::endl;
    // std::cout << "Recieved " << request.size() << " bytes" << std::endl;

    Mat img;
    if (interleaved) {
        img = Mat(lines, width, CV_8UC2, (uchar*)request.data(), stride);
    } else {
        img = Mat(lines, width, CV_8U, (uchar*)request.data(), stride);
    }

    return img.clone();
}

void startServer(){
    // Init ROI detector
    // ROIDetector* ROI = new FaceDetection();
    ROIDetector* ROI = new FES();
    
    //  Socket to talk to clients
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, ZMQ_REP);
    sock.bind("tcp://localhost:5555");
    // cout << "Server started" << endl;

    while (1) {
        Mat planeY = readMatFromSocket(&sock);
        Mat planeUV = readMatFromSocket(&sock);

        Mat img;
        cvtColorTwoPlane(planeY, planeUV, img, COLOR_YUV2BGR_NV12);

        ROI->computeROI(img, img);

        // imshow("Display Image", img);
        // waitKey(1);
        
        if(!img.isContinuous()){
            std::cout << "Image is not continuous" << std::endl;
            exit(1);
        }

        zmq::message_t roiMap(img.data, img.rows * img.cols);

        sock.send(roiMap, zmq::send_flags::none);
        // cout << "Sent reply" << endl;
    }

    delete ROI;

    sock.close();
}

int main(int argc, char** argv )
{
    namedWindow("Display Image", WINDOW_KEEPRATIO );
    namedWindow("Display Saliency", WINDOW_KEEPRATIO );
    
    startServer();

    Mat image;
    
    VideoCapture cap;
    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    Mat img = imread("../image_uni.jpg", IMREAD_COLOR);

    cap.read(image);

    // vector<Vec4i> output;
    // saliency::ObjectnessBING bing = saliency::ObjectnessBING();
    // bing.setTrainingPath("/Users/jonasgoos/Desktop/gits/BING-test/ObjectnessTrainedModel");

    Mat saliencyMap;
    Mat saliencyMapSpectral;
    Mat saliencyMapFineGrained;
    Ptr<saliency::Saliency> saliencySpectral = saliency::StaticSaliencySpectralResidual::create();
    Ptr<saliency::Saliency> saliencyFineGrained = saliency::StaticSaliencyFineGrained::create();

    // Ptr<saliency::Saliency> saliency = saliency::MotionSaliencyBinWangApr2014::create();
    // saliency.dynamicCast<saliency::MotionSaliencyBinWangApr2014>()->setImagesize(image.cols, image.rows);
    // saliency.dynamicCast<saliency::MotionSaliencyBinWangApr2014>()->init();

    for ever{
        cap.read(image);
        
        // check if we succeeded
        if (image.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            continue;
        }

        // ### BING ###
        // bing.computeSaliency(image, output);

        // vector<float> scores;
        // scores = bing.getobjectnessValues();

        // std::vector<int> indices(output.size());
        // std::iota(indices.begin(), indices.end(), 0);
        // std::sort(indices.begin(), indices.end(),
        //     [&](int A, int B) -> bool {
        //             return scores[A] < scores[B];
        //         });


        // // sort output according to indices
        // vector<Vec4i> output_sorted(output.size());
        // for (int i = 0; i < indices.size(); i++) {
        //     output_sorted[i] = output[indices[i]];

        // }

        // Mat heatmap = Mat::zeros(image.rows, image.cols, CV_32S);

        // for(int k = 0; k < 100; k++){
        //     for(int i = output_sorted[k][1]; i <= output_sorted[k][3]; i++){
        //         for(int j = output_sorted[k][0]; j <= output_sorted[k][2]; j++){
        //             heatmap.at<uint32_t>(i,j) += 1;
        //         }
        //     }
        // }

        // Mat heatmap_norm;
        // normalize(heatmap, heatmap_norm, 0, 255, NORM_MINMAX, CV_8UC1);
        // // threshold(heatmap_norm, heatmap_norm, 100, 255, THRESH_BINARY);

        // // rectangle(image, Point(0,0), Point(200,200), Scalar(0, 255, 0), 10);
        // applyColorMap(heatmap_norm, image, COLORMAP_JET);

        // ### Static Saliency ###
        saliencySpectral->computeSaliency(image, saliencyMapSpectral);
        saliencyMapSpectral.convertTo(saliencyMapSpectral, CV_8UC3, 256);

        saliencyFineGrained->computeSaliency(image, saliencyMapFineGrained);
        saliencyMapFineGrained.convertTo(saliencyMapFineGrained, CV_8UC3, 256);

        saliencyMap = (saliencyMapSpectral + saliencyMapFineGrained) / 2;

        threshold(saliencyMap, saliencyMap, 50, 255, THRESH_BINARY);

        Mat kernel = getStructuringElement(MORPH_RECT, Size(75, 75));
        morphologyEx(saliencyMap, saliencyMap, MORPH_ERODE, kernel);
        kernel = getStructuringElement(MORPH_RECT, Size(150, 150));
        morphologyEx(saliencyMap, saliencyMap, MORPH_DILATE, kernel);



        // ### Motion Saliency ###
        // cvtColor(image, image, COLOR_BGR2GRAY);
        // saliency->computeSaliency(image, saliencyMap);

        // Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        // morphologyEx(saliencyMap, saliencyMap, MORPH_CLOSE, kernel);

        // saliencyMap = saliencyMap * 255;


        // show live and wait for a key with timeout long enough to show images
        imshow("Display Image", image);
        imshow("Display Saliency", saliencyMap);
        if (waitKey(2) >= 0)
            break;

    }
    return 0;
}