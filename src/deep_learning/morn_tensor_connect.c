/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cblas.h>
#include "morn_tensor.h"

struct TensorConnectPara
{    
    MLayer *prev;
    
    int height;
    int width;
    int channel;
    
    int res_valid;
    
    float rate;
    float decay;
    float momentum;
};
void *mTensorConnectPara(MFile *ini,char *name)
{
    struct TensorConnectPara *para=mMalloc(sizeof(struct TensorConnectPara));
    
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"height");
    if(value != NULL) para->height= atoi(value);else para->height= 1; 
    
    value = mINIRead(ini,name,"width");
    if(value != NULL) para->width = atoi(value);else para->width = 1;
    
    value = mINIRead(ini,name,"channel");
    if(value != NULL) para->channel = atoi(value);else para->channel = 1;

    value = mINIRead(ini,name,"rate");
    if(value != NULL) para->rate = atof(value);
    else
    {
        value = mINIRead(ini,"para","rate");
        if(value != NULL) para->rate = atof(value);
        else              para->rate = 0.001;
    }
    
    value = mINIRead(ini,name,"decay");
    if(value != NULL) para->decay = atof(value);
    else
    {
        value = mINIRead(ini,"para","decay");
        if(value != NULL) para->decay = atof(value);
        else              para->decay = 0.01;
    }
    mException((para->decay<0.0f)||(para->decay>=1.0f),EXIT,"invalid para decay");
    
    value = mINIRead(ini,name,"momentum");
    if(value != NULL) para->momentum = atof(value); 
    else
    {
        value = mINIRead(ini,"para","momentum");
        if(value != NULL) para->momentum = atof(value);
        else              para->momentum = 0.9;
    }
    
    return para;
}

struct HandleTensorConnect
{
    float *weight;
    float *update;
};
void endTensorConnect(void *info)
{
    struct HandleTensorConnect *handle = info;
    if(handle->weight != NULL) mFree(handle->weight);
    if(handle->update != NULL) mFree(handle->update);
}
#define HASH_TensorConnect 0xb8986c4a

void TensorConnectSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorConnectPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    int weight_height= para->channel*para->height*para->width;
    int weight_width =   in->channel*  in->height*  in->width +1;
    int data_size = weight_height*weight_width;
    
    MHandle *hdl; ObjectHandle(out,TensorConnect,hdl);
    struct HandleTensorConnect *handle = hdl->handle;
    
    mTensorRedefine(out,in->batch,para->channel,para->height,para->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
        
        if(handle->update != NULL) mFree(handle->update);
        handle->update =mMalloc(data_size*sizeof(float));
        memset(handle->update,0,data_size*sizeof(float));
    }
    
    if(handle->weight != NULL) mFree(handle->weight);
    handle->weight =mMalloc(data_size*sizeof(float));
    
    if(morn_network_parafile==NULL)
    {
        float scale = sqrt(2.0f/weight_width);
        for(int i=0;i<data_size;i++)
            handle->weight[i] = scale*mNormalRand(0.0f,1.0f);//((float)mRand(-16384,16383))/16384.0f;
    }
    else
    {
        mNetworkParaRead(layer,"weight",handle->weight,data_size*sizeof(float));
    }
    
    hdl->valid = 1;
}

void mTensorConnectForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Connect",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorConnectPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    
    TensorConnectSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorConnect,hdl);
    struct HandleTensorConnect *handle = hdl->handle;
    
    float *weight_data= handle->weight;
    int weight_height= out->height*out->width*out->channel;
    int weight_width =  in->height* in->width* in->channel +1;
    for(int b=0;b<in->batch;b++)
    {
        float *in_data  =  in->data[b];
        float *out_data = out->data[b];
        
        float buff = in_data[weight_width-1];
        in_data[weight_width-1] = 1.0f;
    
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                    weight_height,1,weight_width,
                    1.0f,
                    weight_data,weight_width,
                        in_data,1,
               0.0f,   out_data,1);
               
        in_data[weight_width-1] = buff;
    }
    layer->state = MORN_FORWARD;
}
        
void mTensorConnectBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Connect",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorConnectPara *para = layer->para;
    MTensor *in  = para->prev->tns;
    MTensor *res = para->prev->res;
    MTensor *out = layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorConnect,hdl);
    struct HandleTensorConnect *handle = hdl->handle;
    mException((hdl->valid==0),EXIT,"no forward operate");
    
    int weight_height= out->height*out->width*out->channel;
    int weight_width =  in->height* in->width* in->channel +1;
    float *weight_data = handle->weight;
    float *update_data = handle->update;
    
    mNetworkParaWrite(layer,"weight",weight_data,weight_height*weight_width*sizeof(float));
    
    for(int b=0;b<in->batch;b++)
    {
        float * in_data = in->data[b];
        float *out_data =out->data[b];
        
        float buff = in_data[weight_width-1];
        in_data[weight_width-1] = 1.0f;
        
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                    weight_height,weight_width,1,
                    1.0f,
                       out_data,1,
                        in_data,1,
                    (b==0)?para->momentum:1.0f,
                    update_data,weight_width);
                    
        in_data[weight_width-1] = buff;
    }
    cblas_saxpby(weight_height*weight_width,
                 (0.0f-(para->rate/(float)(in->batch))),update_data,1, 
                 (1.0f-(para->decay*para->rate))       ,weight_data,1);
                
    if(para->res_valid==0) return;
    
    for(int b=0;b<in->batch;b++)
    {
        float *res_data = res->data[b];
        float *out_data = out->data[b];
        
        cblas_sgemm(CblasRowMajor,CblasTrans,CblasNoTrans,
                    weight_width,1,weight_height,
                    1.0f,
                    weight_data,weight_width,
                       out_data,1,
                    ((para->prev->state==MORN_FORWARD)?0.0f:1.0f),
                       res_data,1);
    }
    para->prev->state = MORN_BACKWARD;
}

