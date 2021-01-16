/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: gcc -O2 -fopenmp test_image_resize.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_image_resize.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"

int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test.jpg",src);
    
    MImage *dst = mImageCreate(src->channel,512,512,NULL);
    
    mImageResize(src,dst,DFLT,DFLT,DFLT);
    mImageSave(dst,"./test_resize1.jpg");
    
    mImageResize(src,dst,DFLT,DFLT,MORN_RESIZE_MINUNIFORM);
    mImageSave(dst,"./test_resize2.jpg");
    
    mImageResize(src,dst,DFLT,DFLT,MORN_RESIZE_MAXUNIFORM);
    mImageSave(dst,"./test_resize3.jpg");
    
    mImageResize(src,dst,DFLT,DFLT,MORN_NEAREST);
    mImageSave(dst,"./test_resize4.jpg");
    
    mImageResize(src,dst,DFLT,DFLT,MORN_NEAREST|MORN_RESIZE_MINUNIFORM);
    mImageSave(dst,"./test_resize5.jpg");
    
    mImageResize(src,dst,DFLT,DFLT,MORN_NEAREST|MORN_RESIZE_MAXUNIFORM);
    mImageSave(dst,"./test_resize6.jpg");
    
    mImageRelease(src);
    mImageRelease(dst);
}