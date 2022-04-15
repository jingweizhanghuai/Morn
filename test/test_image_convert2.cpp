// g++ -O2 -fopenmp test_image_convert2.cpp -o test_image_convert2.exe -L C:\ProgramFiles\OpenCV\lib\mingw -lopencv_world349 -lmorn -ljpeg -lpng -lz
#include "morn_image.h"
#include "opencv2/opencv.hpp"

int main()
{
    MImage *src_morn = mImageCreate();
    MImage *dst_morn = mImageCreate();
    mImageLoad(src_morn,"../doc/test.jpg"); 

    cv::Mat src_cv(src_morn->height,src_morn->width, CV_8UC3);
    cv::Mat dst_cv;
    mImageDataOutput(src_morn,src_cv.data);

    printf("convert image from RGB to YUV:\n");
    
    mTimerBegin("OpenCV");
    for(int i=0;i<1000;i++)
        cv::cvtColor(src_cv,dst_cv,CV_BGR2YUV);
    mTimerEnd("OpenCV");

    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mImageRGBToYUV(src_morn,dst_morn);
    mTimerEnd("Morn");

    printf("convert image from YUV to RGB:\n");
    
    mTimerBegin("OpenCV");
    for(int i=0;i<1000;i++)
        cv::cvtColor(dst_cv,src_cv,CV_YUV2BGR);
    mTimerEnd("OpenCV");

    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mImageYUVToRGB(dst_morn,src_morn);
    mTimerEnd("Morn");
    
    // mImageSave(src_morn,"./test_convert.png");
    // mImageDataInput(src_morn,src_cv.data);
    // mImageSave(src_morn,"./test_convert2.png");

    mImageRelease(src_morn);
    mImageRelease(dst_morn);
    return 0;
}
