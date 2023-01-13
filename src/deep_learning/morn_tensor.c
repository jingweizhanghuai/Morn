/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_tensor.h"

struct HandleTensorCreate
{
    MTensor *tns;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    
    int batch;
    int size;

    float **data;
    MMemory *memory;

    float **backup_data;
    MMemory *backup_memory;
};
#define HASH_TensorCreate 0x6b6cf658
void endTensorCreate(struct HandleTensorCreate *handle)
{
    mException((handle->tns ==NULL),EXIT,"invalid tensor");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->data    !=NULL) mFree(handle->data);
    if(handle->memory  !=NULL) mMemoryRelease(handle->memory);

    if(handle->backup_data  !=NULL) mFree(handle->backup_data);
    if(handle->backup_memory!=NULL) mMemoryRelease(handle->backup_memory);
    memset(handle->tns,0,sizeof(MTensor));
    // mFree(((MList **)(handle->tns))-1);
}

MTensor *TensorCreate(int batch,int channel,int height,int width,float **data,int device)
{
    MTensor *tns = (MTensor *)ObjectAlloc(sizeof(MTensor));
    mObjectType(tns)=HASH_TensorCreate;
    MHandle *hdl=mHandle(tns,TensorCreate);
    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(hdl->handle);
    handle->tns = tns;
    
    if(batch  <0) {batch  = 0;        } tns->batch  = batch;
    if(channel<0) {channel= 0;        } tns->channel= channel;
    if(height <0) {height = 0;        } tns->height = height;
    if(width  <0) {width  = 0;        } tns->width  = width;
    if(device <0) {device = MORN_HOST;} tns->device = MORN_HOST;

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
    handle->memory = mMemoryCreate(batch,size*sizeof(float),device);
    
    void ***idx = malloc(batch*sizeof(void **));
    for(int i=0;i<batch;i++) idx[i]=(void **)(&(handle->data[i]));
    mMemoryIndex(handle->memory,1,size*sizeof(float),idx,batch);
    free(idx);

    mPropertyFunction(tns,"device",mornMemoryDevice,handle->memory);

    for(int b=0;b<batch;b++) tns->data[b][channel*height*width]=1.0f;

    return tns;
}
    
void mTensorRelease(MTensor *tns)
{
    ObjectFree(tns);
}

MMemoryBlock *mTensorMemory(MTensor *tns,int batch)
{
    int size = tns->channel*tns->height*tns->width+8;
    float *data = tns->data[batch];

    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(ObjHandle(tns,0)->handle);
    if(handle->memory == NULL) 
    {
        handle->memory = mMemoryCreate(batch,size*sizeof(float),MORN_HOST);
        mPropertyFunction(tns,"device",mornMemoryDevice,handle->memory);
    }
    MMemoryBlock *mem = handle->memory->data[batch];
    if(mem->size<size)
    {
        void ***idx = malloc(batch*sizeof(void **));
        for(int i=0;i<batch;i++) idx[i]=(void **)(&(handle->data[i]));
        mMemoryIndex(handle->memory,1,size*sizeof(float),idx,batch);
        free(idx);
    }
    
    if(mem->data!=data) memcpy(mem->data,data,size*sizeof(float));
    
    return mem;
}

void TensorRedefine(MTensor *tns,int batch,int channel,int height,int width,float **data,int device)
{
    mException((INVALID_POINTER(tns)),EXIT,"invalid input");
    
    if(batch  <= 0)  batch  = tns->batch;
    if(channel<= 0)  channel= tns->channel;
    if(height <= 0)  height = tns->height;
    if(width  <= 0)  width  = tns->width;
    if(INVALID_POINTER(data)) data=tns->data;

    int size = channel*height*width+8;
    
    if((batch!=tns->batch)||(channel!=tns->channel)||(height!=tns->height)||(width!=tns->width))
        mHandleReset(tns);
    
    int same_size = (batch<=tns->batch)&&(size<tns->channel*tns->height*tns->width)&&(data==tns->data);
    tns->batch=batch; tns->height=height; tns->width=width; tns->channel=channel;
    if(same_size&&(data==NULL)) goto tensor_redefine_end;
    if(same_size&&((device<0)||(device==mMemoryBlock(data[0])->device))) goto tensor_redefine_end;
    
    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(ObjHandle(tns,0)->handle);
    if(device<0)
    {
        if((data!=tns->data)&&(data!=NULL)) device=mMemoryBlock(data[0])->device;
    }
    
    if((data!=tns->data)&&(data!=NULL)) 
    {
        for(int bc=0;bc<batch;bc++) 
            mException(mMemoryBlock(data[bc])->device!=device,EXIT,"invalid data device");
    }

    if((batch<=handle->batch)&&(size<=handle->size)&&(data==handle->data)) return;
    
    // int flag = (tns->batch)&&(tns->channel)&&(tns->height)&&(tns->width);
    // mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");

    if((batch==0)||(size<=8)) 
    {
        mException((data!=tns->data),EXIT,"invalid input");
        tns->data=NULL;
        goto tensor_redefine_end;
    }
    
    if(batch>handle->batch){if(handle->data != NULL) {free(handle->data);}handle->data=NULL;}
    if(handle->data==NULL)
    {
        handle->data = (float **)malloc(batch*sizeof(float *));
        handle->batch = batch;
    }
    
    if(data!=tns->data)
    {
        memcpy(handle->data,data,batch*sizeof(float *));
        tns->data = handle->data;
        if(handle->backup_data  !=NULL) mFree(handle->backup_data);
        if(handle->backup_memory!=NULL) mMemoryRelease(handle->backup_memory);
        goto tensor_redefine_end;
    }
    
    if(handle->memory == NULL) 
    {
        handle->memory = mMemoryCreate(batch,size*sizeof(float),device);
        mPropertyFunction(tns,"device",mornMemoryDevice,handle->memory);
    }
    else mMemoryRedefine(handle->memory,batch,size*sizeof(float),device);

    void ***idx = malloc(batch*sizeof(void **));
    for(int i=0;i<batch;i++) idx[i]=(void **)(&(handle->data[i]));
    mMemoryIndex(handle->memory,1,size*sizeof(float),idx,batch);
    free(idx);
    
    tns->data = handle->data;
    handle->size  = size;

    tensor_redefine_end:
    for(int b=0;b<batch;b++) tns->data[b][channel*height*width]=1.0f;
}

float **mTensorBackup(MTensor *tns,int batch,int cn,int height,int width)
{
    if(batch <=0) batch =tns->batch;
    if(cn    <=0) cn    =tns->channel;
    if(height<=0) height=tns->height;
    if(width <=0) width =tns->width;

    int size = cn*height*width;

    struct HandleTensorCreate *handle = (struct HandleTensorCreate *)(ObjHandle(tns,0)->handle);
    if(handle->backup_data!=NULL) mFree(handle->backup_data);
    handle->backup_data = (float **)mMalloc(batch*sizeof(float *));

    if(handle->backup_memory == NULL) handle->backup_memory = mMemoryCreate(batch,size*sizeof(float),tns->device);
    else mMemoryRedefine(handle->backup_memory,batch,size*sizeof(float),tns->device);
    for(int i=0;i<batch;i++) handle->backup_data[i] = (float *)(handle->backup_memory->data[i]);
    
    return (handle->backup_data);
}

void MemCopy(void *dst,int dst_dev,void *src,int src_dev,int size);
void mTensorCopy(MTensor *src,MTensor *dst,int device)
{
    mException(INVALID_POINTER(src),EXIT,"invalid input source tensor");
    mException(INVALID_POINTER(dst)&&(device<0),EXIT,"invalid input device");
    if(device<0) device=dst->device;

    float **dst_data;
    int flag = (INVALID_POINTER(dst))||(dst==src);
    if(flag) {if(device==src->device){return;} dst_data=mTensorBackup(src,DFLT,DFLT,DFLT,DFLT);}
    else     {if(device!=dst->device){mTensorRedefine(dst,DFLT,DFLT,DFLT,DFLT,NULL,device);} dst_data=dst->data;}
    
    // int size = src->channel*src->height*src->width;
    // for(int i=0;i<src->batch;i++)
    //     MemCopy(dst_data[i],device,src->data[i],src->device,size*sizeof(float));

    if(flag) mTensorRedefine(src,DFLT,DFLT,DFLT,DFLT,dst_data,device);
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




