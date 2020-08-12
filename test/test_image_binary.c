/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// gcc -O2 -fopenmp test_image_binary.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_image_binary.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

int main()
{
    MImage *bin = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("D:/Morn_video/data5/test0_bin.png",bin);
    
    // MImage *bin = mImageCreate(1,src->height,src->width,NULL);
    // void operate(unsigned char *in,unsigned char *out,void *para)
    // {
    //     out[0]=(MIN(MIN(in[0],in[1]),in[2])<128)?255:0;
    // }
    // mImageOperate(src,bin,operate,NULL);
    // mImageSave(bin,"../doc/test_binary_bin.png");
    // mImageRelease(src);
    
    MImage *out = mImageCreate(1,bin->height,bin->width,NULL);

    MSheet *sheet =mSheetCreate(DFLT,NULL,NULL);
    mImageBinaryEdge(bin,sheet,NULL);
    for(int h=0;h<out->height;h++) memset(out->data[0][h],0,out->width*sizeof(unsigned char));
    for(int j=0;j<sheet->row;j++)for(int i=0;i<sheet->col[j];i++)
    {
        MImagePoint *p = (MImagePoint *)(sheet->data[j][i]);
        out->data[0][(int)(p->y)][(int)(p->x)]=255;
    }
    mImageSave(out,"../doc/test_binary_edge.png");
    mSheetRelease(sheet);

    /*
    MList *list = mListCreate(DFLT,NULL);
    for(int j=40;;j+=40)
    {
        if(j>255) j=40;
        mImageBinaryArea(bin,list,NULL);
        if(list->num==0) break;
        for(int i=0;i<list->num;i++)
        {
            MImagePoint *p=(MImagePoint *)(list->data[i]);
            out->data[0][(int)(p->y)][(int)(p->x)]=j;
        }
    }
    mImageSave(out,"../doc/test_binary_area.png");
    mListRelease(list);

    mImageCut(bin,bin,0,1000,600,750);
    mImageSave(bin,"../doc/test_binary_bin2.png");
    mImageBinaryDenoise(bin,out,200);
    mImageSave(out,"../doc/test_binary_denoise.png");
    */
    
    mImageRelease(bin);
    mImageRelease(out);
    return 0;
}