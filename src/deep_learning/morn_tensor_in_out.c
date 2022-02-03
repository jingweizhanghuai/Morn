/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_tensor.h"
#include "morn_tensor_caculate.h"

struct TensorInputPara
{
    int height;
    int width;
    int channel;
};
void *mTensorInputPara(MList *ini,char *name)
{
    struct TensorInputPara *para = (struct TensorInputPara *)mMalloc(sizeof(struct TensorInputPara));
    
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
    
    struct TensorInputPara *para = (struct TensorInputPara *)(layer->para);
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

void *mTensorOutputPara(MList *ini,char *name)
{
    struct TensorOutputPara *para = (struct TensorOutputPara *)mMalloc(sizeof(struct TensorOutputPara));
    
    para->prev = mNetworkLayer(ini,mINIRead(ini,name,"prev"));
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->height =DFLT; mINIRead(ini,name,"height" ,"%d",&(para->height )); 
    para->width  =DFLT; mINIRead(ini,name,"width"  ,"%d",&(para->width  ));
    para->channel=DFLT; mINIRead(ini,name,"channel","%d",&(para->channel));
    
    if(morn_network_flag == MORN_PREDICT) return para;
    
    char *value = mINIRead(ini,name,"loss");
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
    
    value = mINIRead(ini,name,"argv0");if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[0][0]),value);}
    value = mINIRead(ini,name,"argv1");if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[1][0]),value);}
    value = mINIRead(ini,name,"argv2");if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[2][0]),value);}
    value = mINIRead(ini,name,"argv3");if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[3][0]),value);}

    para->rate    =0.001;if(mINIRead(ini,name,"rate"    ,"%f",&(para->rate    ))==NULL) mINIRead(ini,"para","rate"    ,"%f",&(para->rate    ));
    para->momentum= 0.9 ;if(mINIRead(ini,name,"momentum","%f",&(para->momentum))==NULL) mINIRead(ini,"para","momentum","%f",&(para->momentum));
    para->decay   = 0.01;if(mINIRead(ini,name,"decay"   ,"%f",&(para->decay   ))==NULL) mINIRead(ini,"para","decay"   ,"%f",&(para->decay   ));
    mException((para->decay<0.0f)||(para->decay>=1.0f),EXIT,"invalid para decay");
    
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
void endTensorOutput(void *info) {}
#define HASH_TensorOutput 0xa9f05c6f

void TensorOutputSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorOutputPara *para = (struct TensorOutputPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    if(para->channel== DFLT) para->channel= in->channel;
    if(para->height == DFLT) para->height = in->height;
    if(para->width  == DFLT) para->width  = in->width;
    
    if(morn_network_flag == MORN_PREDICT)
    {
        mTensorCopy(in,out,DFLT);
        mTensorRedefine(in,in->batch,in->channel,in->height,in->width,out->data);
    }
    else mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
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
    
    struct TensorOutputPara *para = (struct TensorOutputPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    // printf("in is %f,out is %f,flag=%d,%d\n",in->data[0][0],out->data[0][0],(in->data[0][0]>0)==(out->data[0][0]>=0),(in->data[0][1]>0)==(out->data[0][1]>=0));
    // printf("in is %f,out is %f,flag=%d\n",in->data[1][0],out->data[1][0],(in->data[1][0]>in->data[1][1])==(out->data[1][0]>out->data[1][1]));
    
    morn_network_error = 0.0f;
    
    if(para->channel!= DFLT) mException((para->channel!=in->channel),EXIT,"invalid output");
    if(para->height != DFLT) mException((para->height !=in->height ),EXIT,"invalid output");
    if(para->width  != DFLT) mException((para->width  !=in->width  ),EXIT,"invalid output");
    
    float network_error = para->loss(layer,para->prev,NULL);
    
    MHandle *hdl=mHandle(out,TensorOutput);
    struct HandleTensorOutput *handle = (struct HandleTensorOutput *)(hdl->handle);
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
    printf("network_error is %f\t",network_error);

    if(para->dloss != NULL)
    {
        para->dloss(layer,para->prev);
    }
    
    if(PARA_SAVE)
    {
        morn_network_parafile = mFileCreate("%s/network_para_%d.morn",morn_network_para_dir,morn_network_time);
        printf("save parameter in %s\n",morn_filename);
    }
    // printf("aaaaaaaaaaaaaaaa\n");
    para->prev->state = MORN_BACKWARD;
}
