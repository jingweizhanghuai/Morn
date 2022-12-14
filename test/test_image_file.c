#include "morn_image.h"
    
int main()
{
    MImage *img = mImageCreate();
    mImageLoad(img,"../doc/test2.jpg");
    printf("img->height=%d,img->width=%d\n",img->height,img->width);
    
    mImageSave(img,"./test3.png");
    
    mImageRelease(img);
    
}
