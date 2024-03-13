#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>
#include <zmq.hpp>
#include <chrono>

using namespace cv;
using namespace std;
using namespace std::chrono;

int frameRate = -1;

string createVideoWriter(VideoWriter& wr, const Mat& frame){
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    string filename = "../" + oss.str() + "_" + to_string(frame.cols) + "x" + to_string(frame.rows) + "_" + to_string(frameRate) + ".yuv";
    wr = VideoWriter(filename, 0, 1, Size(frame.cols, frame.rows));

    return oss.str() + "_" + to_string(frame.cols) + "x" + to_string(frame.rows) + "_" + to_string(frameRate);
}

int main(int argc, char** argv) {
    bool useZMQ = false;

    if (argc > 1 && strcmp(argv[1], "--zmq") == 0) {
        useZMQ = true;
    }

    // Open a connection to the webcam (default camera index 0)
    VideoCapture cap;
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    cap.open(deviceID, apiID);
    cap.set(CAP_PROP_AUTO_WB, 0);
    cout << "CAP_PROP_AUTO_WB: " << cap.get(CAP_PROP_AUTO_WB) << endl;

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
    namedWindow("Webcam", WINDOW_NORMAL);

    // ZMQ
    zmq::context_t context{1};
    zmq::socket_t socket;
    if(useZMQ){
        // construct a REQ (request) socket and connect to interface
        socket = zmq::socket_t{context, zmq::socket_type::req};
        socket.connect("tcp://localhost:5555");
        cout << "Connecting to ZMQ server..." << endl;
    }

    Mat *frames = new Mat[9000]; // 5 minutes of recording at 30fps
    size_t frame_count = 0;

    auto last_capture = high_resolution_clock::now();

    bool stopRecording = false;

    while (running) {
        // Capture a frame from the webcam
        cap >> frame;

        double time = duration_cast<milliseconds>(high_resolution_clock::now() - last_capture).count();
        if(time > 1.5f * 1000.0f/frameRate){
            cout << "FRAME SKIPPED: " << time << endl;
        }

        last_capture = high_resolution_clock::now();

        // Check if the frame is empty (end of video)
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame from webcam." << std::endl;
            stopRecording = true;
        }

        switch(waitKey(1)){
        case 'q':
            running = false;
            break;

        case 'r':
            if(!recording){
                recording = true;
                string filename = createVideoWriter(writer, frame);

                if(useZMQ){ 
                    const std::string data{"start:"+filename};
                    socket.send(zmq::buffer(data), zmq::send_flags::none);
                        
                    // wait for reply from server
                    zmq::message_t reply{};
                    socket.recv(reply, zmq::recv_flags::none);

                    if(reply.to_string() != "OK"){
                        std::cerr << "Error: ZMQ server did not sent OK" << endl;
                    }
                }
                
                // Check if the video writer is opened successfully
                if (!writer.isOpened()) {
                    std::cerr << "Error: Could not open video writer." << std::endl;
                    return -1;
                }
            } else {
                stopRecording = true;
            }
            break;
        }

        if(stopRecording){
            if(useZMQ){
                const std::string data{"stop"};
                socket.send(zmq::buffer(data), zmq::send_flags::none);
                    
                // wait for reply from server
                zmq::message_t reply{};
                socket.recv(reply, zmq::recv_flags::none);

                if(reply.to_string() != "OK"){
                    std::cerr << "Error: ZMQ server did not sent OK" << endl;
                }
            }
            
            // Write the YUV frames to the video file
            for(size_t i = 0; i < frame_count; i++){
                writer.write(frames[i]);
                frames[i].release();
            }

            frame_count = 0;
            recording = false;
            writer.release();

            stopRecording = false;
        }

        if(recording){
            // Store the frame in the buffer
            frames[frame_count++] = frame.clone();
            putText(frame, "RECORDING", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2, LINE_AA);
        } else {
            putText(frame, "Press 'r' to start recording", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2, LINE_AA);
        }

        cv::imshow("Webcam", frame);
    }

    delete[] frames;

    // Release the VideoCapture and VideoWriter objects
    cap.release();
    writer.release();

    socket.close();
    
    // Close any OpenCV windows (optional, uncomment if needed)
    cv::destroyAllWindows();

    return 0;
}
