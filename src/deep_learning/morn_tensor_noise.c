/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

void *mTensorDropoutPara(MFile *ini,char *name)
{
    struct TensorDropoutPara *para = mMalloc(sizeof(struct TensorDropoutPara));
   
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
    struct TensorDropoutPara *para = layer->para;
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
    
    struct TensorDropoutPara *para = layer->para;
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
    struct TensorDropoutPara *para = layer->para;
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

void *mTensorJitterPara(MFile *ini,char *name)
{
    struct TensorJitterPara *para = mMalloc(sizeof(struct TensorJitterPara));
   
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
    struct HandleTensorJitter *handle = info;
    if(handle->scale != NULL) mFree(handle->scale);
}
#define HASH_TensorJitter 0x745a6164

void TensorJitterSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorJitterPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    mTensorRedefine(out,in->batch,in->channel,in->height,in->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
    
    MHandle *hdl; ObjectHandle(out,TensorJitter,hdl);
    struct HandleTensorJitter *handle = hdl->handle;
    
    handle->num = in->channel*in->height*in->width; if(handle->num<1024) handle->num=1024;
    handle->num = ((handle->num+in->batch-1)/in->batch)*in->batch;
    
    if(handle->scale != NULL) mFree(handle->scale); handle->scale = mMalloc(handle->num*sizeof(float));
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
    
    struct TensorJitterPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorJitter,hdl);
    struct HandleTensorJitter *handle = hdl->handle;
    
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
    struct TensorJitterPara *para = layer->para;
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorJitter,hdl);
    struct HandleTensorJitter *handle = hdl->handle;
    
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





    
        
        
    
    
    
    
    
    