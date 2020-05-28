/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "morn_image.h"

/*
MHist *mHistCreate(int num,int *data)
{
    MHist *hist;
    
    hist = (MHist *)mMalloc(sizeof(MHist));
    
    if(num<0)
        num = 0;
    
    hist->num = num;
    hist->handle = NULL;
    
    if(num==0)
    {
        hist->data = NULL;
        hist->memory = NULL;
    }
    else if(!INVALID_POINTER(data))
    {
        hist->data = data;
        hist->memory = NULL;
    }
    else
    {
        hist->memory = mMemoryBlockCreate(num*sizeof(int));
        hist->data = (int *)(hist->memory->memory);
    }
    
    return hist;
}

void mHistRelease(MHist *hist)
{   
    mException(INVALID_POINTER(hist),"invalid input",EXIT);
    
    if(!INVALID_POINTER(hist->memory))
        mMemoryRelease(hist->memory);
    
    if(!INVALID_POINTER(hist->handle))
        HandleRelease(hist->handle);
    
    mFree(hist);
}

void mHistRedefine(MHist *hist,int num)
{
    mException(INVALID_POINTER(hist),"invalid input",EXIT);
    
    hist->num = num;
    if(hist->num>=num)
        return;
    
    if(INVALID_POINTER(hist->memory))
        hist->memory = mMemoryBlockCreate(num*sizeof(int));
    else
        mMemoryBlockRedefine(hist->memory,num*sizeof(int));
    
    hist->data = (int *)((hist->memory)->memory);
}
*/

void mImageHist(MImage*img,int cn,MArray *hist)
{
    mException(INVALID_IMAGE(img),EXIT,"invalid input");
    mException((cn>=(img->channel)),EXIT,"invalid input");

    int flag = 0xcc33b0d3;
    
    if(img->channel == 1) cn = 0;
    if(cn >= 0)
    {
        mArrayRedefine(hist,257,S32,hist->data);
        memset(hist->dataS32,0,256*sizeof(int));hist->dataS32[256]=flag;hist->num=256;
        unsigned char **data = img->data[cn];
        for(int j=ImageY1(img);j<ImageY2(img);j++)
            for(int i=ImageX1(img,j);i<ImageX2(img,j);i++)
                hist->dataS32[data[j][i]]++;
    }
    else if(img->channel == 3)
    {
        mArrayRedefine(hist,4097,S32,hist->data);
        memset(hist->dataS32,0,4096*sizeof(int));hist->dataS32[4096]=flag;hist->num=4096;
        for(int j=ImageY1(img);j<ImageY2(img);j++)
            for(int i=ImageX1(img,j);i<ImageX2(img,j);i++)
            {
                int n0=(img->data[0][j][i])>>4;int n1=(img->data[1][j][i])>>4;int n2=(img->data[2][j][i])>>4;
                hist->dataS32[(n2<<8)+(n1<<4)+n0]++;
            }
    }
    else if(img->channel == 4)
    {
        mArrayRedefine(hist,65537,S32,hist->data);
        memset(hist->dataS32,0,65536*sizeof(int));hist->dataS32[65536]=flag;hist->num=65536;
        for(int j=ImageY1(img);j<ImageY2(img);j++)
            for(int i=ImageX1(img,j);i<ImageX2(img,j);i++)
            {
                int n0=(img->data[0][j][i])>>4;int n1=(img->data[1][j][i])>>4;int n2=(img->data[2][j][i])>>4;int n3=(img->data[3][j][i])>>4;
                hist->dataS32[(n3<<12)+(n2<<8)+(n1<<4)+n0]++;
            }
    }
    else //(img->channel == 2)
    {
        mArrayRedefine(hist,1025,S32,hist->data);
        memset(hist->dataS32,0,1024*sizeof(int));hist->dataS32[1024]=flag;hist->num=1024;
        for(int j=ImageY1(img);j<ImageY2(img);j++)
            for(int i=ImageX1(img,j);i<ImageX2(img,j);i++)
            {
                int n0=(img->data[0][j][i])>>3;int n1=(img->data[1][j][i])>>3;
                hist->dataS32[(n1<<5)+n0]++;
            }
    }
}

int mImageHistData(MArray *hist,int c0,...)
{
    mException(INVALID_POINTER(hist),EXIT,"invalid input");
    mException((hist->num!=256)&&(hist->num!=1024)&&(hist->num!=4096)&&(hist->num!=65536),EXIT,"invalid input");
    int *data = hist->dataS32;
    if(hist->num==256)
    {
        mException((data[256]!=0xcc33b0d3),EXIT,"invalid input");
        return data[c0];
    }
    int rst;
    va_list para;va_start(para,c0);
    if(hist->num==1024)
    {
        mException((data[1024]!=0xcc33b0d3),EXIT,"invalid input");
        int c1=va_arg(para,int);
        if(((c0|c1)&0x1f)==0) rst = data[(c1<<2)+(c0>>5)];
        else
        {
            int b0=c0>>3;int b1=c1>>3;int d0=MIN(b0+1,31);int d1=MIN(b1+1,31);
            int w0=c0-(b0<<3);int w1=c1-(b1<<3);
            rst=((data[(b1<<5)+b0]*(8-w0)+data[(b1<<5)+d0]*w0)*(8-w1)
                +(data[(d1<<5)+b0]*(8-w0)+data[(d1<<5)+d0]*w0)*w1+32)/64;
        }
    }
    else if(hist->num==4096)
    {
        mException((data[4096]!=0xcc33b0d3),EXIT,"invalid input");
        int c1=va_arg(para,int);int c2=va_arg(para,int);
        if(((c0|c1|c2)&0x0f)==0) rst = data[(c2<<4)+c1+(c0>>4)];
        else
        {
            int b0=c0>>4;int b1=c1>>4;int b2=c2>>4;int d0=MIN(b0+1,15);int d1=MIN(b1+1,15);int d2=MIN(b2+1,15);
            int w0=c0-(b0<<4);int w1=c1-(b1<<4);int w2=c2-(b2<<4);
            rst=(((data[(b2<<8)+(b1<<4)+b0]*(16-w0)+data[(b2<<8)+(b1<<4)+d0]*w0)*(16-w1)
                 +(data[(b2<<8)+(d1<<4)+b0]*(16-w0)+data[(b2<<8)+(d1<<4)+d0]*w0)*w1)*(16-w2)
                +((data[(d2<<8)+(b1<<4)+b0]*(16-w0)+data[(d2<<8)+(b1<<4)+d0]*w0)*(16-w1)
                 +(data[(d2<<8)+(d1<<4)+b0]*(16-w0)+data[(d2<<8)+(d1<<4)+d0]*w0)*w1)*w2+2048)/4096;
        }
    }
    else //(hist->num==65536)
    {
        mException((data[65536]!=0xcc33b0d3),EXIT,"invalid input");
        int c1=va_arg(para,int);int c2=va_arg(para,int);int c3=va_arg(para,int);
        if(((c0|c1|c2|c3)&0x0f)==0) rst = data[(c3<<8)+(c2<<4)+c1+(c0>>4)];
        else
        {
            int b0=c0>>4;int b1=c1>>4;int b2=c2>>4;int b3=c3>>4;
            int d0=MIN(b0+1,15);int d1=MIN(b1+1,15);int d2=MIN(b2+1,15);int d3=MIN(b3+1,15);
            int w0=c0-(b0<<4);int w1=c1-(b1<<4);int w2=c2-(b2<<4);int w3=c3-(b3<<4);
            rst=((((data[(b3<<12)+(b2<<8)+(b1<<4)+b0]*(16-w0)+data[(b3<<12)+(b2<<8)+(b1<<4)+d0]*w0)*(16-w1)
                  +(data[(b3<<12)+(b2<<8)+(d1<<4)+b0]*(16-w0)+data[(b3<<12)+(b2<<8)+(d1<<4)+d0]*w0)*w1)*(16-w2)
                 +((data[(b3<<12)+(d2<<8)+(b1<<4)+b0]*(16-w0)+data[(b3<<12)+(d2<<8)+(b1<<4)+d0]*w0)*(16-w1)
                  +(data[(b3<<12)+(d2<<8)+(d1<<4)+b0]*(16-w0)+data[(b3<<12)+(d2<<8)+(d1<<4)+d0]*w0)*w1)*w2)*(16-w3)
                +(((data[(d3<<12)+(b2<<8)+(b1<<4)+b0]*(16-w0)+data[(d3<<12)+(b2<<8)+(b1<<4)+d0]*w0)*(16-w1)
                  +(data[(d3<<12)+(b2<<8)+(d1<<4)+b0]*(16-w0)+data[(d3<<12)+(b2<<8)+(d1<<4)+d0]*w0)*w1)*(16-w2)
                 +((data[(d3<<12)+(d2<<8)+(b1<<4)+b0]*(16-w0)+data[(d3<<12)+(d2<<8)+(b1<<4)+d0]*w0)*(16-w1)
                  +(data[(d3<<12)+(d2<<8)+(d1<<4)+b0]*(16-w0)+data[(d3<<12)+(d2<<8)+(d1<<4)+d0]*w0)*w1)*w2)*w3+32768)/65536;
        }
    }
    va_end(para);
    return rst;
}
    
void mHistNormlize(MArray *src,MArray *dst,int norm_data)
{
    int count;
    float k;
    int i;
    
    mException(INVALID_POINTER(src),EXIT,"invalid input");
    
    count = 0;
    for(i=0;i<src->num;i++)
        count = count + src->dataS32[i];
    
    k = ((float)norm_data)/((float)count);
    
    mArrayRedefine(dst,src->num,S32,dst->data);
    
    for(i=0;i<src->num;i++)
        dst->dataS32[i] = (int)(((float)src->dataS32[i]) * k + 0.5f);
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
