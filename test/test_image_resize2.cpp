/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#ifdef USE_OPENCV
// build: g++ -O2 -fopenmp test_image_resize2.cpp -o test_image_resize2_opencv.exe -DUSE_OPENCV -I C:\ProgramFiles\OpenCV\include -I ..\include -L C:\ProgramFiles\OpenCV\lib\mingw -lopencv_world349 -llibjpeg-turbo -llibjasper -lIlmImf -llibtiff  -llibwebp -llibpng -lzlib -L ..\lib\x64_mingw -lmorn
// g++ -O2 -fopenmp test_image_resize2.cpp -o test_image_resize2_opencv.exe -DUSE_OPENCV -I ../include/ -L ../lib/x64_gnu/ -lmorn -lopencv_imgproc -lopencv_imgcodecs -lopencv_core
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "morn_util.h"

using namespace cv;
int main()
{
    Mat src = imread("../doc/test.jpg"); 
    Mat dst;
    
    mTimerBegin("OpenCV");
    for(int i=0;i<1000;i++)
        resize(src,dst,Size(256,256));
    mTimerEnd("OpenCV");
    
    imwrite("./test_resize_cv.jpg",dst);

    return 0;
}
#endif

#ifdef USE_MORN
// build: g++ -O2 -fopenmp test_image_resize2.cpp -o test_image_resize2_morn.exe -DUSE_MORN -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -ljpeg -lpng -lz
// g++ -O2 -fopenmp test_image_resize2.cpp -o test_image_resize2_morn.exe -DUSE_MORN -I ../include/ -L ../lib/x64_gnu/ -lmorn -ljpeg -lpng -lz -lm
#include "morn_image.h"

int main()
{
    MImage *in = mImageCreate();
    mImageLoad(in,"../doc/test.jpg");
    MImage *out = mImageCreate(in->channel,256,256);
    
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mImageResize(in,out);
    mTimerEnd("Morn");
    
    mImageSave(out,"./test_resize_morn.jpg");
    
    mImageRelease(in);
    mImageRelease(out);

    return 0;
}
#endif