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

struct TensorReshapePara
{
    MLayer *prev;

    int channel;
    int height;
    int width;
    
    int res_valid;
};

void *mTensorReshapePara(MFile *ini,char *name)
{
    struct TensorReshapePara *para = (struct TensorReshapePara *)mMalloc(sizeof(struct TensorReshapePara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"height");
    if(value != NULL) para->height= atoi(value);else para->height= DFLT; 
    
    value = mINIRead(ini,name,"width");
    if(value != NULL) para->width = atoi(value);else para->width = DFLT;

    value = mINIRead(ini,name,"channel");
    if(value != NULL) para->channel = atoi(value);else para->channel = DFLT;

    
    return para;
}

void TensorReshapeSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorReshapePara *para = (struct TensorReshapePara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    int size = in->height*in->width*in->channel;
    
    if(para->height <=0) para->height = size/(para->width *para->channel);
    if(para->width  <=0) para->width  = size/(para->height*para->channel);
    if(para->channel<=0) para->channel= size/(para->height*para->width  );
    mException((para->height<0)||(para->width<0)||(para->channel<0)||(para->height*para->width*para->channel!=size),EXIT,"invalid input shape");
    
    mTensorRedefine(out,in->batch,para->channel,para->height,para->width,in->data);
    if(morn_network_flag==MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
}

void mTensorReshapeForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Reshape",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorReshapeSet(layer);

    layer->state = MORN_FORWARD;
}

void mTensorReshapeBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Reshape",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorReshapePara *para = (struct TensorReshapePara *)(layer->para);
    if(para->res_valid==0) return;

    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    int size = res->height*res->width*res->channel;
    for(int b=0;b<res->batch;b++)if(res->data[b]!=out->data[b]) memcpy(res->data[b],out->data[b],size*sizeof(float));
    
    para->prev->state = MORN_BACKWARD;
}
