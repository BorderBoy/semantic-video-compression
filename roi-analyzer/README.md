# ZMQ Server for ROI analyzation
This is a ZMQ server which will run and wait for frames, it will then calculate a ROI map using one of several methods and send it back to the requester.

## Setup
1. Make sure you have OpenCV installed
2. Install [`libzmq`](https://github.com/zeromq/libzmq) and [`cppzmq`](https://github.com/zeromq/cppzmq) via [CMake]((https://github.com/zeromq/cppzmq?tab=readme-ov-file#build-instructions)).
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make`

## Usage
To start the ZMQ server: `./ROI-Detector`. The server will be started on: `localhost:5555`.

To start a debug version which uses the camera as input and shows the resulting ROI map use the `--debug` option.
