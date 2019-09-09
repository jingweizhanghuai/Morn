#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Tensor.h"

struct HandleTensorCreate
{
    MTensor *tns;
    int batch;
    int size;
    
    float **data;
    MMemory *memory;
};
#define HASH_TensorCreate 0x6b6cf658
void endTensorCreate(void *info)
{
    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)info;
    mException((handle->tns ==NULL),EXIT,"invalid tensor");
    
    if(!INVALID_POINTER(handle->data)) mFree(handle->data);
    if(!INVALID_POINTER(handle->memory)) mMemoryRelease(handle->memory);
   
    mFree(handle->tns);
}

MTensor *mTensorCreate(int batch,int channel,int height,int width,float **data)
{
    if(batch <0)  batch = 0;
    if(height<0)  height= 0;
    if(width <0)  width = 0;
    if(channel <0)  channel = 0;
    
    MTensor *tns = (MTensor *)mMalloc(sizeof(MTensor));
    memset(tns,0,sizeof(MTensor));
    
    tns->batch = batch;
    tns->height= height;
    tns->width = width;
    tns->channel = channel;
    
    MHandle *hdl; ObjectHandle(tns,TensorCreate,hdl);
    struct HandleTensorCreate *handle = hdl->handle;
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
    if(handle->memory == NULL) handle->memory = mMemoryCreate(batch,size*sizeof(float));
    mMemoryIndex(handle->memory,batch,size*sizeof(float),(void **)(handle->data));

    return tns;
}
    
void mTensorRelease(MTensor *tns)
{
    mException(INVALID_POINTER(tns),EXIT,"invalid input");
    
    if(!INVALID_POINTER(tns->handle))
        mHandleRelease(tns->handle);
}

void mTensorRedefine(MTensor *tns,int batch,int channel,int height,int width,float **data)
{
    mException((INVALID_POINTER(tns)),EXIT,"invalid input");
    
    if(batch  <= 0)  batch  = tns->batch;
    if(channel<= 0)  channel= tns->channel;
    if(height <= 0)  height = tns->height;
    if(width  <= 0)  width  = tns->width;
    if((batch!=tns->batch)||(channel!=tns->channel)||(height!=tns->height)||(width!=tns->width))
        mHandleReset(tns->handle);

    int size = channel*height*width;
    int same_size = ((batch<=tns->batch)&&(size<=tns->height*tns->width*tns->channel));
    int reuse = (data==tns->data);
    int flag = (tns->batch)&&(tns->channel)&&(tns->height)&&(tns->width);
    
    tns->batch = batch;
    tns->height= height;
    tns->width = width;
    tns->channel = channel;
    
    if(same_size&&reuse) return;
    struct HandleTensorCreate *handle = ((MHandle *)(tns->handle->data[0]))->handle;
    if(same_size&&(data==NULL)&&(handle->size >0)) return;
    mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");
    
    if(reuse) data=NULL;
    handle->size = 0;
    
    if((batch == 0)||(size == 0)) {tns->data=NULL;return;}
       
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
    
    if(data!=NULL) 
    {
        memcpy(handle->data,data,batch*sizeof(float *));
        return;
    }
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(batch,size*sizeof(float));
    mMemoryIndex(handle->memory,batch,size*sizeof(float),(void **)(handle->data));
    handle->size = size;
}

void mTensorCopy(MTensor *src,MTensor *dst)
{
    mException((INVALID_TENSOR(src)||INVALID_POINTER(dst)||(dst==src)),EXIT,"invalid input");
    
    mTensorRedefine(dst,src->batch,src->channel,src->height,src->width,dst->data);
    int size = src->channel*src->height*src->width+8;
    for(int i=0;i<src->batch;i++)
        memcpy(dst->data[i],src->data[i],size*sizeof(float));
}

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

#include "morn_Image.h"
void TensorImage(MTensor *tns,int batch,int channel,char *filename)
{
    // printf("tns->height is %d,tns->width is %d\n",tns->height,tns->width);
    MImage *img = mImageCreate(1,tns->height,tns->width,NULL);
    
    int size = tns->height*tns->width;
    
    for(int j=0;j<tns->height;j++)
    {
        float *tns_data = tns->data[batch]+channel*size+j*tns->width;
        for(int i=0;i<tns->width;i++)
        {
            int data = tns_data[i]*128+128;
                 if(data<  0) img->data[0][j][i]=0;
            else if(data>255) img->data[0][j][i]=255;
            else              img->data[0][j][i]=data;
        }
    }
    
    // printf("size is %d\n",size);
    mBMPSave(img,filename);
    // printf("size is %d\n",size);
    mImageRelease(img);
}
            


