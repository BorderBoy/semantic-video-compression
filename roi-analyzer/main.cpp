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
#include "cvsaliency/CVSaliency.h"

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
    ROIDetector* ROI = new FaceDetection();
    // ROIDetector* ROI = new FES();
    // ROIDetector* ROI = new CVSaliency(OBJECTNESS);
    
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

        imshow("Display Image", img);

        ROI->computeROI(img, img);

        Mat showSaliency;
        resize(img, showSaliency, img.size() * 16, INTER_NEAREST);

        imshow("Display Saliency", showSaliency);
        waitKey(1);
        
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
}