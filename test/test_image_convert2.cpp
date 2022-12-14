// build: g++ -O2 -fopenmp test_image_convert2.cpp -o test_image_convert2.exe -lopencv_imgproc -lopencv_core -lopencv_imgcodecs -lmorn
#include "morn_image.h"
#include "opencv2/opencv.hpp"

void test_opencv()
{
    cv::Mat src_cv=cv::imread("./test3.png");
    cv::Mat dst_cv;
    
    printf("convert image from RGB to YUV:\n");
    mTimerBegin("OpenCV");
    for(int i=0;i<1000;i++)
        cv::cvtColor(src_cv,dst_cv,CV_BGR2YUV);
    mTimerEnd("OpenCV");
    
    printf("convert image from YUV to RGB:\n");
    mTimerBegin("OpenCV");
    for(int i=0;i<1000;i++)
        cv::cvtColor(dst_cv,src_cv,CV_YUV2BGR);
    mTimerEnd("OpenCV");
}

void test_Morn()
{
    MImage *src_morn = mImageCreate();
    MImage *dst_morn = mImageCreate();
    mImageLoad(src_morn,"./test3.png"); 
    
    printf("convert image from RGB to YUV:\n");
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mImageRGBToYUV(src_morn,dst_morn);
    mTimerEnd("Morn");

    printf("convert image from YUV to RGB:\n");
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mImageYUVToRGB(dst_morn,src_morn);
    mTimerEnd("Morn");
    
    mImageRelease(src_morn);
    mImageRelease(dst_morn);
}

int main(int argc,char *argv[])
{
    if(strcasecmp(argv[1],"opencv")==0) test_opencv();
    else                                test_Morn();
    return 0;
}

