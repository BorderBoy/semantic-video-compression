# Camera Recorder
Records video of camera in YUV file format using OpenCV.

## Setup
Make sure to have OpenCV installed. Then just build using CMake:

1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. `make`
5. `./Recorder`

## Usage
Make sure the correct video capture device is selected in `main.cpp:26`. Press 'r' to start/stop recording. The recorded video files will be stored in this directory and will have the following format: `YYYY-MM-DD_HH:mm:ss_WIDTHxHEIGHT_FPS.YUV`. Note that resolution and framrate are not stored in the YUV file format, therefore it is saved in the filename.