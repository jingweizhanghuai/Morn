#include "morn_image.h"

// int main1()
// {
//     MImage *src = mImageCreate();
//     mImageLoad(src,"./test3.png");
//     
//     MImage *dst = mImageCreate();
//     mImageRGBToGray(src,dst);
//     
//     mImageSave(dst,"./test3_gray.png");
//     
//     mImageRelease(src);
//     mImageRelease(dst);
// }

int main()
{
    MImage *src = mImageCreate();
    mImageLoad(src,"./test3.png");
    
    MImage *dst = mImageCreate();
    
//     mImageRGBToYUV(src,dst);
//     mImageYUVToRGB(dst,src);
//     mImageSave(src,"./test3_rgb.png");
    
    mTimerBegin();
    for(int i=0;i<1000;i++)
    mImageRGBToHSV(src,dst);
    mTimerEnd();
    
    mTimerBegin();
    for(int i=0;i<1000;i++)
    mImageHSVToRGB(dst,src);
    mTimerEnd();
    mImageSave(src,"./test3_rgb.png");
    
    mImageRelease(src);
    mImageRelease(dst);
}
