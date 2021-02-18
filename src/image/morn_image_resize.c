/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_image.h"

struct HandleBinaryImageResize
{
    int height;
    int width;
    int type;
    
    int *lx;
    int *ly;
};
#define HASH_BinaryImageResize 0x3b7f3813
void endBinaryImageResize(void *info)
{
    struct HandleBinaryImageResize *handle = (struct HandleBinaryImageResize *)info;
    if(handle->lx != NULL) mFree(handle->lx);
    if(handle->ly != NULL) mFree(handle->ly);
}
void mBinaryImageResize(MImage *src,MImage *dst,int height,int width,int type)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    type = type|0xFC;
    
    MImage *p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
    {
        if((height>0)&&(width>0));
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        dst = mImageCreate(src->channel,height,width,NULL);
    }
    else
    {
        if(height <= 0) height = dst->height;
        if(width  <= 0) width = dst->width;
        
        if((height>0)&&(width>0));
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        mImageRedefine(dst,src->channel,height,width,dst->data);
    }

    MHandle *hdl=mHandle(src,BinaryImageResize);
    struct HandleBinaryImageResize *handle = (struct HandleBinaryImageResize *)hdl->handle;
    if((hdl->valid == 0)||(handle->height != height)||(handle->width != width)||(handle->type != type))
    {
        float kx = ((float)(src->width ))/((float)width );
        float ky = ((float)(src->height))/((float)height);
             if(type == MORN_RESIZE_MINUNIFORM) {kx = MIN(kx,ky); ky = kx;}
        else if(type == MORN_RESIZE_MAXUNIFORM) {kx = MAX(kx,ky); ky = kx;}
        float scx = ((float)(src->width))/2.0f; float scy = ((float)(src->height))/2.0f;
        float dcx = ((float)      width )/2.0f; float dcy = ((float)      height )/2.0f;
    
        handle->type = type;
        
        if(handle->width <width)
        {
            if(handle->lx != NULL) mFree(handle->lx);
            handle->lx = (int *)mMalloc(width * sizeof(int));
        }
        handle->width = width;
        
        if(handle->height < height)
        {
            if(handle->ly != NULL) mFree(handle->ly);
            handle->ly = (int *)mMalloc(height * sizeof(int));
        }
        handle->height = height;
        
        for(int i=0;i<width;i++)
        {
            float x = ((float)i-dcx)*kx+scx;
            handle->lx[i] = (int)(x+0.5);
        }
        
        for(int j=0;j<height;j++)
        {
            float y = ((float)j-dcy)*ky+scy;
            handle->ly[j] = (int)(y+0.5);
        }
    }
    int *lx = handle->lx;
    int *ly = handle->ly;
    
    int j;
    #pragma omp parallel for
    for(j=0;j<height;j++)
        for(int i=0;i<width;i++)
            for(int cn=0;cn<src->channel;cn++)
                dst->data[cn][j][i] =src->data[cn][ly[j]][lx[i]];
   
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
    hdl->valid = 1;
}

struct HandleImageResize
{
    int height;
    int width;
    int type;
    
    int *lx;
    unsigned char *wx;
    int *ly;
    unsigned char *wy;
};
#define HASH_ImageResize 0x56db84c
void endImageResize(void *info)
{
    struct HandleImageResize *handle = (struct HandleImageResize *)info;
    if(handle->lx != NULL) mFree(handle->lx);
    if(handle->wx != NULL) mFree(handle->wx);
    if(handle->ly != NULL) mFree(handle->ly);
    if(handle->wy != NULL) mFree(handle->wy);
}
void m_ImageResize(MImage *src,MImage *dst,int height,int width,int type)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    if((type|MORN_NEAREST)==MORN_NEAREST) {mBinaryImageResize(src,dst,height,width,type);return;}
   
    MImage *p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
    {
        if((height>0)&&(width>0));
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        dst = mImageCreate(src->channel,height,width,NULL);
    }
    else
    {
        if(height <= 0) height = dst->height;
        if(width  <= 0) width = dst->width;
        
        if((height>0)&&(width>0));
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        mImageRedefine(dst,src->channel,height,width,dst->data);
    }

    MHandle *hdl=mHandle(src,ImageResize);
    struct HandleImageResize *handle = (struct HandleImageResize *)(hdl->handle);
    if((hdl->valid == 0)||(handle->height != height)||(handle->width != width)||(handle->type != type))
    {
        float kx = ((float)(src->width ))/((float)width );
        float ky = ((float)(src->height))/((float)height);
             if(type == MORN_RESIZE_MINUNIFORM) {kx = MIN(kx,ky); ky = kx;}
        else if(type == MORN_RESIZE_MAXUNIFORM) {kx = MAX(kx,ky); ky = kx;}
        float scx = ((float)(src->width))/2.0f; float scy = ((float)(src->height))/2.0f;
        float dcx = ((float)      width )/2.0f; float dcy = ((float)      height )/2.0f;
    
        handle->type = type;
        
        if(handle->width <width)
        {
            if(handle->lx != NULL) mFree(handle->lx);
            if(handle->wx != NULL) mFree(handle->wx);
            handle->lx = (          int *)mMalloc(width * sizeof(int));
            handle->wx = (unsigned char *)mMalloc(width * sizeof(unsigned char));
        }
        handle->width = width;
        
        if(handle->height < height)
        {
            if(handle->ly != NULL) mFree(handle->ly);
            if(handle->wy != NULL) mFree(handle->wy);
            handle->ly = (          int *)mMalloc(height * sizeof(int));
            handle->wy = (unsigned char *)mMalloc(height * sizeof(unsigned char));
        }
        handle->height = height;
        
        for(int i=0;i<width;i++)
        {
            float x = ((float)i-dcx)*kx+scx;
            handle->lx[i] = floor(x);
            handle->wx[i] = 128 - (unsigned char)((x-(float)(handle->lx[i]))*128.0f);
        }
        
        for(int j=0;j<height;j++)
        {
            float y = ((float)j-dcy)*ky+scy;
            handle->ly[j] = floor(y);
            handle->wy[j] = 128 - (unsigned char)((y-(float)(handle->ly[j]))*128.0f);
        }
    }
    int *lx = handle->lx; unsigned char *wx = handle->wx;
    int *ly = handle->ly; unsigned char *wy = handle->wy;
   
    int j;
    #pragma omp parallel for
    // for(j=0;j<height;j++)for(int i=0;i<width;i++)
    for(j=ImageY1(dst);j<ImageY2(dst);j++)for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
    {
        if((lx[i]<0)||(lx[i]>src->width-1)||(ly[j]<0)||(ly[j]>src->height-1))
        {
            for(int cn=0;cn<src->channel;cn++)
                dst->data[cn][j][i] = 0.0f;
        }
        else
        {
            int x1 = lx[i];int x2 = x1+1;
            int y1 = ly[j];int y2 = y1+1;
            unsigned char wx1 = wx[i];unsigned char wx2 = 128-wx1;
            unsigned char wy1 = wy[j];unsigned char wy2 = 128-wy1;
            for(int cn=0;cn<src->channel;cn++)
            {
                dst->data[cn][j][i] =((src->data[cn][y1][x1]*wx1+src->data[cn][y1][x2]*wx2)*wy1
                                     +(src->data[cn][y2][x1]*wx1+src->data[cn][y2][x2]*wx2)*wy2)/16384;
            }
        }
    }
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
    hdl->valid = 1;
}







