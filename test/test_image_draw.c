/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: gcc -O2 -fopenmp test_image_draw.c -o test_image_draw.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -ljpeg -lpng -lz

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
    mPoint(&p1,30,20);
    mPoint(&p2,210,160);
    mImageDrawLine(img,&p1,&p2,color,3);
    
    color[0]=0;color[1]=255;color[2]=0;
    MImageRect rect;
    mRect(&rect,220,50,380,150);
    mImageDrawRect(img,&rect,color,3);
    
    color[0]=0;color[1]=0;color[2]=255;
    MList *polygon = mListCreate(DFLT,NULL);
    mPolygon(polygon,430,40,540,20,570,120,480,180,410,100);
    mImageDrawShape(img,polygon,color,3);
    mListRelease(polygon);
    
    color[0]=255;color[1]=0;color[2]=255;
    MImageCircle circle;
    mCircle(&circle,700,100,80);
    mImageDrawCircle(img,&circle,color,3);

    float func(float x,float *para) {return (x-900)*(x-900)/50;}
    MImageCurve curve;
    mCurve(&curve,810,980,DFLT,func,NULL);
    mImageDrawCurve(img,&curve,3);
    
    mImageSave(img,"./test_draw.png");
    mImageRelease(img);
    return 0;
}