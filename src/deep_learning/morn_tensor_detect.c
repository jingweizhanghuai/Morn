#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_Tensor.h"
#include "morn_tensor_caculate.h"

float Detect(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;
    struct TensorOutputPara *para = layer->para;
    int group = in->channel/atoi(para->argv[0]);
    float thresh = atof(para->argv[1]);
    
    mException((INVALID_TENSOR(in)||INVALID_TENSOR(true)),EXIT,"invalid input");
    
    int size = in->height*in->width;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size*in->channel),EXIT,"invalid tensor");
    
    float sum=0.0;
    for(int b =0;b<in->batch;b++)
    {
        // printf("b is %d,group is %d\n",b,group);
        float err = 0.0;
        for(int c=0;c<in->channel;c++)
        {
            int l = (c/group)*group;
            
            
            float *ldata = true->data[b]+l*size;
            float *idata = in  ->data[b]+c*size;
            float *tdata = true->data[b]+c*size;
            
            // printf("c is %d,l is %d,ldata[10] is %f\n",c,l,ldata[10]);
            
            for(int i=0;i<size;i++)
                if((c%group==0)||(ldata[i]>thresh))
                {
                    float e = (idata[i] - tdata[i]);
                    err += e*e;
                    // printf("l is %d,ldata[i] is %f,tdata[i] is %f,e is %f,err is %f\n",l,ldata[i],tdata[i],e*e,err);
                }
        }
        // printf("err is %f\n",err);    
        err = err/2.0f;
        
        sum += err;
        if(error!=NULL) error[b]=err;
    }
    // printf("sum is %f\n",sum);
    return (sum/(float)(in->batch));
}

void D_Detect(MLayer *layer,MLayer *prev)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    struct TensorOutputPara *para = layer->para;
    int group = in->channel/atoi(para->argv[0]);
    float thresh = atof(para->argv[1]);
    
    int size = in->height*in->width;
    
    // printf("in->channel is %d,in->height is %d,in->width is %d\n",in->channel,in->height,in->width);
    // printf("in->data[0] is %p,in->data[0][10] is %f\n",in->data[0],in->data[0][10]);
    
    for(int b = 0;b<in->batch;b++)for(int c=0;c<in->channel;c++)
    {
        int l = (c/group)*group;
        
        float *ldata = true->data[b]+l*size;
        float *idata = in  ->data[b]+c*size;
        float *tdata = true->data[b]+c*size;
        float *rdata = res ->data[b]+c*size;
        
        for(int i=0;i<size;i++)
        {
            if((c%group==0)||(ldata[i]>thresh))
            {
                float data = (idata[i]-tdata[i]);
                rdata[i] = (prev->state==MORN_FORWARD)?data:(rdata[i]+data);
            }
            else if(prev->state==MORN_FORWARD)
                rdata[i]=0.0f;
        }
    }
}

float Split(MLayer *layer,MLayer *prev,float *error)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;
    mException((INVALID_TENSOR(in)||INVALID_TENSOR(true)),EXIT,"invalid input");
    
    int size = in->height*in->width*in->channel;
    mException((true->batch!=in->batch)||(true->height*true->width*true->channel!=size),EXIT,"invalid tensor");
    
    // printf("layer is %s,%p,true is %p\n",layer->name,layer,true);
    // printf("true->data[2] is %p\n",true->data[2]);
    
    float err=0.0;
    for(int b =0;b<in->batch;b++)
    {
        int sum1=0;int sum2=0;
        int err1=0;int err2=0;
        for(int i=0;i<size;i++)
        {
            if(true->data[b][i]<0.5f) {sum1+=1;if(in->data[b][i]>0.5f) err1+=1;}
            else                      {sum2+=1;if(in->data[b][i]<0.5f) err2+=1;}
        }
        if(sum1==0)      err+=(float)err2/(float)sum2;
        else if(sum2==0) err+=(float)err1/(float)sum1;
        else err += (((float)err1/(float)sum1+(float)err2/(float)sum2)/2.0f);
        printf("err1 is %f,err2 is %f\n",(float)err1/(float)sum1,(float)err2/(float)sum2);
    }
    
    return err/(in->batch);
}

void D_Split(MLayer *layer,MLayer *prev)
{
    MTensor *true = layer->tns;MTensor *in = prev->tns;MTensor *res=prev->res;
    
    int size = in->height*in->width*in->channel;
    for(int b =0;b<in->batch;b++)
    {
        int sum1=0,sum2=0;
        for(int i=0;i<size;i++)
        {
            if(true->data[b][i]>0.5f) sum1+=1;
            else                      sum2+=1;
        }
        float k=(sum2>0)?((float)sum1/(float)sum2):1.0f;
        // k=k*k;
        // printf("sum1 is %d,sum2 is %d,k is %f\n",sum1,sum2,k);
        
        
        for(int i=0;i<size;i++)
        {
            float data;
            if(true->data[b][i]>0.5f) data=(in->data[b][i]-1.0f);
            else                      data=(in->data[b][i]-0.0f)*k;
            // if(i<10)printf("%f(%f),",in->data[b][i],data);
                
            res->data[b][i] = (prev->state==MORN_FORWARD)?data:(res->data[b][i]+data);
        }
        // printf("\n");
    }
}
        
