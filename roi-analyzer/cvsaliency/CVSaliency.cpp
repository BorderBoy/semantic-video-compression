#include "CVSaliency.h"

using namespace cv;
using namespace std;

CVSaliency::CVSaliency(SaliencyType type) {
    switch (type)
    {
    case STATIC_FINE_GRAINED_SALIENCY:
        this->saliencyAlgorithm = saliency::StaticSaliencyFineGrained::create();
        break;
    case STATIC_SPECTRAL_RESIDUAL_SALIENCY:
        this->saliencyAlgorithm = saliency::StaticSaliencySpectralResidual::create();
        break;
    case MOTION:
        this->saliencyAlgorithm = saliency::MotionSaliencyBinWangApr2014::create();
        break;
    case OBJECTNESS:
        Ptr<saliency::ObjectnessBING> bing = saliency::ObjectnessBING::create();
        bing->setTrainingPath("../ObjectnessTrainedModel");
        this->saliencyAlgorithm = bing;
        break;
    }
}

void CVSaliency::computeROI(Mat& img, Mat& roiMap) {
    Size roiMapSize = img.size() / 16;

    Ptr<saliency::MotionSaliencyBinWangApr2014> motionSaliency = this->saliencyAlgorithm.dynamicCast<saliency::MotionSaliencyBinWangApr2014>();
    Ptr<saliency::ObjectnessBING> bing = this->saliencyAlgorithm.dynamicCast<saliency::ObjectnessBING>();

    if(!motionSaliency.empty()){
        if(fristFrame){
            fristFrame = false;

            motionSaliency->setImagesize(img.size().width, img.size().height);
            motionSaliency->init();
        }

        cvtColor(img, img, COLOR_BGR2GRAY);
        this->saliencyAlgorithm->computeSaliency(img, roiMap);
        roiMap = roiMap * 255;
        resize(roiMap, roiMap, roiMap.size() / 16);
    } else if(!bing.empty()){
        Mat resizedImg;
        resize(img, resizedImg, img.size() / BING_SCALE);

        vector<Vec4i> output;
        bing->computeSaliency(resizedImg, output);

        // vector<float> scores = bing->getobjectnessValues();

        Mat heatmap = Mat::zeros(resizedImg.rows, resizedImg.cols, CV_32S);

        for(int k = 10; k < 50; k++){
            rectangle(resizedImg, Point(output[k][0], output[k][1]), Point(output[k][2], output[k][3]), Scalar(0, 0, 255), 1, 8, 0);

            for(int i = output[k][1]; i <= output[k][3]; i++){
                for(int j = output[k][0]; j <= output[k][2]; j++){
                    heatmap.at<uint32_t>(i,j) += 1;
                }
            }
        }

        imshow("Display Image", resizedImg);

        Mat heatmap_norm;
        normalize(heatmap, heatmap_norm, 0, 255, NORM_MINMAX, CV_8UC1);
        threshold(heatmap_norm, roiMap, 100, 255, THRESH_BINARY);

        resize(roiMap, roiMap, roiMapSize);
    } else {
        this->saliencyAlgorithm->computeSaliency(img, roiMap);
        resize(roiMap, roiMap, roiMapSize);

        roiMap.convertTo(roiMap, CV_8U, 255);
    }


}