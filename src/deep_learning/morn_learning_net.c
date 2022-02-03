/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_tensor.h"

char *morn_network_para_dir = NULL;
char morn_network_para_filename[256];
MFile *morn_network_parafile = NULL;

int morn_network_time = 0;
int morn_network_save = 500;
int morn_network_time_max=100000;

float morn_network_error;
float morn_network_error_thresh = 0.01;

struct TensorRegister morn_tensor_register[256];
int morn_tensor_register_num = 0;

void mTensorRegister(const char *type,void *(*para)(MList *,char *),void (*forward)(MLayer *),void (*backward)(MLayer *))
{
    int n = morn_tensor_register_num;
    for(int i=0;i<n;i++)
    {
        if(strcmp(morn_tensor_register[n].type,type)==0)
            return;
    }
    morn_tensor_register_num = n+1;
    strcpy(morn_tensor_register[n].type,type);
    morn_tensor_register[n].para    = para;
    morn_tensor_register[n].forward = forward;
    morn_tensor_register[n].backward= backward;
}

void mTensorRegisterAll()
{
    mTensorRegister("Input"     ,mTensorInputPara     ,mTensorInputForward     ,mTensorInputBackward     );
    mTensorRegister("Output"    ,mTensorOutputPara    ,mTensorOutputForward    ,mTensorOutputBackward    );
    mTensorRegister("Connect"   ,mTensorConnectPara   ,mTensorConnectForward   ,mTensorConnectBackward   );
    mTensorRegister("Conv"      ,mTensorConvPara      ,mTensorConvForward      ,mTensorConvBackward      );
    mTensorRegister("MaxPool"   ,mTensorPoolPara      ,mTensorMaxPoolForward   ,mTensorMaxPoolBackward   );
    mTensorRegister("AvgPool"   ,mTensorPoolPara      ,mTensorAvgPoolForward   ,mTensorAvgPoolBackward   );
    mTensorRegister("Activation",mTensorActivationPara,mTensorActivationForward,mTensorActivationBackward);
    mTensorRegister("BatchNorm" ,mTensorBatchNormPara ,mTensorBatchNormForward ,mTensorBatchNormBackward );
    mTensorRegister("Merge"     ,mTensorMergePara     ,mTensorMergeForward     ,mTensorMergeBackward     );
    mTensorRegister("Resize"    ,mTensorResizePara    ,mTensorResizeForward    ,mTensorResizeBackward    );
    mTensorRegister("Reshape"   ,mTensorReshapePara   ,mTensorReshapeForward   ,mTensorReshapeBackward   );
    mTensorRegister("Mul"       ,mTensorMulPara       ,mTensorMulForward       ,mTensorMulBackward       );
}

int mTensorRegisterIndex(const char *type)
{
    for(int i=0;i<morn_tensor_register_num;i++)
    {
        if(strcmp(morn_tensor_register[i].type,type)==0)
            return i;
    }
    mException(1,EXIT,"no tensor operator named %s",type);
    return DFLT;
}

int GetLayerIndex(MList *net,char *name)
{
    for(int i=0;i<net->num;i++)
    {
        MLayer *layer = (MLayer *)(net->data[i]);
        if(strcmp(layer->name,name)==0)
            return i;
    }
    mException(1,EXIT,"no layer named %s",name);
    return DFLT;
}

void NetworkPara(MList *ini)
{
    char *value;
    
    morn_network_para_dir = mINIRead(ini,"para","para_dir");
    mException((morn_network_para_dir==NULL),EXIT,"no parameter dir");
    
    value = mINIRead(ini,"para","para_file");
    mException((value==NULL)&&(morn_network_flag==MORN_PREDICT),EXIT,"no parameter file");
    if(value!=NULL)
    {
        sprintf(morn_network_para_filename,"%s/%s",morn_network_para_dir,value);
        morn_network_parafile = mFileCreate(morn_network_para_filename);
    }
    
    if(morn_network_flag == MORN_PREDICT) return;
    
    value = mINIRead(ini,"para","time_max");
    if(value != NULL) morn_network_time_max= atoi(value);
    
    value = mINIRead(ini,"para","time_save");
    if(value != NULL) morn_network_save = atoi(value);
    
    value = mINIRead(ini,"para","error_thresh");
    if(value != NULL) morn_network_error_thresh = atof(value);
}

struct HandleNetworkGenerate
{
    MList *net;
};
void endNetworkGenerate(void *info)
{
    if(morn_network_parafile != NULL) mFileRelease(morn_network_parafile);
    
    struct HandleNetworkGenerate *handle = (struct HandleNetworkGenerate *)info;
    MList *list = handle->net;
    if(list==NULL) return;
    
    for(int i=0;i<list->num;i++)
    {
        MLayer *layer = (MLayer *)(list->data[i]);
        if(layer->para!=NULL) mFree(layer->para);
        if(layer->tns !=NULL) mTensorRelease(layer->tns);
        if(layer->res !=NULL) mTensorRelease(layer->res);
    }
    
    mListRelease(handle->net);
}
#define HASH_NetworkGenerate 0x589a0934

MList *mNetworkGenerate(MList *ini)
{
    MHandle *hdl=mHandle(ini,NetworkGenerate);
    struct HandleNetworkGenerate *handle = (struct HandleNetworkGenerate *)(hdl->handle);
    if(hdl->valid == 1) return (handle->net);
    
    mActivationRegisterAll();
    mTensorRegisterAll();
    mLossRegisterAll();
    
    if(handle->net == NULL) handle->net = mListCreate(DFLT,NULL);
    
    MLayer layer_buff; MLayer *layer = &layer_buff;
    
    mListClear(handle->net);
    for(int j=0;j<ini->num;j++)
    {
        if(strcmp((char *)(ini->data[j]),"para")==0) continue;
        strcpy(layer->name,(char *)(ini->data[j]));
    
        layer->state = DFLT;
        
        char *type = mINIRead(ini,layer->name,"type");
        layer->type_index = mTensorRegisterIndex(type);
        
        layer->tns = mTensorCreate(DFLT,DFLT,DFLT,DFLT,NULL);
        layer->res = mTensorCreate(DFLT,DFLT,DFLT,DFLT,NULL);
        
        mListWrite(handle->net,DFLT,layer,sizeof(MLayer));
    }
    hdl->valid =1;
    
    for(int j=0;j<handle->net->num;j++)
    {
        layer = (MLayer *)(handle->net->data[j]);
        layer->para= (morn_tensor_register[layer->type_index].para)(ini,layer->name);
    }

    NetworkPara(ini);
    return (handle->net);
}

MLayer *mNetworkLayer(MList *ini,char *name)
{
    if(name==NULL) return NULL;
    MList *net = mNetworkGenerate(ini);
    
    for(int i=0;i<net->num;i++)
    {
        MLayer *layer = (MLayer *)(net->data[i]);
        if(strcmp(layer->name,name)==0)
            return layer;
    }
    return NULL;
}

void mNetworkForward(MList *net)
{
    MLayer **layer = (MLayer **)(net->data);
    for(int i=0;i<net->num;i++)
    {
        // printf("layer name is %s\n",layer[i]->name);
        (morn_tensor_register[layer[i]->type_index].forward)(layer[i]);
    }
}

void mNetworkBackward(MList *net)
{
    MLayer **layer = (MLayer **)(net->data);
    
    for(int i=net->num-1;i>=0;i--)
    {
        // printf("layer name is %s\n",layer[i]->name);
        (morn_tensor_register[layer[i]->type_index].backward)(layer[i]);
    }
}

int morn_network_flag = MORN_PREDICT;

void mDeeplearningTrain(MFile *file)
{
    morn_network_flag = MORN_TRAIN;
    MList *ini = mINILoad(file);
    
    MList *net = mNetworkGenerate(ini);
    
    for(morn_network_time=0;morn_network_time<=morn_network_time_max;morn_network_time++)
    {
        mTrainData(ini);
        mNetworkForward(net);
        mNetworkBackward(net);
        mLog(MORN_INFO,"%05d:error is %f\n",morn_network_time,morn_network_error);
        if(morn_network_error <= morn_network_error_thresh) break;
    }
}

void mNetworkTrain(MList *ini,char *name[],MTensor *tns[])
{
    morn_network_flag = MORN_TRAIN;
    MList *net = mNetworkGenerate(ini);

    mNetworkTensor(ini,name,tns);
    mNetworkForward(net);
    mNetworkBackward(net);
    
    morn_network_time+=1;
}
 
void mNetworkPredict(MList *ini,char *name[],MTensor *tns[])
{
    morn_network_flag = MORN_PREDICT;
    MList *net = mNetworkGenerate(ini);
    
    mNetworkTensor(ini,name,tns);
    mNetworkForward(net);
}

/*
void LayerState(MLayer *layer)
{
    int size = layer->tns->height*layer->tns->width*layer->tns->channel;
    
    float sum,sum2,max,min;
    float mean,delta;
    float *data;
    
    data = layer->tns->data[0];
    sum=0;sum2=0;max=data[0];min=data[0];
    for(int i=0;i<size;i++)
    {
        sum+=data[i];
        sum2+=data[i]*data[i];
        min = MIN(min,data[i]);
        max = MAX(max,data[i]);
    }
    mean = sum/size;
    delta = sum2/size-mean*mean;
    
    printf("%s:tns mean is %f,delta is %f,min is %f,max is %f\n",layer->name,mean,delta,min,max);
    
    size = layer->res->height*layer->res->width*layer->res->channel;
    if(size<=0) return;
    
    data = layer->res->data[0];
    sum=0;sum2=0;max=data[0];min=data[0];
    for(int i=0;i<size;i++)
    {
        sum+=data[i];
        sum2+=data[i]*data[i];
        min = MIN(min,data[i]);
        max = MAX(max,data[i]);
    }
    mean = sum/size;
    delta = sum2/size-mean*mean;
    
    printf("%s:res mean is %f,delta is %f,min is %f,max is %f\n",layer->name,mean,delta,min,max);
}
*/



    