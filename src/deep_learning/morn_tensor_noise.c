/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_tensor.h"

struct TensorDropoutPara
{
    MLayer *prev;
    int res_valid;
    float prob;
};

void *mTensorDropoutPara(MSheet *ini,char *name)
{
    struct TensorDropoutPara *para = (struct TensorDropoutPara *)mMalloc(sizeof(struct TensorDropoutPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"prob");
    if(value != NULL) para->prob= atof(value);else para->prob= 0.01f;
    
    return para;
}

void TensorDropoutSet(MLayer *layer)
{
    if(layer->state!=DFLT) return;
    struct TensorDropoutPara *para = (struct TensorDropoutPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    mTensorRedefine(out,in->batch,in->channel,in->height,in->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
}

void mTensorDropoutForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Droupout",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorDropoutSet(layer);
    layer->state = MORN_FORWARD;
    if(morn_network_flag == MORN_PREDICT) return;
    
    struct TensorDropoutPara *para =  (struct TensorDropoutPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    
    int size = in->channel*in->height*in->width;
    int num = (int)(((float)(in->batch*size))*para->prob+0.5);
    para->prob = (float)num/((float)(in->batch*size));
    float k = 1.0f/para->prob;
    
    for(int b=0;b<in->batch;b++)for(int i=0;i<size;i++)
        out->data[b][i] = in->data[b][i]*k;
        
    for(int j=0;j<num;j++)
    {
        int n = mRand(0,size*in->batch);
        int b = n/size;int i = n%size;
        out->data[b][i] =0.0f;
    }
}

void mTensorDropoutBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Droupout",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorDropoutPara *para =  (struct TensorDropoutPara *)(layer->para);
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    int size = out->channel*out->height*out->width;
    int num = (int)(((float)(out->batch*size))*para->prob+0.5);
    float k = 1.0f/para->prob;
    
    for(int b=0;b<out->batch;b++)for(int i=0;i<size;i++)
    {
        float data = out->data[b][i]*k;
        res->data[b][i] =(para->prev->state==MORN_FORWARD)?data:(res->data[b][i]+data);
    }
    
    for(int j=0;j<num;j++)
    {
        int n = mRand(0,size*out->batch);
        int b = n/size;int i = n%size;
        res->data[b][i] =(para->prev->state==MORN_FORWARD)?0.0f:(res->data[b][i]);
    }
    
    para->prev->state = MORN_BACKWARD;
}

struct TensorJitterPara
{
    MLayer *prev;
    int res_valid;
    float delta;
};

void *mTensorJitterPara(MSheet *ini,char *name)
{
    struct TensorJitterPara *para = (struct TensorJitterPara *)mMalloc(sizeof(struct TensorJitterPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"delta");
    if(value != NULL) para->delta= atof(value);else para->delta= 0.1f;
    
    return para;
}

struct HandleTensorJitter
{
    int num;
    float *scale;
};
void endTensorJitter(void *info)
{
    struct HandleTensorJitter *handle = (struct HandleTensorJitter *)info;
    if(handle->scale != NULL) mFree(handle->scale);
}
#define HASH_TensorJitter 0x745a6164

void TensorJitterSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorJitterPara *para = (struct TensorJitterPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    mTensorRedefine(out,in->batch,in->channel,in->height,in->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
    
    MHandle *hdl=mHandle(out,TensorJitter);
    struct HandleTensorJitter *handle = (struct HandleTensorJitter *)(hdl->handle);
    
    handle->num = in->channel*in->height*in->width; if(handle->num<1024) handle->num=1024;
    handle->num = ((handle->num+in->batch-1)/in->batch)*in->batch;
    
    if(handle->scale != NULL) mFree(handle->scale); handle->scale = (float *)mMalloc(handle->num*sizeof(float));
    for(int i=0;i<handle->num;i++)
        handle->scale[i] = mNormalRand(1.0f,para->delta);
    
    hdl->valid = 1;
}

void mTensorJitterForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Jitter",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorJitterSet(layer);
    layer->state = MORN_FORWARD;
    if(morn_network_flag == MORN_PREDICT) return;
    
    struct TensorJitterPara *para = (struct TensorJitterPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    
    MHandle *hdl=mHandle(out,TensorJitter);
    struct HandleTensorJitter *handle = (struct HandleTensorJitter *)(hdl->handle);
    
    int size = in->channel*in->height*in->width;
    int num = handle->num/in->batch;
    
    for(int b=0;b<in->batch;b++)
    {
        for(int i=b*num;i<(b+1)*num;i++)
        {
            int n = mRand(0,handle->num);
            float buff=handle->scale[i];handle->scale[i]=handle->scale[n];handle->scale[n]=buff;
        }
        
        for(int i=0;i<size;i++)
            out->data[b][i] = in->data[b][i]*handle->scale[i];
    }
}

void mTensorJitterBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Droupout",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorJitterPara *para = (struct TensorJitterPara *)(layer->para);
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    MHandle *hdl=mHandle(layer->tns,TensorJitter);
    struct HandleTensorJitter *handle = (struct HandleTensorJitter *)(hdl->handle);
    
    int size = out->channel*out->height*out->width;
    int num = handle->num/out->batch;
    
    for(int b=0;b<out->batch;b++)
    {
        for(int i=b*num;i<(b+1)*num;i++)
        {
            int n = mRand(0,handle->num);
            float buff=handle->scale[i];handle->scale[i]=handle->scale[n];handle->scale[n]=buff;
        }
        
        for(int i=0;i<size;i++)
            res->data[b][i] = out->data[b][i]*handle->scale[i];
    }
    
    para->prev->state = MORN_BACKWARD;
}
