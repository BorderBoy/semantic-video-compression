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

#include "FES.h"

#include "facedetection/facedetectcnn.h"
#define DETECT_BUFFER_SIZE 0x9000
#define faceDetectionScale 8

using namespace cv;
using namespace std;
#define ever (;;)

void testFaceDetection(Mat image, Mat& outImg){
    Mat resizedImage;
    resize(image, resizedImage, image.size() / faceDetectionScale);

    int * pResults = NULL; 
    //pBuffer is used in the detection functions.
    //If you call functions in multiple threads, please create one buffer for each thread!
    unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
    if(!pBuffer)
    {
        fprintf(stderr, "Can not alloc buffer.\n");
        exit(-1);
    }
	

	///////////////////////////////////////////
	// CNN face detection 
	// Best detection rate
	//////////////////////////////////////////
	//!!! The input image must be a BGR one (three-channel) instead of RGB
	//!!! DO NOT RELEASE pResults !!!
    TickMeter cvtm;
    cvtm.start();

	pResults = facedetect_cnn(pBuffer, (unsigned char*)(resizedImage.ptr(0)), resizedImage.cols, resizedImage.rows, (int)resizedImage.step);
    
    cvtm.stop();    
    printf("time = %gms\n", cvtm.getTimeMilli());
    
    printf("%d faces detected.\n", (pResults ? *pResults : 0));
	Mat result_image = image.clone();
    Mat result_map = Mat::zeros(image.size(), CV_8UC1);
	//print the detection results
	for(int i = 0; i < (pResults ? *pResults : 0); i++)
	{
        short * p = ((short*)(pResults + 1)) + 16*i;
		int confidence = p[0];
		int x = p[1];
		int y = p[2];
		int w = p[3];
		int h = p[4];
        
        //show the score of the face. Its range is [0-100]
        char sScore[256];
        snprintf(sScore, 256, "%d", confidence);
        cv::putText(result_image, sScore, cv::Point(x, y-3)*faceDetectionScale, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        //draw face rectangle
		rectangle(result_image, Rect(x*faceDetectionScale, y*faceDetectionScale, w*faceDetectionScale, h*faceDetectionScale), Scalar(0, 255, 0), 2);
        //draw five face landmarks in different colors
        cv::circle(result_image, cv::Point(p[5]*faceDetectionScale, p[5 + 1]*faceDetectionScale), 1, cv::Scalar(255, 0, 0), 2);
        cv::circle(result_image, cv::Point(p[5 + 2]*faceDetectionScale, p[5 + 3]*faceDetectionScale), 1, cv::Scalar(0, 0, 255), 2);
        cv::circle(result_image, cv::Point(p[5 + 4]*faceDetectionScale, p[5 + 5]*faceDetectionScale), 1, cv::Scalar(0, 255, 0), 2);
        cv::circle(result_image, cv::Point(p[5 + 6]*faceDetectionScale, p[5 + 7]*faceDetectionScale), 1, cv::Scalar(255, 0, 255), 2);
        cv::circle(result_image, cv::Point(p[5 + 8]*faceDetectionScale, p[5 + 9*faceDetectionScale]), 1, cv::Scalar(0, 255, 255), 2);
        
        //print the result
        printf("face %d: confidence=%d, [%d, %d, %d, %d] (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n", 
                i, confidence, x, y, w, h, 
                p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13],p[14]);

        // generate map
        rectangle(result_map, Rect(x*faceDetectionScale, y*faceDetectionScale, w*faceDetectionScale, h*faceDetectionScale), Scalar(255), FILLED);

	}

	outImg = result_map;

    //release the buffer
    free(pBuffer);
}

cv::Mat createMatFromBuffer(const uchar* buffer, int lines, int width, int stride) {
    // Create a Mat using the buffer
    cv::Mat mat(lines, width, CV_8U);

    // Iterate through each row and set the correct stride
    for (int i = 0; i < lines; ++i) {
        // Calculate the starting position of each row in the buffer
        const uchar* rowDataPtr = buffer + i * stride;

        // Set the data pointer for each row in the Mat
        memcpy(mat.ptr<uchar>(i), rowDataPtr, width);
    }

    return mat.clone();  // Clone the Mat to ensure data ownership
}

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

    std::cout << "Image size: " << width << "x" << lines << ", stride: " << stride << std::endl;
    std::cout << "Recieved " << request.size() << " bytes" << std::endl;

    Mat img;
    if (interleaved) {
        img = Mat(lines, width, CV_8UC2, (uchar*)request.data(), stride);
    } else {
        // img = createMatFromBuffer((uchar*)request.data(), lines, width, stride);
        img = Mat(lines, width, CV_8U, (uchar*)request.data(), stride);
    }

    return img.clone();
}

void startServer(){
    //  Socket to talk to clients
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, ZMQ_REP);
    sock.bind("tcp://localhost:5555");
    cout << "Server started" << endl;

    while (1) {
        Mat planeY = readMatFromSocket(&sock);
        Mat planeUV = readMatFromSocket(&sock);

        Mat img;
        cvtColorTwoPlane(planeY, planeUV, img, COLOR_YUV2BGR_NV12);

        testFaceDetection(img, img);
        resize(img, img, img.size() / 16); // every pixel is a macroblock

        imshow("Display Image", img);
        waitKey(1);

        // this_thread::sleep_for(chrono::milliseconds(20));
        
        const char bytes[10] = {0,1,2,3,4,5,6,7,8,9};
        zmq::message_t reply (bytes, 10);
        sock.send (reply, zmq::send_flags::none);
        cout << "Sent reply" << endl;
    }

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

    // TEST FACEDETECTION CNN
    for ever {
        cap.read(img);

        testFaceDetection(img, img);

        imshow("Display Image", img);

        if (waitKey(2) >= 0)
            break;
    }
    return 0;
    // END TEST FACEDETECTION CNN

    // TEST FES
    for ever {
        cap.read(img);

        Mat floatImg;
        img.convertTo(floatImg, CV_32F, 1/255.0);

        Mat labImg;
        labImg.create(img.size(), CV_32FC3);
        cvtColor(floatImg, labImg, COLOR_BGR2Lab);

        Mat p1 = Mat::ones(128, 171, CV_32F) / 2;

        string priorPath  = "../prior.yml";
        FileStorage fs( priorPath, FileStorage::READ);
        fs["p1"] >> p1;
        fs.release();

        printMat(p1, "p1");
        cout << p1.at<float>(0,1) << endl;
        cout << p1.at<float>(1,0) << endl;
        cout << p1.at<float>(35,154) << endl;


        Mat test = computeFinalSaliency(labImg, {8,8,8}, {13, 25, 28}, 30, 10, 1, p1);

        resize(test, test, test.size() / 16);

        threshold(test, test, 0.2, 1, THRESH_BINARY);

        Mat kernelFES = getStructuringElement(MORPH_RECT, Size(10, 10));
        morphologyEx(test, test, MORPH_ERODE, kernelFES);
        kernelFES = getStructuringElement(MORPH_RECT, Size(15, 15));
        morphologyEx(test, test, MORPH_DILATE, kernelFES);

        imshow("Display Saliency", test);
        imshow("Display Image", img);

        if (waitKey(2) >= 0)
            break;
    }

    return 0;

    // END TEST FES
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