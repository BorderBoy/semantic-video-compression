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

void startServer(ROIDetector* ROI){
    //  Socket to talk to clients
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, ZMQ_REP);
    sock.bind("tcp://localhost:5555");
    cout << "Server started" << endl;

    Mat test = Mat::zeros(80, 45, CV_8UC1);

    while (1) {
        Mat planeY = readMatFromSocket(&sock);
        Mat planeUV = readMatFromSocket(&sock);

        Mat img;
        cvtColorTwoPlane(planeY, planeUV, img, COLOR_YUV2BGR_NV12);

        ROI->computeROI(img, img);

        if(!img.isContinuous()){
            std::cout << "Image is not continuous" << std::endl;
            exit(1);
        }

        zmq::message_t roiMap(img.data, img.rows * img.cols);
        // zmq::message_t roiMap(test.data, test.rows * test.cols);

        sock.send(roiMap, zmq::send_flags::none);
        // cout << "Sent reply" << endl;
    }

    sock.close();
}

void debug(ROIDetector* ROI){
    namedWindow("Display Image", WINDOW_KEEPRATIO );
    namedWindow("Display Saliency", WINDOW_KEEPRATIO );

    // capture frames from camera and display in window
    VideoCapture cap(2);
    if (!cap.isOpened()) {
        cerr << "ERROR: Unable to open the camera" << endl;
        return;
    }

    int frameCounter = 0;
    double totalTime = 0;
    for ever {
        Mat frame;
        cap >> frame;
        if (frame.empty()) {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            break;
        }

        Mat showSaliency;

        TickMeter cvtm;
        cvtm.start();
        
        ROI->computeROI(frame, showSaliency);
    
        cvtm.stop();    
        printf("time = %gms\n", cvtm.getTimeMilli());

        frameCounter++;
        totalTime += cvtm.getTimeMilli();

        resize(showSaliency, showSaliency, frame.size(), 0, 0, INTER_NEAREST);

        Mat masked;
        bitwise_and(frame, frame, masked, showSaliency);

        imshow("Display Image", masked);
        imshow("Display Saliency", showSaliency);

        // move and resize windows side by side, so they fill the screen
        moveWindow("Display Image", 0, 0);
        resizeWindow("Display Image", 640, 480);
        moveWindow("Display Saliency", 640, 0);
        resizeWindow("Display Saliency", 640, 480);

        // wait for key input

        if (waitKey(5) == 'q') {
            break;
        }
    }

    printf("Avg time/frame = %gms\n", totalTime / frameCounter);
}

int main(int argc, char** argv )
{
    ROIDetector* ROI = nullptr;
    bool useDebug = false;

    for(int i = 1; i < argc; i++){
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            useDebug = true;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mode") == 0){
            i++;
            if (strcmp(argv[i], "face") == 0){
                ROI = new FaceDetection();
            } else if (strcmp(argv[i], "fes") == 0){
                ROI = new FES();
            } else if (strcmp(argv[i], "cvsaliency") == 0){
                ROI = new CVSaliency(OBJECTNESS);
            } else {
                std::cout << "Invalid mode: " << argv[i] << std::endl;
                exit(1);
            }
        } else {
            std::cout << "Invalid argument: " << argv[i] << std::endl;
            exit(1);
        }
    }

    if(ROI == nullptr){
        ROI = new FaceDetection();
    }

    if(useDebug){
        debug(ROI);
    } else {
        startServer(ROI);
    }

    delete ROI;

    return 0;
}