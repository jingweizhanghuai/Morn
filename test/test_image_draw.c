/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_image_draw.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_image_draw.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

int main()
{
    MImage *img = mImageCreate(3,200,1000,NULL);
    for(int j=0;j<img->height;j++)
    {
        memset(img->data[0][j],0,img->width*sizeof(unsigned char));
        memset(img->data[1][j],0,img->width*sizeof(unsigned char));
        memset(img->data[2][j],0,img->width*sizeof(unsigned char));
    }
    unsigned char color[3];
    
    color[0]=255;color[1]=0;color[2]=0;
    MImagePoint p1,p2;
    p1.x= 30;p1.y= 20;
    p2.x=210;p2.y=160;
    mImageDrawLine(img,img,&p1,&p2,color,3);
    
    color[0]=0;color[1]=255;color[2]=0;
    MImageRect rect;
    rect.x1 = 220;rect.y1 =  50;
    rect.x2 = 380;rect.y2 = 150;
    mImageDrawRect(img,img,&rect,color,3);
    
    color[0]=0;color[1]=0;color[2]=255;
    MList *polygon = mListCreate(DFLT,NULL);
    mPolygon(polygon,5,430,40,540,20,570,120,480,180,410,100);
    mImageDrawShape(img,img,polygon,color,3);
    mListRelease(polygon);
    
    color[0]=255;color[1]=0;color[2]=255;
    MImageCircle circle;
    mImageCircleSetup(&circle,700,100,80);
    mImageDrawCircle(img,img,&circle,color,3);
    
    float func(float x,void *para) {return (x-900)*(x-900)/50;}
    mImageDrawCurve(img,img,func,NULL,NULL,3);
    
    mBMPSave(img,"./test_draw.png");
    mImageRelease(img);
    return 0;
}
    