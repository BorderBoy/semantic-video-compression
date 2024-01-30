# ZMQ Server for ROI analyzation
This is a ZMQ server which will run and wait for frames, it will then calculate a ROI map using one of several methods and send it back to the requester.

## Setup
1. Make sure you have OpenCV installed
2. Install [`libzmq`](https://github.com/zeromq/libzmq) and [`cppzmq`](https://github.com/zeromq/cppzmq) via [CMake](https://github.com/zeromq/cppzmq?tab=readme-ov-file#build-instructions).
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make`

## Usage
**`-m`/`--mode`**: Select ROI detection mode
  - `face`: Face detection based on [libfacedetection](https://github.com/ShiqiYu/libfacedetection) (default)
  - `fes`: Fast and efficient saliency based on [this paper](https://link.springer.com/chapter/10.1007/978-3-642-21227-7_62)
  - `cvsaliency`: OpenCV saliency modules
  
**`-d`/`--debug`**: Starts the program in debug mode. Camera is used as input and ROI map is displayed. Also prints frame stats. **This will not start the ZMQ server.**

The server will be started on `localhost:5555`.
