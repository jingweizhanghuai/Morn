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

#include "morn_Tensor.h"
#include "morn_tensor_caculate.h"

struct TensorInputPara
{
    int height;
    int width;
    int channel;
};
void *mTensorInputPara(MFile *ini,char *name)
{
    struct TensorInputPara *para = mMalloc(sizeof(struct TensorInputPara));
    
    char *value;
    value=mINIRead(ini,name,"height" );if(value!=NULL) para->height =atoi(value);else para->height = DFLT;
    value=mINIRead(ini,name,"width"  );if(value!=NULL) para->width  =atoi(value);else para->width  = DFLT;
    value=mINIRead(ini,name,"channel");if(value!=NULL) para->channel=atoi(value);else para->channel= DFLT;
    
    return para;
}

void mTensorInputForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Input",mLayerType(layer)),EXIT,"invalid layer type");
    
    struct TensorInputPara *para = layer->para;
    MTensor *out=layer->tns;

    if(para->channel!= DFLT) mException((out->channel!=para->channel),EXIT,"invalid para channel");
    if(para->height != DFLT) mException((out->height !=para->height ),EXIT,"invalid para height");
    if(para->width  != DFLT) mException((out->width  !=para->width  ),EXIT,"invalid para width");
       
    layer->state = MORN_FORWARD;
}

void mTensorInputBackward(MLayer *layer)
{
    if(PARA_SAVE)
    {
        mFileRelease(morn_network_parafile);
        morn_network_parafile = NULL;
    }
}

void *mTensorOutputPara(MFile *ini,char *name)
{
    struct TensorOutputPara *para = mMalloc(sizeof(struct TensorOutputPara));
    
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    value=mINIRead(ini,name,"height" );if(value!=NULL) para->height =atoi(value);else para->height =DFLT; 
    value=mINIRead(ini,name,"width"  );if(value!=NULL) para->width  =atoi(value);else para->width  =DFLT;
    value=mINIRead(ini,name,"channel");if(value!=NULL) para->channel=atoi(value);else para->channel=DFLT;
    
    if(morn_network_flag == MORN_PREDICT) return para;
    
    value = mINIRead(ini,name,"loss");
    if(value == NULL) value = mINIRead(ini,"para","loss");
    mException((value == NULL),EXIT,"layer %s no loss function defined",name);
  
    int i;for(i=0;i<morn_loss_register_num;i++)
    {
        if(strcmp(value,morn_loss_register[i].name)==0)
        {
            para->loss = morn_loss_register[i].loss;
            para->dloss= morn_loss_register[i].dloss;
            break;
        }
    }
    mException((i==morn_loss_register_num),EXIT,"invalid loss function");
    
    value = mINIRead(ini,name,"argv0");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[0][0]),value);}
    value = mINIRead(ini,name,"argv1");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[1][0]),value);}
    value = mINIRead(ini,name,"argv2");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[2][0]),value);}
    value = mINIRead(ini,name,"argv3");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[3][0]),value);}

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

struct HandleTensorOutput
{
    float k;
    float b;
    
    float error[32];
    int idx;
    float sum;
};
void endTensorOutput(void *info) {NULL;}
#define HASH_TensorOutput 0xa9f05c6f

void TensorOutputSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorOutputPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    if(para->channel== DFLT) para->channel= in->channel;
    if(para->height == DFLT) para->height = in->height;
    if(para->width  == DFLT) para->width  = in->width;
    
    if(morn_network_flag == MORN_PREDICT)
    {
        mTensorCopy(in,out);
        mTensorRedefine(in,in->batch,in->channel,in->height,in->width,out->data);
    }
    else
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
}

void mTensorOutputForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Output",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorOutputSet(layer);
    
    layer->state = MORN_FORWARD;
}

void mTensorOutputBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Output",mLayerType(layer)),EXIT,"invalid layer type");
    
    struct TensorOutputPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    morn_network_error = 0.0f;
    
    if(para->channel!= DFLT) mException((para->channel!=in->channel),EXIT,"invalid output");
    if(para->height != DFLT) mException((para->height !=in->height ),EXIT,"invalid output");
    if(para->width  != DFLT) mException((para->width  !=in->width  ),EXIT,"invalid output");
    
    float network_error = para->loss(layer,para->prev,NULL);
    
    MHandle *hdl; ObjectHandle(out,TensorOutput,hdl);
    struct HandleTensorOutput *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        for(int i=0;i<32;i++) handle->error[i] = network_error;
        handle->idx = 0;
        handle->sum = network_error*32;
        hdl->valid = 1;
    }
    handle->sum = handle->sum - handle->error[handle->idx] + network_error;
    handle->error[handle->idx] = network_error;
    handle->idx += 1; 
    if(handle->idx == 32) 
    {
        handle->sum = 0.0f;for(int i=0;i<32;i++) handle->sum+=handle->error[i];
        handle->idx = 0;
    }
    morn_network_error = handle->sum/32.0f;
    // printf("network_error is %f\t",network_error);

    if(para->dloss != NULL)
    {
        para->dloss(layer,para->prev);
    }
    
    if(PARA_SAVE)
    {
        sprintf(morn_network_para_filename,"%s/network_para_%d.morn",morn_network_para_dir,morn_network_time);
        printf("filename is %s\n",morn_network_para_filename);
        morn_network_parafile = mFileCreate(morn_network_para_filename);
    }
    
    para->prev->state = MORN_BACKWARD;
}
