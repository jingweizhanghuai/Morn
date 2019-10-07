/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译：g++ -O2 -fopenmp test_image_resize2.cpp -I E:\opencv\install\include -I ..\include\ -L E:\opencv\install\x64\mingw\staticlib -lopencv_world331 -llibjpeg -llibjasper -lIlmImf -llibtiff  -llibwebp -llibpng -lzlib -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_image_resize2.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "morn_Image.h"

using namespace cv;

int main()
{
    //////////////////////////////////////////////////////////////
    mLog(INFO,"image resize using opencv");
    Mat src = imread("../doc/test.jpg"); 
    Mat dst;
    
    mTimerBegin();
    for(int i=0;i<10;i++)
        resize(src,dst,Size(256,256),(0, 0),(0, 0),INTER_LINEAR);
    mTimerEnd();
    
    imwrite("./test_resize_cv.jpg",dst);
    //////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////
    mLog(INFO,"image resize using morn");
    MImage *in = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test.jpg",in);
    MImage *out = mImageCreate(in->channel,256,256,NULL);
    
    mTimerBegin();
    for(int i=0;i<10;i++)
        mImageResize(in,out,DFLT,DFLT,DFLT);
    mTimerEnd();
    
    mImageSave(out,"./test_resize_morn.jpg");
    
    mImageRelease(in);
    mImageRelease(out);
    //////////////////////////////////////////////////////////////

    return 0;
}