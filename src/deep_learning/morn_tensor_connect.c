/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

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
void *mTensorConnectPara(MSheet *ini,char *name)
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
    MMemoryBlock *weight;
    MMemoryBlock *update;
};
void endTensorConnect(struct HandleTensorConnect *handle)
{
    if(handle->weight != NULL) mMemoryBlockRelease(handle->weight);
    if(handle->update != NULL) mMemoryBlockRelease(handle->update);
}
#define HASH_TensorConnect 0xb8986c4a

struct HandleTensorConnect *TensorConnectSet(MLayer *layer)
{
    struct TensorConnectPara *para = (struct TensorConnectPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    int device = out->device;

    MHandle *hdl=mHandle(out,TensorConnect);
    struct HandleTensorConnect *handle = (struct HandleTensorConnect *)(hdl->handle);
    if(layer->state != DFLT) return handle;
    
    int weight_height= para->channel*para->height*para->width;
    int weight_width =   in->channel*  in->height*  in->width +1;
    int data_size = weight_height*weight_width;
    
    mTensorRedefine(out,in->batch,para->channel,para->height,para->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
        
        if(handle->update != NULL) mMemoryBlockRelease(handle->update);
        handle->update =mMemoryBlockCreate(data_size*sizeof(float),device);
        memset(handle->update->data,0,data_size*sizeof(float));
        handle->update->flag = MORN_HOST;
        mMemoryBlockWrite(handle->update);
    }
    
    if(handle->weight != NULL) mMemoryBlockRelease(handle->weight);
    handle->weight =mMemoryBlockCreate(data_size*sizeof(float),device);
    float *weight_data = (float *)(handle->weight->data);
    if(morn_network_parafile==NULL)
    {
        float scale = sqrt(2.0f/weight_width);
        for(int i=0;i<data_size;i++)
            weight_data[i] = scale*mNormalRand(0.0f,1.0f);//((float)mRand(-16384,16383))/16384.0f;
    }
    else
    {
        mNetworkParaRead(layer,"weight",weight_data,data_size*sizeof(float));
    }
    handle->weight->flag = MORN_HOST;
    mMemoryBlockWrite(handle->weight);
    
    hdl->valid = 1;
    return handle;
}

void mTensorConnectForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Connect",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorConnectPara *para = (struct TensorConnectPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    int device = out->device;
    
    struct HandleTensorConnect *handle = TensorConnectSet(layer);
    
    int weight_height= out->height*out->width*out->channel;
    int weight_width =  in->height* in->width* in->channel +1;
    for(int b=0;b<in->batch;b++)
    {
        mSgemm(device,MORN_NO_TRANS,MORN_NO_TRANS,weight_height,1,weight_width,1.0f,handle->weight,weight_width,mTensorMemory(in,b),1,0.0f,mTensorMemory(out,b),1);
        printf("out_data[8]=%f\n",((float *)(out->data[b]))[8]);
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
    int device = out->device;
    
    MHandle *hdl=mHandle(layer->tns,TensorConnect);
    struct HandleTensorConnect *handle = (struct HandleTensorConnect *)(hdl->handle);
    mException((hdl->valid==0),EXIT,"no forward operate");
    
    int weight_height= out->height*out->width*out->channel;
    int weight_width =  in->height* in->width* in->channel +1;
    float *update_data = (float*)(handle->update->data);
    
    mNetworkParaWrite(layer,"weight",handle->weight->data,weight_height*weight_width*sizeof(float));
    
    float beta = para->momentum;
    for(int b=0;b<in->batch;b++)
    {
        mSgemm(device,MORN_NO_TRANS,MORN_TRANS,weight_height,weight_width,1,1.0f,mTensorMemory(out,b),1,mTensorMemory(in,b),1,beta,handle->update,weight_width);
        printf("update_data[8]=%f\n",((float*)(handle->update->data))[8]);
        beta = 1.0;
    }
    
    if(para->res_valid) for(int b=0;b<in->batch;b++)
    {
        mSgemm(device,MORN_TRANS,MORN_NO_TRANS,weight_width,1,weight_height,1.0f,handle->weight,weight_width,mTensorMemory(out,b),1,((para->prev->state==MORN_FORWARD)?0.0f:1.0f),mTensorMemory(res,b),1);
        printf("res_data[8]=%f\n",((float *)(res->data[b]))[8]);
    }
    mSaxpby(device,weight_height*weight_width,(0.0f-(para->rate/(float)(in->batch))),handle->update,1,(1.0f-(para->decay*para->rate)),handle->weight,1);

    para->prev->state = MORN_BACKWARD;
}

