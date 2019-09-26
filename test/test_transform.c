/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_transform.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_transform.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"

int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test2.jpg",src);
    
    MImage *rotate = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageRotate(src,rotate,NULL,NULL,60);
    mImageSave(rotate,"./test_rotate.jpg");
    mImageRelease(rotate);
    
    MImagePoint ps[4];
    MImagePoint pd[4];
    ps[0].x=  0;ps[0].y= 0;ps[1].x= 0;ps[1].y=200;ps[2].x=100;ps[2].y=100;
    pd[0].x=-20;pd[0].y=10;pd[1].x=30;pd[1].y=180;pd[2].x= 90;pd[2].y=120;
    MImage *affine = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageAffineCorrection(src,affine,ps,pd);
    mImageSave(affine,"./test_affine.jpg");
    mImageRelease(affine);
    
    ps[0].x=  0;ps[0].y= 0;ps[1].x= 0;ps[1].y=500;ps[2].x=500;ps[2].y=500;ps[3].x=500;ps[3].y= 0;
    pd[0].x= 50;pd[0].y=-30;pd[1].x=-50;pd[1].y=480;pd[2].x=520;pd[2].y=420;pd[3].x=390;pd[3].y=60;
    MImage *perspective = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImagePerspectiveCorrection(src,perspective,ps,pd);
    mImageSave(perspective,"./test_perspective.jpg");
    mImageRelease(perspective);
    
    float x_transform(int x,int y,void *para) {return ( 0.0022*x*x+0.38*x+0.31*y-150);}
    float y_transform(int x,int y,void *para) {return (-0.0023*x*y+1.66*y+0.11*x-55);}
    MImage *transform = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageCoordinateTransform(src,transform,x_transform,y_transform,NULL);
    mImageSave(transform,"./test_transform.jpg");
    mImageRelease(transform);
    
    mImageRelease(src);
    
    return 0;
}
    