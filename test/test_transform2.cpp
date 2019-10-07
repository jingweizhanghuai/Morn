/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译：g++ -O2 -fopenmp test_transform2.cpp -I E:\opencv\install\include -I ..\include\ -L E:\opencv\install\x64\mingw\staticlib -lopencv_world331 -llibjpeg -llibjasper -lIlmImf -llibtiff  -llibwebp -llibpng -lzlib -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_transform2.exe
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
    Mat src = imread("../doc/test2.jpg");
    Mat dst;
    Point2f ps[4] = {Point2f( 0,  0),Point2f(  0,500),Point2f(500,500),Point2f(500, 0)};
	Point2f pd[4] = {Point2f(50,-30),Point2f(-50,480),Point2f(520,420),Point2f(390,60)};
    
    mLog(INFO,"image affine using opencv");
    mTimerBegin();
    Mat affine_trans = getAffineTransform(ps,pd);
    for(int i=0;i<100;i++)
        warpAffine(src,dst,affine_trans,Size(src.cols,src.rows));
    mTimerEnd();
    imwrite("./test_affine_cv.jpg",dst);
    
    mLog(INFO,"image perspective using opencv");
    mTimerBegin();
    Mat perspective_trans = getPerspectiveTransform(ps, pd);
	for(int i=0;i<100;i++)
        warpPerspective(src,dst,perspective_trans,Size(src.cols, src.rows));
    mTimerEnd();
    imwrite("./test_perspective_cv.jpg",dst);
    //////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////
    MImage *in = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test2.jpg",in);
    MImage *out = mImageCreate(DFLT,DFLT,DFLT,NULL);
    
    MImagePoint pi[4]; pi[0].x=  0;pi[0].y=  0;pi[1].x=  0;pi[1].y=500;pi[2].x=500;pi[2].y=500;pi[3].x=500;pi[3].y= 0;
    MImagePoint po[4]; po[0].x= 50;po[0].y=-30;po[1].x=-50;po[1].y=480;po[2].x=520;po[2].y=420;po[3].x=390;po[3].y=60;
    
    mLog(INFO,"image affine using morn");
    mTimerBegin();
    for(int i=0;i<100;i++)
        mImageAffineCorrection(in,out,pi,po);
    mTimerEnd();
    mImageSave(out,"./test_affine_morn.jpg");
    
    mLog(INFO,"image perspective using morn");
    mTimerBegin();
    for(int i=0;i<100;i++)
        mImagePerspectiveCorrection(in,out,pi,po);
    mTimerEnd();
    mImageSave(out,"./test_perspective_morn.jpg");
    
    mImageRelease(in);
    mImageRelease(out);
    //////////////////////////////////////////////////////////////

    return 0;
}
    