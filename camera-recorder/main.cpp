#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>


using namespace cv;
using namespace std;

int frameRate = -1;

void createVideoWriter(VideoWriter& wr, const Mat& frame){
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    string filename = "../" + oss.str() + "_" + to_string(frame.cols) + "x" + to_string(frame.rows) + "_" + to_string(frameRate) + ".yuv";
    wr = VideoWriter(filename, 0, 1, Size(frame.cols, frame.rows));
}

int main() {
    // Open a connection to the webcam (default camera index 0)
    VideoCapture cap;
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    cap.open(deviceID, apiID);

    frameRate = cap.get(CAP_PROP_FPS);

    cout << "Frame rate: " << frameRate << endl;

    // Check if the webcam is opened successfully
    if (!cap.isOpened()) {
        cerr << "Error: Could not open webcam." << std::endl;
        return -1;
    }

    // Open a video writer for YUV format
    VideoWriter writer;

    bool recording = false;
    bool running = true;
    Mat frame;
    while (running) {
        // Capture a frame from the webcam
        cap >> frame;

        // Check if the frame is empty (end of video)
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame from webcam." << std::endl;
            break;
        }

        switch(waitKey(1)){
        case 'q':
            running = false;
            break;

        case 'r':
            if(!recording){
                recording = true;
                createVideoWriter(writer, frame);
                
                // Check if the video writer is opened successfully
                if (!writer.isOpened()) {
                    std::cerr << "Error: Could not open video writer." << std::endl;
                    return -1;
                }
            } else {
                recording = false;
                writer.release();
            }
            break;
        }

        if(recording){
            // Write the YUV frame to the video file
            writer.write(frame);
            putText(frame, "RECORDING", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2, LINE_AA);
        } else {
            putText(frame, "Press 'r' to start recording", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2, LINE_AA);
        }

        cv::imshow("Webcam", frame);
    }

    // Release the VideoCapture and VideoWriter objects
    cap.release();
    writer.release();

    // Close any OpenCV windows (optional, uncomment if needed)
    cv::destroyAllWindows();

    return 0;
}
