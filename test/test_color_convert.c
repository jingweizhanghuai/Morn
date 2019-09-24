/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_color_convert.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_color_convert.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    MImage *img = mImageCreate(1,DFLT,DFLT,NULL);
    mImageLoad("../doc/test2.jpg",src);
    
    MImage *hsv = mImageCreate(3,src->height,src->width,NULL);
    mImageRGBToHSV(src,hsv);
    
    mImageRedefine(img,1,src->height,src->width,&(hsv->data[0]));
    mImageSave(img,"./test_color_convert_hsv_h.png");
    
    mImageRedefine(img,1,src->height,src->width,&(hsv->data[1]));
    mImageSave(img,"./test_color_convert_hsv_s.png");
    
    mImageRedefine(img,1,src->height,src->width,&(hsv->data[2]));
    mImageSave(img,"./test_color_convert_hsv_v.png");
    
    MImage *rgb1 = mImageCreate(3,src->height,src->width,NULL);
    mImageHSVToRGB(hsv,rgb1);
    mImageSave(rgb1,"./test_ColorConvert_rgb1.png");
    
    MImage *yuv = mImageCreate(3,src->height,src->width,NULL);
    mImageRGBToYUV(src,yuv);
    
    mImageRedefine(img,1,src->height,src->width,&(yuv->data[0]));
    mImageSave(img,"./test_color_convert_yuv_y.png");
    
    mImageRedefine(img,1,src->height,src->width,&(yuv->data[1]));
    mImageSave(img,"./test_color_convert_yuv_u.png");
    
    mImageRedefine(img,1,src->height,src->width,&(yuv->data[2]));
    mImageSave(img,"./test_color_convert_yuv_v.png");
    
    MImage *rgb2 = mImageCreate(3,src->height,src->width,NULL);
    mImageYUVToRGB(yuv,rgb2);
    mImageSave(rgb2,"./test_ColorConvert_rgb2.png");
    
    mImageRelease(src);
    mImageRelease(img);
    mImageRelease(hsv);
    mImageRelease(yuv);
    mImageRelease(rgb1);
    mImageRelease(rgb2);
    
    return 0;
}
    