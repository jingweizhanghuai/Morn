/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_tensor.h"

struct TensorMulPara
{
    MLayer *prev1;
    MLayer *prev2;
    int res_valid1;
    int res_valid2;
};

void *mTensorMulPara(MSheet *ini,char *name)
{
    struct TensorMulPara *para = (struct TensorMulPara *)mMalloc(sizeof(struct TensorMulPara));
   
    char *value;
    
    value = mINIRead(ini,name,"prev1");
    para->prev1 = mNetworkLayer(ini,value);
    mException((para->prev1 == NULL),EXIT,"invalid prev");
    
    para->res_valid1 = (strcmp("Input",mLayerType(para->prev1))!=0);

    value = mINIRead(ini,name,"prev2");
    para->prev2 = mNetworkLayer(ini,value);
    mException((para->prev2 == NULL),EXIT,"invalid prev");
    
    para->res_valid2 = (strcmp("Input",mLayerType(para->prev2))!=0);
    
    return para;
}

void TensorMulSet(MLayer *layer)
{
    if(layer->state!=DFLT) return;
    struct TensorMulPara *para = (struct TensorMulPara *)(layer->para);
    MTensor *in1 = para->prev1->tns;MTensor *res1= para->prev1->res;
    MTensor *in2 = para->prev2->tns;MTensor *res2= para->prev2->res;
    MTensor *out= layer->tns;
    
    int batch = in1->batch;int channel = in1->channel;int height = in1->height;int width = in1->width;
    mException((height!=in2->height)||(channel!=in2->channel)||(width!=in2->width)||(batch!=in2->batch),EXIT,"invalid prev layer");
    
    mTensorRedefine(out,batch,channel,height,width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res1)) mTensorRedefine(res1,batch,in1->channel,height,width,in1->data);
        else                     mTensorRedefine(res1,batch,in1->channel,height,width,NULL);
        if(INVALID_TENSOR(res2)) mTensorRedefine(res2,batch,in2->channel,height,width,in2->data);
        else                     mTensorRedefine(res2,batch,in2->channel,height,width,NULL);
    }
}

void mTensorMulForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Mul",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorMulSet(layer);
    
    struct TensorMulPara *para = (struct TensorMulPara *)(layer->para);
    MTensor *in1 = para->prev1->tns;
    MTensor *in2 = para->prev2->tns;
    MTensor *out= layer->tns;
    
    int size = out->height*out->width*out->channel;
    for(int b=0;b<out->batch;b++)for(int i=0;i<size;i++)
        out->data[b][i] = in1->data[b][i]*in2->data[b][i];
    
    layer->state = MORN_FORWARD;
}

void mTensorMulBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Mul",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorMulPara *para = (struct TensorMulPara *)(layer->para);
    
    MTensor *in1 = para->prev1->tns;
    MTensor *in2 = para->prev2->tns;
    MTensor *res1= para->prev1->res;
    MTensor *res2= para->prev2->res;
    MTensor *out = layer->res;
    
    int size = out->height*out->width;
    for(int b=0;b<out->batch;b++)for(int i=0;i<size;i++)
    {
        float res1_data = out->data[b][i]*in2->data[b][i];
        float res2_data = out->data[b][i]*in1->data[b][i];
        
        if((b==0)&&(i==100))
        {
            printf("%s:in1 is %f,in2 is %f,res is %f,res1 is %f,res2 is %f\n",
            layer->name,in1->data[b][i],in2->data[b][i],out->data[b][i],res1_data,res2_data);
        }
        
        if(para->res_valid1==1)
        {
            if(para->prev1->state ==MORN_FORWARD) res1->data[b][i] = res1_data;
            else                                  res1->data[b][i]+= res1_data;
        }
        if(para->res_valid2==1) 
        {
            if(para->prev2->state ==MORN_FORWARD) res2->data[b][i] = res2_data;
            else                                  res2->data[b][i]+= res2_data;
            
        }
    }
   
    if(para->res_valid1==1) para->prev1->state = MORN_BACKWARD;
    if(para->res_valid2==1) para->prev2->state = MORN_BACKWARD;
}



