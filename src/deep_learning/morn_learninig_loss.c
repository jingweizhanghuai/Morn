/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_tensor.h"

void TensorSub(MTensor *in,MTensor *tv,MTensor *res,int state)
{
    int size = in->channel*in->height*in->width;
    if(state==MORN_FORWARD)
    {
        for(int b=0;b<in->batch;b++)for(int i=0;i<size;i++)
            res->data[b][i] = (in->data[b][i]-tv->data[b][i]);
    }
    else
    {
        for(int b=0;b<in->batch;b++)for(int i=0;i<size;i++)
            res->data[b][i]+= (in->data[b][i]-tv->data[b][i]);
    }
    
}

float MSE(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;
    
    int i;
    mException((INVALID_TENSOR(in)||INVALID_TENSOR(tv)),EXIT,"invalid input");
    
    int size = in->height*in->width*in->channel;
    mException((tv->batch!=in->batch)||(tv->height*tv->width*tv->channel!=size),EXIT,"invalid tensor");
    
    float sum=0.0;
    for(int b = 0;b<in->batch;b++)
    {
        float *idata = in->data[b];
        float *tdata = tv->data[b];
        
        float err = 0.0;
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
            err += (idata[i] - tdata[i])*(idata[i] - tdata[i]);
        
        err = err/2.0f;
        sum += err;
        if(error!=NULL) error[b]=err;
    }
    return (sum/(float)(in->batch));
}

void D_MSE(MLayer *layer,MLayer *prev)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    TensorSub(in,tv,res,state);
}

float MAE(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;
    
    int i;
    mException((INVALID_TENSOR(in)||INVALID_TENSOR(tv)),EXIT,"invalid input");
    
    int size = in->height*in->width*in->channel;
    mException((tv->batch!=in->batch)||(tv->height*tv->width*tv->channel!=size),EXIT,"invalid tensor");
   
    float sum=0.0;
    for(int b = 0;b<in->batch;b++)
    {
        float *idata = in->data[b];
        float *tdata = tv->data[b];
        
        float err = 0.0;
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
            err+=(idata[i] - tdata[i]);
        
        sum += err;
        if(error!=NULL) error[b]=err;
    }
    return (sum/(float)(in->batch));
}

void D_MAE(MLayer *layer,MLayer *prev)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    
    int size = in->height*in->width*in->channel;
    mException((tv->batch!=in->batch)||(tv->height*tv->width*tv->channel!=size),EXIT,"invalid tensor");
    
    for(int b=0;b<in->batch;b++)
    {
        float *idata = in->data[b];
        float *tdata = tv->data[b];
        for(int i=0;i<size;i++)
        {
            register float data = ((idata[i]>tdata[i])?1.0f:-1.0f);
            res->data[b][i] = (state==MORN_FORWARD)?data:(res->data[b][i]+data);
        }
    }
}

float Softmax(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;
    
    int i;
    mException(INVALID_TENSOR(in)||INVALID_TENSOR(tv),EXIT,"invalid tensor");
    
    int size = in->height*in->width*in->channel;
    mException((tv->batch!=in->batch)||(tv->height*tv->width*tv->channel!=size),EXIT,"invalid tensor");
    
    float sum = 0.0;
    
    for(int b=0;b<in->batch;b++)
    {
        float *idata = in->data[b];
        float *tdata = tv->data[b];
        
        float err = 0.0f;
        
        float max = idata[0];
        for(i=1;i<size;i++)
            max = MAX(max,idata[i]);
        
        float e_sum = 0.0f;
        // #pragma omp parallel for
        for(i=0;i<size;i++)
        {
            idata[i] = exp((idata[i]-max)/1.0f);
            // printf("idata=%f\n",idata[i]);
            e_sum += idata[i];
        }
        // printf("e_sum=%f\n",e_sum);
        
        // #pragma omp parallel for
        for(i=0;i<size;i++)
        {
            idata[i] = idata[i]/e_sum;
            // printf("idata=%f\n",idata[i]);
            if(tdata[i]!=0.0f) err -= tdata[i]*log(idata[i]);
        }
        sum += err;
        if(error!=NULL) error[b] = err;
    }
    return (sum/(float)(in->batch));
}

void D_Softmax(MLayer *layer,MLayer *prev)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    // printf("aaaaaaaaaaaaa in=%p,res=%p\n",in->data[0],res->data[0]);
    // int state = prev->state;
    // int size = in->channel*in->height*in->width;
    // for(int b=0;b<in->batch;b++)for(int i=0;i<size;i++)
    // {
    //     register float data = 2*(in->data[b][i]-tv->data[b][i]);
    //     res->data[b][i] = (state==MORN_FORWARD)?data:(res->data[b][i]+data);
        
    //     printf("res->data[b][i]=%f\n",res->data[b][i]);
    // }
    TensorSub(in,tv,res,prev->state);
}

float Logistic(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;
    int i;
    mException(INVALID_TENSOR(in)||INVALID_TENSOR(tv),EXIT,"invalid tensor");
   
    int size = in->height*in->width*in->channel;
    mException((tv->batch!=in->batch)||(tv->height*tv->width*tv->channel!=size),EXIT,"invalid tensor");
    
    float sum = 0.0;

    float *e = (float *)mMalloc(size*sizeof(float));
    for(int b=0;b<in->batch;b++)
    {
        float *idata = in->data[b];
        float *tdata = tv->data[b];
        
        float err = 0.0f;
        
        #pragma omp parallel for
        for(i=0;i<size;i++)
        {
            float out = 1.0f/(1.0f+exp(0.0f-idata[i]));
            err -= (tdata[i]==1)?log(out):log(1.0f-out);
        }
        sum += err;
        if(error!=NULL) error[b]=err;
    }
    mFree(e);
    return (sum/(float)(in->batch));
}
void D_Logistic(MLayer *layer,MLayer *prev)
{
    MTensor *tv = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    TensorSub(in,tv,res,state);
}

struct LossRegister morn_loss_register[64];
int morn_loss_register_num = 0;

void mLossRegister(const char *name,float (*loss)(MLayer *,MLayer *,float *),void (*dloss)(MLayer *,MLayer *))
{
    int n = morn_loss_register_num;
    morn_loss_register_num = n+1;
    
    strcpy(morn_loss_register[n].name,name);
    morn_loss_register[n].loss = loss;
    morn_loss_register[n].dloss= dloss;
}

void mLossRegisterAll()
{
    mLossRegister("MSE"     ,MSE     ,D_MSE     );
    mLossRegister("MAE"     ,MAE     ,D_MAE     );
    mLossRegister("Softmax" ,Softmax ,D_Softmax );
    mLossRegister("Logistic",Logistic,D_Logistic);
}

