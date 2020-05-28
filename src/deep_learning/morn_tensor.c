/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_tensor.h"

struct HandleTensorCreate
{
    MTensor *tns;
    int batch;
    int size;

    int dev;
    float **data;
    MMemory *memory;

    float **backup_data;
    MMemory *backup_memory;
};
#define HASH_TensorCreate 0x6b6cf658
void endTensorCreate(void *info)
{
    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)info;
    mException((handle->tns ==NULL),EXIT,"invalid tensor");
    
    if(!INVALID_POINTER(handle->data)) mFree(handle->data);
    if(!INVALID_POINTER(handle->memory)) mMemoryRelease(handle->memory);

    if(!INVALID_POINTER(handle->backup_data)) mFree(handle->backup_data);
    if(!INVALID_POINTER(handle->backup_memory)) mMemoryRelease(handle->backup_memory);
    
    mFree(handle->tns);
}

MTensor *TensorCreate(int batch,int channel,int height,int width,float **data,int dev)
{
    if(batch  <0) batch  = 0;
    if(channel<0) channel= 0;
    if(height <0) height = 0;
    if(width  <0) width  = 0;
    if(dev    <0) dev    = MORN_HOST_CPU;
    
    MTensor *tns = (MTensor *)mMalloc(sizeof(MTensor));
    memset(tns,0,sizeof(MTensor));
    
    tns->batch  = batch;
    tns->channel= channel;
    tns->height = height;
    tns->width  = width;
    tns->dev    = dev;

    tns->handle = mHandleCreate();
    MHandle *hdl=mHandle(tns,TensorCreate);
    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(hdl->handle);
    handle->tns = tns;
    
    int size = channel*height*width;
    if((batch==0)||(size == 0))
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        return tns;
    }
    size = size+8;
    
    handle->batch = batch;
    handle->data = (float **)mMalloc(batch*sizeof(float *));
    tns->data = handle->data;
    if(!INVALID_POINTER(data)) 
    {
        handle->size = 0;
        memcpy(handle->data,data,batch*sizeof(float *));
        return tns;
    }
    
    handle->size= size;
    handle->dev = dev;
    handle->memory = mMemoryCreate(batch,size*sizeof(float),dev);
    for(int i=0;i<batch;i++) handle->data[i] = (float *)(handle->memory->data[i]);

    return tns;
}
    
void mTensorRelease(MTensor *tns)
{
    mException(INVALID_POINTER(tns),EXIT,"invalid input");
    
    if(!INVALID_POINTER(tns->handle))
        mHandleRelease(tns->handle);
}

void TensorRedefine(MTensor *tns,int batch,int channel,int height,int width,float **data,int dev)
{
    mException((INVALID_POINTER(tns)),EXIT,"invalid input");
    
    if(batch  <= 0)  batch  = tns->batch;
    if(channel<= 0)  channel= tns->channel;
    if(height <= 0)  height = tns->height;
    if(width  <= 0)  width  = tns->width;
    if(dev    <  0)  dev    = tns->dev;
    
    if((batch!=tns->batch)||(channel!=tns->channel)||(height!=tns->height)||(width!=tns->width)||(dev!=tns->dev))
        mHandleReset(tns->handle);

    int size = channel*height*width;
    int same_size = ((batch<=tns->batch)&&(size<=tns->height*tns->width*tns->channel));
    int reuse = (data==tns->data);
    mException(reuse&&(dev!=tns->dev),EXIT,"invalid input data device");
    int flag = (tns->batch)&&(tns->channel)&&(tns->height)&&(tns->width);
    
    tns->batch  = batch;
    tns->height = height;
    tns->width  = width;
    tns->channel= channel;
    
    if(same_size&&reuse) return;
    tns->dev    = dev;
    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(((MHandle *)(tns->handle->data[0]))->handle);
    mException(reuse&&(dev!=handle->dev),EXIT,"invalid input memory device");
    if(same_size&&(INVALID_POINTER(data))&&(handle->size >0)) return;
    mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");

    handle->size = 0;
    if((batch == 0)||(size == 0)) 
    {
        mException((!INVALID_POINTER(data))&&(!reuse),EXIT,"invalid input");
        tns->data=NULL;
        return;
    }

    if(reuse) data=NULL;
       
    size = size +8;

    if(batch> handle->batch)
    {
        if(handle->data != NULL) mFree(handle->data);
        handle->data=NULL;
    }
    if(handle->data==NULL)
    {
        handle->data = (float **)mMalloc(batch*sizeof(float *));
        handle->batch = batch;
    }
    tns->data = handle->data;
    
    if(!INVALID_POINTER(data)) 
    {
        memcpy(handle->data,data,batch*sizeof(float *));
        if(!INVALID_POINTER(handle->backup_data)) mFree(handle->backup_data);
        if(!INVALID_POINTER(handle->backup_memory)) mMemoryRelease(handle->backup_memory);
        return;
    }
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(batch,size*sizeof(float),dev);
    else mMemoryRedefine(handle->memory,batch,size*sizeof(float),dev);
    for(int i=0;i<batch;i++) handle->data[i]=(float *)(handle->memory->data[i]);
    handle->size = size;
    handle->dev  = dev;
}

float **mTensorBackup(MTensor *tns,int batch,int cn,int height,int width)
{
    if(batch <=0) batch =tns->batch;
    if(cn    <=0) cn    =tns->channel;
    if(height<=0) height=tns->height;
    if(width <=0) width =tns->width;

    int size = cn*height*width;

    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(((MHandle *)(tns->handle->data[0]))->handle);
    if(handle->backup_data!=NULL) mFree(handle->backup_data);
    handle->backup_data = (float **)mMalloc(batch*sizeof(float *));

    if(handle->backup_memory == NULL) handle->backup_memory = mMemoryCreate(batch,size*sizeof(float),tns->dev);
    else mMemoryRedefine(handle->backup_memory,batch,size*sizeof(float),tns->dev);
    for(int i=0;i<batch;i++) handle->backup_data[i] = (float *)(handle->backup_memory->data[i]);
    
    return (handle->backup_data);
}

void MemCopy(void *dst,int dst_dev,void *src,int src_dev,int size);
void mTensorCopy(MTensor *src,MTensor *dst,int dev)
{
    mException(INVALID_POINTER(src),EXIT,"invalid input source tensor");
    mException(INVALID_POINTER(dst)&&(dev<0),EXIT,"invalid input device");
    if(dev<0) dev=dst->dev;

    float **dst_data;
    int flag = (!INVALID_POINTER(dst))||(dst==src);
    if(flag) {if(dev==src->dev){return;} dst_data=mTensorBackup(src,DFLT,DFLT,DFLT,DFLT);}
    else     {if(dev!=dst->dev){mTensorRedefine(dst,DFLT,DFLT,DFLT,DFLT,NULL,dev);} dst_data=dst->data;}
    
    int size = src->channel*src->height*src->width;
    for(int i=0;i<src->batch;i++)
        MemCopy(dst_data[i],dev,src->data[i],src->dev,size*sizeof(float));
    
    if(flag) mTensorRedefine(src,DFLT,DFLT,DFLT,DFLT,dst_data,dev);
}

/*
void mTensorAdd(MTensor *src1,MTensor *src2,MTensor *dst)
{
    int i;
    mException((INVALID_TENSOR(src1)||INVALID_TENSOR(src2)),EXIT,"invalid input source");
    
    int batch = src1->batch;
    mException((src2->batch!=batch)&&(src2->batch!=1),EXIT,"invalid input source");
    mException((batch>1)&&(src2->batch==1)&&(dst==src2),EXIT,"invalid input");
    
    int channel = src1->channel;
    int height = src1->height;
    int width = src1->width;
    mException((src2->channel!=channel)||(src2->height!=height)||(src2->width!=width),EXIT,"invalid input source");

    int size = channel*height*width;
    
    if(dst==NULL) dst = src1;
    if((dst!=src1)&&(dst!=src2)) mTensorRedefine(dst,batch,channel,height,width,dst->data);
    
    for(int b=0;b<batch;b++)
    {
        float *data1 = src1->data[b];
        float *data2 = (src2->batch>1)?src2->data[b]:src2->data[0];
        float *data  = dst ->data[b];
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
            data[i] = data1[i]+data2[i];
    }
}

void mTensorSub(MTensor *src1,MTensor *src2,MTensor *dst)
{
    int i;
    mException((INVALID_TENSOR(src1)||INVALID_TENSOR(src2)),EXIT,"invalid input source");
    
    int batch = src1->batch;
    mException((src2->batch!=batch)&&(src2->batch!=1),EXIT,"invalid input source");
    mException((batch>1)&&(src2->batch==1)&&(dst==src2),EXIT,"invalid input");
    
    int channel = src1->channel;
    int height = src1->height;
    int width = src1->width;
    mException((src2->channel!=channel)||(src2->height!=height)||(src2->width!=width),EXIT,"invalid input source");

    int size = channel*height*width;
    
    if(dst==NULL) dst = src1;
    if((dst!=src1)&&(dst!=src2)) mTensorRedefine(dst,batch,channel,height,width,dst->data);
    
    for(int b=0;b<batch;b++)
    {
        float *data1 = src1->data[b];
        float *data2 = (src2->batch>1)?src2->data[b]:src2->data[0];
        float *data  = dst ->data[b];
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
            data[i] = data1[i]-data2[i];
    }
}
        
void mTensorScalarMul(MTensor *src1,MTensor *src2,MTensor *dst)
{
    int i;
    mException((INVALID_TENSOR(src1)||INVALID_TENSOR(src2)),EXIT,"invalid input source");
    
    int batch = src1->batch;
    mException((src2->batch!=batch)&&(src2->batch!=1),EXIT,"invalid input source");
    mException((batch>1)&&(src2->batch==1)&&(dst==src2),EXIT,"invalid input");
    
    int channel = src1->channel;
    int height = src1->height;
    int width = src1->width;
    mException((src2->channel!=channel)||(src2->height!=height)||(src2->width!=width),EXIT,"invalid input source");

    int size = channel*height*width;
    
    if(dst==NULL) dst = src1;
    if((dst!=src1)&&(dst!=src2)) mTensorRedefine(dst,batch,channel,height,width,dst->data);
    
    for(int b=0;b<batch;b++)
    {
        float *data1 = src1->data[b];
        float *data2 = (src2->batch>1)?src2->data[b]:src2->data[0];
        float *data  = dst ->data[b];
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
            data[i] = data1[i]*data2[i];
    }
}
   
void mTensorScalarDiv(MTensor *src1,MTensor *src2,MTensor *dst)
{
    int i;
    mException((INVALID_TENSOR(src1)||INVALID_TENSOR(src2)),EXIT,"invalid input source");
    
    int batch = src1->batch;
    mException((src2->batch!=batch)&&(src2->batch!=1),EXIT,"invalid input source");
    mException((batch>1)&&(src2->batch==1)&&(dst==src2),EXIT,"invalid input");
    
    int channel = src1->channel;
    int height = src1->height;
    int width = src1->width;
    mException((src2->channel!=channel)||(src2->height!=height)||(src2->width!=width),EXIT,"invalid input source");

    int size = channel*height*width;
    
    if(dst==NULL) dst = src1;
    if((dst!=src1)&&(dst!=src2)) mTensorRedefine(dst,batch,channel,height,width,dst->data);
    
    for(int b=0;b<batch;b++)
    {
        float *data1 = src1->data[b];
        float *data2 = (src2->batch>1)?src2->data[b]:src2->data[0];
        float *data  = dst ->data[b];
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
            data[i] = data1[i]/data2[i];
    }
}
*/

void mTensorOperate(MTensor *src,MTensor *dst, float (*func)(float))
{
    int i;
    mException(INVALID_TENSOR(src),EXIT,"invalid input source");
    
    if(dst==NULL) dst = src;
    if(dst!=src ) mTensorRedefine(dst,src->batch,src->channel,src->height,src->width,dst->data);
    
    int size = src->channel*src->height*src->width;
    
    for(int b=0;b<src->batch;b++)
    {
        #pragma omp parallel for
        for(i=0;i<size;i++)
            dst->data[b][i] = func(src->data[b][i]);
    }
}




