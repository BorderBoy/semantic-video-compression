# Semantic video endcoding
This repo contains a pipeline to record video and semantically encode it into a H.264 byte stream (Annex B). Semantic video encoding means that the encoding will treat different parts of the image differently based on what's in them. It consists of the following parts:
1. [Camera Recorder](./camera-recorder/): Let's you record video and store the raw, unencoded frames in .YUV format
2. [x264](./x264/): An modifed version of x264 which has a mode to semantically encode the input video.
3. [ROI Analyzer](./roi-analyzer/): An ZMQ server which listens for frames from x264, calculates a ROI map and sends it back.
