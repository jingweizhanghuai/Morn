/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"
#include "morn_image_caculate.h"

void mImageDilation(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) {r=1;} if(stride <=0) {stride=1;}
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
   
    
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define ImageDilation(X,Y) {\
            unsigned char Max = 0;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                    if(sdata[M][N] > Max)\
                        Max = sdata[M][N];\
            ddata[Y][X] = Max;\
        }
        
        mImageRegion(src,r,ImageDilation);
    }
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageErosion(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) {r=1;} if(stride <=0) {stride=1;}
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define ImageErosion(X,Y) {\
            unsigned char Min = 255;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                    if(sdata[M][N] < Min)\
                        Min = sdata[M][N];\
            ddata[Y][X] = Min;\
        }
        
        mImageRegion(src,r,ImageErosion);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageMorphologySmooth(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) {r=1;} if(stride <=0) {stride=1;}
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    int num = (r+r+1)/stride;num=num*num;
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define MorphologySmooth(X,Y) {\
            unsigned char Min = 255;\
            unsigned char Max = 0;\
            int Sum = 0;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                {\
                    Sum += sdata[M][N];\
                    Min = MIN(sdata[M][N],Min);\
                    Max = MIN(sdata[M][N],Min);\
                }\
            ddata[Y][X]=(Sum/num > sdata[Y][X])?Max:Min;\
        }
        
        mImageRegion(src,r,MorphologySmooth);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageMorphologySharp(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) {r=1;} if(stride <=0) {stride=1;}
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    int num = (r+r+1)/stride;num=num*num;
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define MorphologySharp(X,Y) {\
            unsigned char Min = 255;\
            unsigned char Max = 0;\
            int Sum = 0;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                {\
                    Sum += sdata[M][N];\
                    Min = MIN(sdata[M][N],Min);\
                    Max = MIN(sdata[M][N],Min);\
                }\
            ddata[Y][X]=(Sum/num > sdata[Y][X])?Min:Max;\
        }
        
        mImageRegion(src,r,MorphologySharp);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}
