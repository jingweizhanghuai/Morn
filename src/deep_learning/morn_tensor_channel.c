/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_tensor.h"

struct TensorMergePara
{
    MLayer *prev1;
    MLayer *prev2;
    int res_valid1;
    int res_valid2;
};

void *mTensorMergePara(MSheet *ini,char *name)
{
    struct TensorMergePara *para = (struct TensorMergePara *)mMalloc(sizeof(struct TensorMergePara));
   
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

void TensorMergeSet(MLayer *layer)
{
    if(layer->state!=DFLT) return;
    struct TensorMergePara *para = (struct TensorMergePara *)(layer->para);
    MTensor *in1 = para->prev1->tns;MTensor *res1= para->prev1->res;
    MTensor *in2 = para->prev2->tns;MTensor *res2= para->prev2->res;
    MTensor *out= layer->tns;
    
    int batch = in1->batch;int height = in1->height;int width = in1->width;
    printf("in1->height is %d,in1->width is %d\n",in1->height,in1->width);
    printf("in2->height is %d,in2->width is %d\n",in2->height,in2->width);
    mException((height!=in2->height)||(width!=in2->width)||(batch!=in2->batch),EXIT,"invalid prev layer");
    
    mTensorRedefine(out,batch,in1->channel+in2->channel,height,width,NULL);
    float **data1=(float **)mMalloc(batch*sizeof(float *));float **data2=(float **)mMalloc(batch*sizeof(float *));
    for(int b=0;b<batch;b++) 
    {
        data1[b]=out->data[b];
        data2[b]=data1[b]+height*width*in1->channel;
        memcpy(data1[b],in1->data[b],height*width*in1->channel*sizeof(float));
        memcpy(data2[b],in2->data[b],height*width*in2->channel*sizeof(float));
    }
    mTensorRedefine(in1,batch,in1->channel,height,width,data1);
    mTensorRedefine(in2,batch,in2->channel,height,width,data2);
    mFree(data1); mFree(data2);
    
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res1)) mTensorRedefine(res1,batch,in1->channel,height,width,in1->data);
        else                     mTensorRedefine(res1,batch,in1->channel,height,width,NULL);
        if(INVALID_TENSOR(res2)) mTensorRedefine(res2,batch,in2->channel,height,width,in2->data);
        else                     mTensorRedefine(res2,batch,in2->channel,height,width,NULL);
        
        // if(strcmp(layer->name,"merge7"))
        // {
            // printf("in2->data[0] is %p,res2->data[0] is %p\n",in2->data[0],res2->data[0]);
            // exit(0);
        // }
    }
}

void mTensorMergeForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Merge",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorMergeSet(layer);
    
    struct TensorMergePara *para = (struct TensorMergePara *)(layer->para);
    MTensor *in1 = para->prev1->tns;
    MTensor *in2 = para->prev2->tns;
    MTensor *out= layer->tns;
    
    int size = out->height*out->width;
    for(int b=0;b<out->batch;b++)
    {
        if(in1->data[b]!=out->data[b]) 
            memcpy(out->data[b]                  ,in1->data[b],in1->channel*size*sizeof(float));
        if(in2->data[b]!=out->data[b]+size*in1->channel)
            memcpy(out->data[b]+size*in1->channel,in2->data[b],in2->channel*size*sizeof(float));
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorMergeBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Merge",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorMergePara *para = (struct TensorMergePara *)(layer->para);
    MTensor *res1= para->prev1->res;
    MTensor *res2= para->prev2->res;
    MTensor *out= layer->res;
    
    int size = out->height*out->width;
    for(int b=0;b<out->batch;b++)
    {
        if((para->res_valid1==1)&&(res1->data[b]!=out->data[b]))
        {
            if(para->prev1->state == MORN_FORWARD)
                memcpy(res1->data[b],out->data[b],res1->channel*size*sizeof(float));
            else
                {for(int i=0;i<res1->channel*size;i++) res1->data[b][i]+=out->data[b][i];}
        }
        if((para->res_valid2==1)&&(res2->data[b]!=out->data[b]+size*res1->channel))
        {
            if(para->prev2->state == MORN_FORWARD)
                memcpy(res2->data[b],out->data[b]+size*res1->channel,res2->channel*size*sizeof(float));
            else
                {for(int i=0;i<res2->channel*size;i++) res2->data[b][i]+=out->data[b][size*res1->channel+i];}
        }
    }
    
    if(para->res_valid1==1) para->prev1->state = MORN_BACKWARD;
    if(para->res_valid2==1) para->prev2->state = MORN_BACKWARD;
}

struct TensorSplitPara
{
    MLayer *prev;
    int res_valid;
    int begin;
    int end;
};

void *mTensorSplitPara(MSheet *ini,char *name)
{
    struct TensorSplitPara *para = (struct TensorSplitPara *)mMalloc(sizeof(struct TensorSplitPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);

    value = mINIRead(ini,name,"channel");
    mException((value == NULL),EXIT,"invalid channel");
    para->begin = atoi(value);para->end = para->begin;
    MList *list = mStringSplit(value,":");
    mException((list->num>2),EXIT,"invalid channel para");
    if(list->num>1) para->end = atoi((char *)(list->data[1]));
    if(para->begin>para->end) {int buff=para->begin;para->begin=para->end;para->end=buff;}
    
    return para;
}

void TensorSplitSet(MLayer *layer)
{
    if(layer->state!=DFLT) return;
    struct TensorSplitPara *para = (struct TensorSplitPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    int channel = para->end-para->begin+1;
    
    int offset = para->begin*in->height*in->width;
    float **data = (float **)mMalloc(in->batch*sizeof(float *));
    
    for(int b=0;b<in->batch;b++)
        data[b] = in->data[b]+offset;
    mTensorRedefine(out,in->batch,channel,in->height,in->width,data);
    
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
    
    mFree(data);
}

void mTensorSplitForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Split",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorSplitSet(layer);
    
    struct TensorSplitPara *para = (struct TensorSplitPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;
    
    int size = out->height*out->width;
    int offset = para->begin*size;
    for(int b=0;b<out->batch;b++)
    {
        if(in->data[b]+offset!=out->data[b]) 
            memcpy(out->data[b],in->data[b]+offset,out->channel*size*sizeof(float));
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorSplitBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Split",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorSplitPara *para = (struct TensorSplitPara *)(layer->para);
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    
    int size = out->height*out->width;
    int offset = para->begin*size;
    for(int b=0;b<out->batch;b++)
    {
        if((para->res_valid==1)&&(res->data[b]!=out->data[b]))
        {
            if(para->prev->state == MORN_FORWARD)
            {
                if(para->begin>0) memset(res->data[b],0,para->begin*size*sizeof(float));
                memcpy(res->data[b]+offset,out->data[b],out->channel*size*sizeof(float));
                if(para->end<res->channel-1) memset(res->data[b]+offset+out->channel*size,0,(res->channel-1-para->end)*size*sizeof(float));
            }
            else
                {for(int i=0;i<out->channel*size;i++) res->data[b][offset+i]+=out->data[b][i];}
        }
    }
    
    if(para->res_valid==1) para->prev->state = MORN_BACKWARD;
}
