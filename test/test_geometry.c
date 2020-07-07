/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_geometry.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_geometry.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_image.h"

int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test_geometry.jpg",src);
    
    MImage *dst = mImageCreate(1,src->height,src->width,NULL);
    void thresh(unsigned char *in,unsigned char *out,void *para)
    {
        int min=MIN(in[0],MIN(in[1],in[2]));
        int max=MAX(in[0],MAX(in[1],in[2]));
        int mean = (in[0]+in[1]+in[2])/3;
        out[0]=((mean>80)&&(max-min<50))?255:0;
    }
    mImageOperate(src,dst,thresh,NULL);
    mImageBinaryFilter(dst,dst,2,3);
    
    mImageSave(dst,"./test_geometry_bin.jpg");
    
    MSheet *sheet=mSheetCreate(DFLT,NULL,NULL);
    mImageBinaryEdge(dst,sheet,NULL);
    
    MList *polygon1 = mListCreate(sheet->col[0],sheet->data[0]);
    MList *polygon2 = mListCreate(sheet->col[1],sheet->data[1]);
    
    unsigned char color1[3] = {255,0,0};
    unsigned char color2[3] = {0,0,255};
    
    mEdgeBoundary(polygon1,polygon1,2);
    mEdgeBoundary(polygon2,polygon2,2);
    for(int i=0;i<polygon1->num;i++)
        mImageDrawPoint(src,src,polygon1->data[i],color1,4);
    for(int i=0;i<polygon2->num;i++)
        mImageDrawPoint(src,src,polygon2->data[i],color2,4);
    mImageSave(src,"./test_geometry_rst1.png");
    
    float area1 = mPolygonArea(polygon1);
    printf("area1 is %f\n",area1);
    float area2 = mPolygonArea(polygon2);
    printf("area2 is %f\n",area2);
    
    mShapeBounding(polygon1,polygon1);
    mImageDrawShape(src,src,polygon1,color1,2);
    mShapeBounding(polygon2,polygon2);
    mImageDrawShape(src,src,polygon2,color2,2);
    mImageSave(src,"./test_geometry_rst2.bmp");
    
    mImageRelease(src);
    mImageRelease(dst);
    mSheetRelease(sheet);
    mListRelease(polygon1);
    mListRelease(polygon2);
    
    return 0;
}



    