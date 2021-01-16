/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#ifdef USE_OPENCV
// build: g++ -O2 -fopenmp test_transform2.cpp -o test_transform2_opencv.exe -DUSE_OPENCV -I C:\ProgramFiles\OpenCV\include -I ..\include -L C:\ProgramFiles\OpenCV\lib\mingw -lopencv_world349 -llibjpeg-turbo -llibjasper -lIlmImf -llibtiff  -llibwebp -llibpng -lzlib -L ..\lib\x64_mingw -lmorn

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "morn_util.h"

using namespace cv;
int main()
{
    Mat src = imread("../doc/test2.jpg");
    Mat dst;
    Point2f ps[4] = {Point2f( 0,  0),Point2f(  0,500),Point2f(500,500),Point2f(500, 0)};
	Point2f pd[4] = {Point2f(50,-30),Point2f(-50,480),Point2f(520,420),Point2f(390,60)};
    
    mTimerBegin("affine transform by OpenCV");
    Mat affine_trans = getAffineTransform(ps,pd);
    for(int i=0;i<100;i++)
        warpAffine(src,dst,affine_trans,Size(src.cols,src.rows));
    mTimerEnd("affine transform by OpenCV");
    imwrite("./test_affine_cv.jpg",dst);
    
    mTimerBegin("perspective transform by OpenCV");
    Mat perspective_trans = getPerspectiveTransform(ps, pd);
	for(int i=0;i<100;i++)
        warpPerspective(src,dst,perspective_trans,Size(src.cols, src.rows));
    mTimerEnd("perspective transform by OpenCV");
    imwrite("./test_perspective_cv.jpg",dst);

    return 0;
}
#endif

#ifdef USE_MORN
// build: g++ -O2 -fopenmp test_transform2.cpp -o test_transform2_morn.exe -DUSE_MORN -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -ljpeg -lpng -lz
#include "morn_image.h"

int main()
{
    MImage *in = mImageCreate();
    MImage *out = mImageCreate();
    mImageLoad(in,"../doc/test2.jpg");
    
    MImagePoint pi[4]; mPoint(pi+0, 0,  0);mPoint(pi+1,  0,500);mPoint(pi+2,500,500);mPoint(pi+3,500, 0);
    MImagePoint po[4]; mPoint(po+0,50,-30);mPoint(po+1,-50,480);mPoint(po+2,520,420);mPoint(po+3,390,60);
    
    mTimerBegin("affine transform by Morn");
    for(int i=0;i<100;i++)
        mImageAffineCorrection(in,out,pi,po,DFLT);
    mTimerEnd("affine transform by Morn");
    mImageSave(out,"./test_affine_morn.jpg");
    
    mTimerBegin("perspective transform by Morn");
    for(int i=0;i<100;i++)
        mImagePerspectiveCorrection(in,out,pi,po,DFLT);
    mTimerEnd("perspective transform by Morn");
    mImageSave(out,"./test_perspective_morn.jpg");
    
    mImageRelease(in);
    mImageRelease(out);

    return 0;
}
#endif