#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"

void mBinaryImageResize(MImage *src,MImage *dst,int height,int width,int type);

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
    struct HandleImageResize *handle = info;
    if(handle->lx != NULL) mFree(handle->lx);
    if(handle->wx != NULL) mFree(handle->wx);
    if(handle->ly != NULL) mFree(handle->ly);
    if(handle->wy != NULL) mFree(handle->wy);
}
void mImageResize(MImage *src,MImage *dst,int height,int width,int type)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    if(type<=0) type = 0;
    else if((type&0xF0) == MORN_RESIZE_NEAREST)
    {
        mBinaryImageResize(src,dst,height,width,type);
        return;
    }
    
    MImage *p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
    {
        if((height>0)&&(width>0)) NULL;
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        dst = mImageCreate(src->cn,height,width,NULL);
    }
    else
    {
        if(height <= 0) height = dst->height;
        if(width  <= 0) width = dst->width;
        
        if((height>0)&&(width>0)) NULL;
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        mImageRedefine(dst,src->cn,height,width);
    }

    MHandle *hdl; ObjectHandle(src,ImageResize,hdl);
    struct HandleImageResize *handle = hdl->handle;
    if((hdl->valid == 0)||(handle->height != height)||(handle->width != width)||(handle->type != type))
    {
        float kx = ((float)(src->width )-1.0f)/((float)width -1.0f);
        float ky = ((float)(src->height)-1.0f)/((float)height-1.0f);
             if(type == MORN_RESIZE_MINUNIFORM) {kx = MIN(kx,ky); ky = kx;}
        else if(type == MORN_RESIZE_MAXUNIFORM) {kx = MAX(kx,ky); ky = kx;}
        float scx = ((float)(src->width)-1.0f)/2.0f; float scy = ((float)(src->height)-1.0f)/2.0f;
        float dcx = ((float)      width -1.0f)/2.0f; float dcy = ((float)      height -1.0f)/2.0f;
    
        handle->type = type;
        
        if(handle->width <width)
        {
            if(handle->lx != NULL) mFree(handle->lx);
            if(handle->wx != NULL) mFree(handle->wx);
            handle->lx = mMalloc(width * sizeof(int));
            handle->wx = mMalloc(width * sizeof(unsigned char));
        }
        handle->width = width;
        
        if(handle->height < height)
        {
            if(handle->ly != NULL) mFree(handle->ly);
            if(handle->wy != NULL) mFree(handle->wy);
            handle->ly = mMalloc(height * sizeof(int));
            handle->wy = mMalloc(height * sizeof(unsigned char));
        }
        handle->height = height;
        
        for(int i=0;i<width;i++)
        {
            float x = ((float)i-dcx)*kx+scx;
            handle->lx[i] = (int)x;
            handle->wx[i] = (x>0)?(128 - (unsigned char)((x-(float)(handle->lx[i]))*128.0f)):0;
        }
        
        for(int j=0;j<height;j++)
        {
            float y = ((float)j-dcy)*ky+scy;
            handle->ly[j] = (int)y;
            handle->wy[j] = (y>0)?(128 - (unsigned char)((y-(float)(handle->ly[j]))*128.0f)):0;
        }
        
        hdl->valid = 1;
    }
    int *lx = handle->lx; unsigned char *wx = handle->wx;
    int *ly = handle->ly; unsigned char *wy = handle->wy;
    
    int j;
    #pragma omp parallel for
    for(j=0;j<height;j++)
    {
        int y1 = ly[j];int y2 = y1+1;
        if((y1<0)||(y2>src->height))
        {
            for(int cn=0;cn<src->cn;cn++)
                memset(dst->data[cn][j],0,width*sizeof(unsigned char));
            continue;
        }
            
        for(int i=0;i<width;i++)
        {
            int x1 = lx[i];int x2 = x1+1;
            
            if((x1<0)||(x2>src->width))
            {
                for(int cn=0;cn<src->cn;cn++)
                    dst->data[cn][j][i] = 0;
                continue;
            }
            
            unsigned char wx1 = wx[i];unsigned char wx2 = 128-wx1;
            unsigned char wy1 = wy[j];unsigned char wy2 = 128-wy1;
            
            for(int cn=0;cn<src->cn;cn++)
                dst->data[cn][j][i] =((src->data[cn][y1][x1]*wx1+src->data[cn][y1][x2]*wx2)*wy1
                                     +(src->data[cn][y2][x1]*wx1+src->data[cn][y2][x2]*wx2)*wy2)/16384;
            
        }
    }
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

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
    struct HandleBinaryImageResize *handle = info;
    if(handle->lx != NULL) mFree(handle->lx);
    if(handle->ly != NULL) mFree(handle->ly);
}
void mBinaryImageResize(MImage *src,MImage *dst,int height,int width,int type)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    if(type <=0) type = MORN_RESIZE_UNUNIFORM;
    else type = type&0x0F;
    
    MImage *p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
    {
        if((height>0)&&(width>0)) NULL;
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        dst = mImageCreate(src->cn,height,width,NULL);
    }
    else
    {
        if(height <= 0) height = dst->height;
        if(width  <= 0) width = dst->width;
        
        if((height>0)&&(width>0)) NULL;
        else if((height<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            height = (src->height)*width/(src->width);
        else if((width<=0)&&(type != MORN_RESIZE_UNUNIFORM))
            width = (src->width)*height/(src->height);
        else
            mException(1,EXIT,"invalid input");
        mImageRedefine(dst,src->cn,height,width);
    }

    MHandle *hdl; ObjectHandle(src,BinaryImageResize,hdl);
    struct HandleBinaryImageResize *handle = hdl->handle;
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
            handle->lx = mMalloc(width * sizeof(int));
        }
        handle->width = width;
        
        if(handle->height < height)
        {
            if(handle->ly != NULL) mFree(handle->ly);
            handle->ly = mMalloc(height * sizeof(int));
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
        
        hdl->valid = 1;
    }
    int *lx = handle->lx;
    int *ly = handle->ly;
    
    int j;
    #pragma omp parallel for
    for(j=0;j<height;j++)
    {
        if((ly[j]<0)||(ly[j]>=src->height)) 
        {
            for(int cn=0;cn<src->cn;cn++)
                memset(dst->data[cn][j],0,width*sizeof(unsigned char));
            continue;
        }
        for(int i=0;i<width;i++)
        {
            if((lx[i]<0)||(lx[i]>=src->width))
            {
                for(int cn=0;cn<src->cn;cn++)
                    dst->data[cn][j][i] = 0;
                continue;
            }
            for(int cn=0;cn<src->cn;cn++)
                dst->data[cn][j][i] =src->data[cn][ly[j]][lx[i]];
        }
    }

    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

#define MORN_WITH_INTERPOLATE         0
#define MORN_WITHOUT_INTERPOLATE (1<<4)





