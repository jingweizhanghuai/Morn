#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_Tensor.h"

void TensorSub(MTensor *in,MTensor *true,MTensor *res,int state)
{
    int size = in->channel*in->height*in->width;
    for(int b=0;b<in->batch;b++)
    {
        for(int i=0;i<size;i++)
        {
            register float data = (in->data[b][i]-true->data[b][i]);
            if((b==0)&&(i==25728))
                printf("\nccc true is %f,in is %f,res is %f,",true->data[b][i],in->data[b][i],data);
            // if(data>1.0f) data=1.0f; else if(data<-1.0f) data=-1.0f;
            res->data[b][i] = (state==MORN_FORWARD)?data:(res->data[b][i]+data);
            
        }
    }
}

float MSE(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;
    
    int i;
    mException((INVALID_TENSOR(in)||INVALID_TENSOR(true)),EXIT,"invalid input");
    
    int size = in->height*in->width*in->channel;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size),EXIT,"invalid tensor");
    
    float sum=0.0;
    for(int b = 0;b<in->batch;b++)
    {
        float *idata = in  ->data[b];
        float *tdata = true->data[b];
        
        // printf("idata is %f,%f,%f,%f\n",idata[0],idata[1],idata[2],idata[3]);
        // printf("tdata is %f,%f,%f,%f\n",tdata[0],tdata[1],tdata[2],tdata[3]);
        
        float err = 0.0;
        
        // #pragma omp parallel for
        for(i=0;i<size;i++)
        {
            float e = (idata[i] - tdata[i]);
            // printf("e is %f,e*e is %f\n",e,e*e);
            err += e*e;
        }
        err = err/2.0f;
        sum += err;
        if(error!=NULL) error[b]=err;
    }
    return (sum/(float)(in->batch));
}

void D_MSE(MLayer *layer,MLayer *prev)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    TensorSub(in,true,res,state);
    // printf("bbb true is %f,in is %f,res is %f\n",true->data[0][1000],in->data[0][1000],res->data[0][1000]);
}

float MAE(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;
    
    int i;
    mException((INVALID_TENSOR(in)||INVALID_TENSOR(true)),EXIT,"invalid input");
    
    int size = in->height*in->width*in->channel;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size),EXIT,"invalid tensor");
   
    float sum=0.0;
    for(int b = 0;b<in->batch;b++)
    {
        float *idata = in  ->data[b];
        float *tdata = true->data[b];
        
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
    MTensor *true = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    
    int size = in->height*in->width*in->channel;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size),EXIT,"invalid tensor");
    
    for(int b=0;b<in->batch;b++)
    {
        float *idata = in  ->data[b];
        float *tdata = true->data[b];
        for(int i=0;i<size;i++)
        {
            register float data = ((idata[i]>tdata[i])?1.0f:-1.0f);
            res->data[b][i] = (state==MORN_FORWARD)?data:(res->data[b][i]+data);
        }
    }
}

float Softmax(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;
    
    int i;
    mException(INVALID_TENSOR(in)||INVALID_TENSOR(true),EXIT,"invalid tensor");
    
    int size = in->height*in->width*in->channel;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size),EXIT,"invalid tensor");
    
    float sum = 0.0;
    
    // printf("size is %d\n",size);
    // float *e = mMalloc(size*sizeof(float));
    for(int b=0;b<in->batch;b++)
    {
        float *idata = in  ->data[b];
        float *tdata = true->data[b];
        
        float err = 0.0f;
        
        float max = idata[0];
        for(i=1;i<size;i++)
            max = MAX(max,idata[i]);
        
        float e_sum = 0.0f;
        // #pragma omp parallel for
        for(i=0;i<size;i++)
        {
            // if(b==2) printf("idata[i] is %f\n",idata[i]);
            
            idata[i] = exp((idata[i]-max)/100.0f);
            e_sum += idata[i];
        }
        // #pragma omp parallel for
        for(i=0;i<size;i++)
        {
            idata[i] = idata[i]/e_sum;
            
            // if((morn_network_time%1 == 0)&&(b == 0))
                // printf("idata[%d] is %f,e[i] is %f,e_sum is %f\n",i,idata[i],e[i],e_sum);
            
            if(tdata[i]==0.0f) continue;
            
            // if((morn_network_time%1 == 0)&&(b==0))
                // printf("aaaaaaaaaaa true is %d\n",i);
            
            err -= tdata[i]*log(idata[i]);
        }
        // printf("error[b] is %f\n",error[b]);
        sum += err;
        if(error!=NULL) error[b] = err;
    }
    // mFree(e);
    return (sum/(float)(in->batch));
}

void D_Softmax(MLayer *layer,MLayer *prev)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    TensorSub(in,true,res,state);
}

float Logistic(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;
    int i;
    mException(INVALID_TENSOR(in)||INVALID_TENSOR(true),EXIT,"invalid tensor");
   
    int size = in->height*in->width*in->channel;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size),EXIT,"invalid tensor");
    
    float sum = 0.0;

    float *e = mMalloc(size*sizeof(float));
    for(int b=0;b<in->batch;b++)
    {
        float *idata = in  ->data[b];
        float *tdata = true->data[b];
        
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
    MTensor *true = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    int state = prev->state;
    TensorSub(in,true,res,state);
}

struct LossRegister morn_loss_register[64];
int morn_loss_register_num = 0;

void mLossRegister(char *name,float (*loss)(MLayer *,MLayer *,float *),void (*dloss)(MLayer *,MLayer *))
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

