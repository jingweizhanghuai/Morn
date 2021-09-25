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

struct TensorResizePara
{
    MLayer *prev;
    
    int height;
    int width;
    
    int res_valid;
};

void *mTensorResizePara(MList *ini,char *name)
{
    struct TensorResizePara *para = (struct TensorResizePara *)mMalloc(sizeof(struct TensorResizePara));
   
    para->prev = mNetworkLayer(ini,mINIRead(ini,name,"prev"));
    mException((para->prev == NULL),EXIT,"invalid prev");
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    para->height= DFLT; mINIRead(ini,name,"height","%d",&(para->height));
    para->width = DFLT; mINIRead(ini,name,"width" ,"%d",&(para->width ));
    
    return para;
}

struct HandleTensorResize
{
    int *lx;
    int *ly;
    float *wx;
    float *wy;
};
void endTensorResize(void *info)
{
    struct HandleTensorResize *handle = (struct HandleTensorResize *)info;
    if(handle->lx !=NULL) mFree(handle->lx);
    if(handle->ly !=NULL) mFree(handle->ly);
    if(handle->wx !=NULL) mFree(handle->wx);
    if(handle->wy !=NULL) mFree(handle->wy);
}
#define HASH_TensorResize 0xb6f42d3a

void TensorResizeSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorResizePara *para = (struct TensorResizePara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    MHandle *hdl=mHandle(out,TensorResize);
    struct HandleTensorResize *handle = (struct HandleTensorResize *)(hdl->handle);
    
    if(para->height<=0) para->height= in->height;
    if(para->width <=0) para->width = in->width;
    
    mTensorRedefine(out,in->batch,in->channel,para->height,para->width,NULL);
    if(morn_network_flag==MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
    
    if(handle->lx !=NULL) {mFree(handle->lx);} handle->lx=(int   *)mMalloc(para->width *sizeof(int));
    if(handle->ly !=NULL) {mFree(handle->ly);} handle->ly=(int   *)mMalloc(para->height*sizeof(int));
    if(handle->wx !=NULL) {mFree(handle->wx);} handle->wx=(float *)mMalloc(para->width *sizeof(float));
    if(handle->wy !=NULL) {mFree(handle->wy);} handle->wy=(float *)mMalloc(para->height*sizeof(float));
    
    float kx = (float)(in->width)/(float)(out->width);
    for(int i=0;i<out->width;i++)
    {
        float l=kx*((float)i-0.5);
        if(l<0) l=0.0f;else if(l>=in->width-1) l=(float)(in->width)-1.00001;
        handle->lx[i] = (int)l;
        handle->wx[i] = (float)(handle->lx[i]+1)-l;
    }
    
    float ky = (float)(in->height)/(float)(out->height);
    for(int i=0;i<out->height;i++)
    {
        float l=ky*((float)i-0.5); 
        if(l<0) l=0.0f;else if(l>=in->height-1) l=(float)(in->height)-1.00001;
        handle->ly[i] = (int)l;
        handle->wy[i] = (float)(handle->ly[i]+1)-l;
    }
    
    hdl->valid = 1;
}

void mTensorResizeForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Resize",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorResizePara *para = (struct TensorResizePara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorResizeSet(layer);
    
    MHandle *hdl=mHandle(out,TensorResize);
    struct HandleTensorResize *handle = (struct HandleTensorResize *)(hdl->handle);
    
    int  in_size = in->width* in->height;
    int out_size =out->width*out->height;
    
    for(int h=0;h<out->height;h++)for(int w=0;w<out->width;w++)
    {
        int out_idx = h*out->width+w;
        int in_idx1 =(handle->ly[h]  )*in->width + (handle->lx[w]  );
        int in_idx2 =(handle->ly[h]  )*in->width + (handle->lx[w]+1);
        int in_idx3 =(handle->ly[h]+1)*in->width + (handle->lx[w]  );
        int in_idx4 =(handle->ly[h]+1)*in->width + (handle->lx[w]+1);
        float w1 = (    handle->wy[h])*(    handle->wx[w]);
        float w2 = (    handle->wy[h])*(1.0-handle->wx[w]);
        float w3 = (1.0-handle->wy[h])*(    handle->wx[w]);
        float w4 = (1.0-handle->wy[h])*(1.0-handle->wx[w]);
        
        for(int b=0;b<in->batch;b++)for(int c=0;c<in->channel;c++)
        {
            out->data[b][c*out_size+out_idx] = in->data[b][c*in_size+in_idx1]*w1
                                              +in->data[b][c*in_size+in_idx2]*w2
                                              +in->data[b][c*in_size+in_idx3]*w3
                                              +in->data[b][c*in_size+in_idx4]*w4;
        }
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorResizeBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Resize",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorResizePara *para = (struct TensorResizePara *)(layer->para);
    if(para->res_valid==0) return;
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    MHandle *hdl=mHandle(layer->tns,TensorResize);
    struct HandleTensorResize *handle = (struct HandleTensorResize *)(hdl->handle);
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    int  in_size =res->width*res->height;
    int out_size =out->width*out->height;
    
    if(para->prev->state == MORN_FORWARD)
    {
        for(int b=0;b<res->batch;b++) memset(res->data[b],0,in_size*res->channel*sizeof(float));
        para->prev->state = MORN_BACKWARD;
    }
    
    for(int h=0;h<out->height;h++)for(int w=0;w<out->width;w++)
    {
        int out_idx = h*out->width+w;
        int in_idx1 =(handle->ly[h]  )*res->width + (handle->lx[w]  );
        int in_idx2 =(handle->ly[h]  )*res->width + (handle->lx[w]+1);
        int in_idx3 =(handle->ly[h]+1)*res->width + (handle->lx[w]  );
        int in_idx4 =(handle->ly[h]+1)*res->width + (handle->lx[w]+1);
        float w1 = (    handle->wy[h])*(    handle->wx[w]);
        float w2 = (    handle->wy[h])*(1.0-handle->wx[w]);
        float w3 = (1.0-handle->wy[h])*(    handle->wx[w]);
        float w4 = (1.0-handle->wy[h])*(1.0-handle->wx[w]);
        
        for(int b=0;b<res->batch;b++)for(int c=0;c<res->channel;c++)
        {
            res->data[b][c*in_size+in_idx1] += out->data[b][c*out_size+out_idx]*w1;
            res->data[b][c*in_size+in_idx2] += out->data[b][c*out_size+out_idx]*w2;
            res->data[b][c*in_size+in_idx3] += out->data[b][c*out_size+out_idx]*w3;
            res->data[b][c*in_size+in_idx4] += out->data[b][c*out_size+out_idx]*w4;
        }
    }
}

