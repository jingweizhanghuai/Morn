/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
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
    if(p!=dst) {mImageExchange(src,dst); mImageRelease(dst);}
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
