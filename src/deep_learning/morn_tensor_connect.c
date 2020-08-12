/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
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
    struct TensorConnectPara *para=(struct TensorConnectPara *)mMalloc(sizeof(struct TensorConnectPara));
    
    para->prev = mNetworkLayer(ini,mINIRead(ini,name,"prev"));
    mException((para->prev == NULL),EXIT,"invalid prev");
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    para->height = 1; mINIRead(ini,name,"height" ,"%d",&(para->height ));
    para->width  = 1; mINIRead(ini,name,"width"  ,"%d",&(para->width  ));
    para->channel= 1; mINIRead(ini,name,"channel","%d",&(para->channel));

    para->rate    =0.001;if(mINIRead(ini,name,"rate"    ,"%f",&(para->rate    ))==NULL) mINIRead(ini,"para","rate"    ,"%f",&(para->rate    ));
    para->momentum= 0.9 ;if(mINIRead(ini,name,"momentum","%f",&(para->momentum))==NULL) mINIRead(ini,"para","momentum","%f",&(para->momentum));
    para->decay   = 0.01;if(mINIRead(ini,name,"decay"   ,"%f",&(para->decay   ))==NULL) mINIRead(ini,"para","decay"   ,"%f",&(para->decay   ));
    mException((para->decay<0.0f)||(para->decay>=1.0f),EXIT,"invalid para decay");
    
    return para;
}

struct HandleTensorConnect
{
    float *weight;
    float *update;
};
void endTensorConnect(void *info)
{
    struct HandleTensorConnect *handle = (struct HandleTensorConnect *)info;
    if(handle->weight != NULL) mFree(handle->weight);
    if(handle->update != NULL) mFree(handle->update);
}
#define HASH_TensorConnect 0xb8986c4a

void TensorConnectSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorConnectPara *para = (struct TensorConnectPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    int weight_height= para->channel*para->height*para->width;
    int weight_width =   in->channel*  in->height*  in->width +1;
    int data_size = weight_height*weight_width;
    
    MHandle *hdl=mHandle(out,TensorConnect);
    struct HandleTensorConnect *handle = (struct HandleTensorConnect *)(hdl->handle);
    
    mTensorRedefine(out,in->batch,para->channel,para->height,para->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
        
        if(handle->update != NULL) mFree(handle->update);
        handle->update =(float *)mMalloc(data_size*sizeof(float));
        memset(handle->update,0,data_size*sizeof(float));
    }
    
    if(handle->weight != NULL) mFree(handle->weight);
    handle->weight =(float *)mMalloc(data_size*sizeof(float));
    
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
    struct TensorConnectPara *para = (struct TensorConnectPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    
    TensorConnectSet(layer);
    
    MHandle *hdl=mHandle(out,TensorConnect);
    struct HandleTensorConnect *handle = (struct HandleTensorConnect *)(hdl->handle);
    
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

        // printf("\nout=\n");for(int ii=0;ii<10;ii++) printf("%f,",out_data[ii]);
    }

    layer->state = MORN_FORWARD;
}
        
void mTensorConnectBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Connect",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorConnectPara *para = (struct TensorConnectPara *)(layer->para);
    MTensor *in  = para->prev->tns;
    MTensor *res = para->prev->res;
    MTensor *out = layer->res;
    
    MHandle *hdl=mHandle(layer->tns,TensorConnect);
    struct HandleTensorConnect *handle = (struct HandleTensorConnect *)(hdl->handle);
    mException((hdl->valid==0),EXIT,"no forward operate");
    
    int weight_height= out->height*out->width*out->channel;
    int weight_width =  in->height* in->width* in->channel +1;
    float *weight_data = handle->weight;
    float *update_data = handle->update;
    
    mNetworkParaWrite(layer,"weight",weight_data,weight_height*weight_width*sizeof(float));

    float beta = para->momentum;
    for(int b=0;b<in->batch;b++)
    {
        float * in_data = in->data[b];
        float *out_data =out->data[b];
        
        float buff = in_data[weight_width-1];
        in_data[weight_width-1] = 1.0f;

        // printf("m=%d,n=%d,k=%d\n",weight_height,weight_width,1);
        // printf("\nconnectdelta=\n");for(int ii=0;ii<10;ii++) printf("%f,",out_data[ii]);
        // printf("\nconnectin=\n");for(int ii=0;ii<10;ii++) printf("%f,",in_data[ii]);
        // printf("para->momentum=%f\n",para->momentum);
        
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                    weight_height,weight_width,1,
                    1.0f,
                       out_data,1,
                        in_data,1,
                    beta,
                    update_data,weight_width);

        // printf("\nconnectupdate=\n");for(int ii=0;ii<100;ii++) printf("%f,",update_data[ii]);
        
        in_data[weight_width-1] = buff;
        beta = 1.0;
    }
                
    if(para->res_valid) for(int b=0;b<in->batch;b++)
    {
        float *res_data = res->data[b];
        float *out_data = out->data[b];

        // printf("\nweights=\n");for(int ii=400;ii<600;ii++) printf("%f,",weight_data[ii]);
        cblas_sgemm(CblasRowMajor,CblasTrans,CblasNoTrans,
                    weight_width,1,weight_height,
                    1.0f,
                    weight_data,weight_width,
                       out_data,1,
                    ((para->prev->state==MORN_FORWARD)?0.0f:1.0f),
                       res_data,1);
        // printf("\ndeltaout=\n");for(int ii=400;ii<600;ii++) printf("%f,",res_data[ii]);
    }

    cblas_saxpby(weight_height*weight_width,
                 (0.0f-(para->rate/(float)(in->batch))),update_data,1, 
                 (1.0f-(para->decay*para->rate))       ,weight_data,1);
    
    para->prev->state = MORN_BACKWARD;
}

