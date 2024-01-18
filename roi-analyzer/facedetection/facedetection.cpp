/*
By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                  License Agreement For libfacedetection
                     (3-clause BSD License)

Copyright (c) 2018-2020, Shiqi Yu, all rights reserved.
shiqi.yu@gmail.com

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

#include "facedetection.h"
#include "facedetectcnn.h"

using namespace cv;

void FaceDetection::computeROI(Mat& image, Mat& roiMap){
    Mat resizedImage;
    resize(image, resizedImage, image.size() / FACE_DETECTION_SCALE);

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
    // printf("time = %gms\n", cvtm.getTimeMilli());
    
    // printf("%d faces detected.\n", (pResults ? *pResults : 0));
	Mat result_image = image.clone();
    roiMap = Mat::zeros(image.size(), CV_8UC1);
	//print the detection results
	for(int i = 0; i < (pResults ? *pResults : 0); i++)
	{
        short * p = ((short*)(pResults + 1)) + 16*i;
		int confidence = p[0];
		int x = p[1];
		int y = p[2];
		int w = p[3];
		int h = p[4];
        
        //print the result
        // printf("face %d: confidence=%d, [%d, %d, %d, %d] (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n", 
                // i, confidence, x, y, w, h, 
                // p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13],p[14]);

        // generate map
        rectangle(roiMap, Rect(x*FACE_DETECTION_SCALE, y*FACE_DETECTION_SCALE, w*FACE_DETECTION_SCALE, h*FACE_DETECTION_SCALE), Scalar(255), FILLED);
	}

    resize(roiMap, roiMap, roiMap.size() / 16);

    //release the buffer
    free(pBuffer);
}