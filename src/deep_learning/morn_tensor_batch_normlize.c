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

struct TensorBatchNormPara
{
    MLayer *prev;
    
    int res_valid;
    
    float rate;
    float decay;
    float momentum;
};

void *mTensorBatchNormPara(MFile *ini,char *name)
{
    struct TensorBatchNormPara *para = (struct TensorBatchNormPara *)mMalloc(sizeof(struct TensorBatchNormPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
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

struct HandleTensorBatchNorm
{
    float *k;
    float *b;
    
    float *k_update;
    float *b_update;
    
    double *mean;
    double *var;
    float *roll_mean;
    float *roll_var;
};
void endTensorBatchNorm(void *info)
{
    struct HandleTensorBatchNorm *handle = (struct HandleTensorBatchNorm *)info;
    if(handle->k        != NULL) mFree(handle->k);
    if(handle->b        != NULL) mFree(handle->b);
    if(handle->mean     != NULL) mFree(handle->mean);
    if(handle->var      != NULL) mFree(handle->var );
    if(handle->roll_mean!= NULL) mFree(handle->roll_mean);
    if(handle->roll_var != NULL) mFree(handle->roll_var );
}
#define HASH_TensorBatchNorm 0xde8952f6

void TensorBatchNormSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorBatchNormPara *para = (struct TensorBatchNormPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorBatchNorm,hdl);
    struct HandleTensorBatchNorm *handle = (struct HandleTensorBatchNorm *)(hdl->handle);
    
    if(handle->k != NULL) {mFree(handle->k);} handle->k =(float *)mMalloc(in->channel*sizeof(float));
    if(handle->b != NULL) {mFree(handle->b);} handle->b =(float *)mMalloc(in->channel*sizeof(float));
    
    if(handle->mean!= NULL) {mFree(handle->mean);} handle->mean=(double *)mMalloc(in->channel*sizeof(double));
    if(handle->var != NULL) {mFree(handle->var );} handle->var =(double *)mMalloc(in->channel*sizeof(double));
    
    if(handle->roll_mean!= NULL) {mFree(handle->roll_mean);} handle->roll_mean=(float *)mMalloc(in->channel*sizeof(float));
    if(handle->roll_var != NULL) {mFree(handle->roll_var );} handle->roll_var =(float *)mMalloc(in->channel*sizeof(float));
    
    mTensorRedefine(out,in->batch,in->channel,in->height,in->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,out->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
        
        handle->k_update = handle->roll_mean;
        handle->b_update = handle->roll_var ;
        memset(handle->k_update,0,in->channel*sizeof(float));
        memset(handle->b_update,0,in->channel*sizeof(float));
    }
    
    if(morn_network_parafile==NULL)
    {
        for(int i=0;i<in->channel;i++){handle->k[i] = 1.0f;handle->b[i] = 1.0f;}
        // printf("handle->k[0] is %f\n",handle->k[0]);
        // printf("handle->b[0] is %f\n",handle->b[0]);
    }
    else
    {
        mNetworkParaRead(layer,"scale",handle->k,in->channel*sizeof(float));
        mNetworkParaRead(layer,"bias" ,handle->b,in->channel*sizeof(float));
        // printf("handle->k[0] is %f\n",handle->k[0]);
        // printf("handle->b[0] is %f\n",handle->b[0]);
    }
    hdl->valid = 1;
}

void mTensorBatchNormForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("BatchNorm",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorBatchNormPara *para = (struct TensorBatchNormPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorBatchNormSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorBatchNorm,hdl);
    struct HandleTensorBatchNorm *handle = (struct HandleTensorBatchNorm *)(hdl->handle);
    
    int in_size = in->height*in->width;
    
    memset(handle->mean,0,in->channel*sizeof(double));
    memset(handle->var ,0,in->channel*sizeof(double));
    for(int b=0;b<in->batch;b++)for(int c=0;c<in->channel;c++)for(int i=0;i<in_size;i++)
    {
        double data = (double)(in->data[b][c*in_size+i]);
        handle->mean[c] += data;
        handle->var [c] += data*data;
    }
    for(int c=0;c<in->channel;c++)
    {
        handle->mean[c] = handle->mean[c]/(double)(in->batch*in_size);
        handle->var [c] = handle->var [c]/(double)(in->batch*in_size);
        handle->var [c]-= (handle->mean[c]*handle->mean[c]);
        handle->var [c] = (handle->var[c]<=0.0f)?0.00001f:sqrt(handle->var[c]);
        // printf("handle->mean[c] is %f,handle->var[c] is %f\n",handle->mean[c],handle->var[c]);
    }
    
    if(morn_network_flag == MORN_PREDICT)
    {
        for(int c=0;c<in->channel;c++)
        {
            if(layer->state == DFLT)
            {
                handle->roll_mean[c] = handle->mean[c];
                handle->roll_var [c] = handle->var [c];
            }
        
            handle->mean[c] = handle->roll_mean[c]*0.99f+handle->mean[c]*0.01;
            handle->var [c] = handle->roll_var [c]*0.99f+handle->var [c]*0.01;
            handle->roll_mean[c] = handle->mean[c];
            handle->roll_var [c] = handle->var [c];
        }
    }
    
    for(int b=0;b<in->batch;b++)for(int c=0;c<in->channel;c++)for(int i=0;i<in_size;i++)
    {
        float data = in->data[b][c*in_size+i];
        out->data[b][c*in_size+i] = ((data-handle->mean[c])/handle->var[c])*handle->k[c]+handle->b[c];
    }
            
    layer->state = MORN_FORWARD;
}

void mTensorBatchNormBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("BatchNorm",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorBatchNormPara *para = (struct TensorBatchNormPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorBatchNorm,hdl);
    struct HandleTensorBatchNorm *handle = (struct HandleTensorBatchNorm *)(hdl->handle);
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    mNetworkParaWrite(layer,"scale",handle->k,in->channel*sizeof(float));
    mNetworkParaWrite(layer,"bias" ,handle->b,in->channel*sizeof(float));
    
    int in_size = in->height*in->width;
    
    for(int c=0;c<in->channel;c++)
    {
        handle->k_update[c] *= para->momentum;
        handle->b_update[c] *= para->momentum;
    }
    for(int b=0;b<in->batch;b++)for(int c=0;c<in->channel;c++)for(int i=0;i<in_size;i++)
    {
        int idx = c*in_size+i;
        handle->k_update[c] += out->data[b][idx]*((in->data[b][idx]-handle->mean[c])/handle->var[c]);
        handle->b_update[c] += out->data[b][idx];
    }
    for(int c=0;c<in->channel;c++)
    {
        handle->k[c] = (handle->k[c]*(1.0f-(para->decay*para->rate)))-(handle->k_update[c]*para->rate/(float)(in->batch*in_size));
        handle->b[c] = (handle->b[c]*(1.0f-(para->decay*para->rate)))-(handle->b_update[c]*para->rate/(float)(in->batch*in_size));
    }
    
    if(para->res_valid==0) return;
    
    for(int b=0;b<in->batch;b++)for(int c=0;c<in->channel;c++)for(int i=0;i<in_size;i++)
    {
        int idx = c*in_size+i;
        float data;
        data = (in->data[b][idx] - handle->mean[c])/handle->var[c];
        data = (out->data[b][idx]*handle->k[c]/handle->var[c])*(1.0+(1.0f-data*data)/((float)(in->batch*in_size)));
        
        res->data[b][idx]=(para->prev->state == MORN_FORWARD)?data:(res->data[b][idx]+data);
    }
    
    para->prev->state = MORN_BACKWARD;
}
