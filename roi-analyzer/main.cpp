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

    delete ROI;

    sock.close();
}

void debug(){
    // capture frames from camera and display in window
    VideoCapture cap(2);
    if (!cap.isOpened()) {
        cerr << "ERROR: Unable to open the camera" << endl;
        return;
    }

    // Init ROI detector
    // ROIDetector* ROI = new FaceDetection();
    ROIDetector* ROI = new FES();
    // ROIDetector* ROI = new CVSaliency(OBJECTNESS);

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

        imshow("Display Image", frame);
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

    delete ROI;
}

int main(int argc, char** argv )
{
    namedWindow("Display Image", WINDOW_KEEPRATIO );
    namedWindow("Display Saliency", WINDOW_KEEPRATIO );

    if(argc > 1 && strcmp(argv[1], "--debug") == 0){
        cout << "Entering debug mode" << endl;
        debug();
        return 0;
    }
    
    startServer();
}