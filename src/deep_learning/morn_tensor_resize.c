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
#include "morn_Tensor.h"

struct TensorResizePara
{
    MLayer *prev;
    
    int height;
    int width;
    
    int res_valid;
};

void *mTensorResizePara(MFile *ini,char *name)
{
    struct TensorResizePara *para = mMalloc(sizeof(struct TensorResizePara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"height");
    if(value != NULL) para->height= atoi(value);else para->height= DFLT; 
    
    value = mINIRead(ini,name,"width");
    if(value != NULL) para->width = atoi(value);else para->width = DFLT;
    
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
    struct HandleTensorResize *handle = info;
    if(handle->lx !=NULL) mFree(handle->lx);
    if(handle->ly !=NULL) mFree(handle->ly);
    if(handle->wx !=NULL) mFree(handle->wx);
    if(handle->wy !=NULL) mFree(handle->wy);
}
#define HASH_TensorResize 0xb6f42d3a

void TensorResizeSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorResizePara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorResize,hdl);
    struct HandleTensorResize *handle = hdl->handle;
    
    if(para->height<=0) para->height= in->height;
    if(para->width <=0) para->width = in->width;
    
    mTensorRedefine(out,in->batch,in->channel,para->height,para->width,NULL);
    if(morn_network_flag==MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
    
    if(handle->lx !=NULL) {mFree(handle->lx);} handle->lx=mMalloc(para->width *sizeof(int));
    if(handle->ly !=NULL) {mFree(handle->ly);} handle->ly=mMalloc(para->height*sizeof(int));
    if(handle->wx !=NULL) {mFree(handle->wx);} handle->wx=mMalloc(para->width *sizeof(float));
    if(handle->wy !=NULL) {mFree(handle->wy);} handle->wy=mMalloc(para->height*sizeof(float));
    
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
    struct TensorResizePara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorResizeSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorResize,hdl);
    struct HandleTensorResize *handle = hdl->handle;
    
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
    struct TensorResizePara *para = layer->para;
    if(para->res_valid==0) return;
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorResize,hdl);
    struct HandleTensorResize *handle = hdl->handle;
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

