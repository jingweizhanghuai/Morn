/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译：gcc -O2 -fopenmp test_image_resize.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_image_resize.exe

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