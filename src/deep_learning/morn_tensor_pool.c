/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_tensor.h"

struct TensorPoolPara
{
    MLayer *prev;
    
    int res_valid;
    
    int pool_height;
    int pool_width;
    
    int x_stride;
    int y_stride;
    
    int x0;
    int y0;
};
void *mTensorPoolPara(MFile *ini,char *name)
{
    struct TensorPoolPara *para = (struct TensorPoolPara *)mMalloc(sizeof(struct TensorPoolPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value=mINIRead(ini,name,"pool_height");if(value!=NULL) para->pool_height=atoi(value);else para->pool_height=1;
    value=mINIRead(ini,name,"pool_width" );if(value!=NULL) para->pool_width =atoi(value);else para->pool_width =1; 
    value=mINIRead(ini,name,"x_stride"   );if(value!=NULL) para->x_stride   =atoi(value);else para->x_stride   =1;
    value=mINIRead(ini,name,"y_stride"   );if(value!=NULL) para->y_stride   =atoi(value);else para->y_stride   =1;
    
    return para;
}

struct HandleTensorMaxPool
{
    MTensor *locate;
};
void endTensorMaxPool(void *info)
{
    struct HandleTensorMaxPool *handle = (struct HandleTensorMaxPool *)info;
    if(handle->locate!=NULL) mTensorRelease(handle->locate);
}
#define HASH_TensorMaxPool 0x9917de8c

void TensorMaxPoolSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    mException(strcmp("MaxPool",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
   
    int out_height = in->height/para->y_stride;
    int out_width  = in->width /para->x_stride;
    mException((out_height<1)||(out_width<1),EXIT,"invalid stride");
    
    para->x0=(in->width -(out_width -1)*para->x_stride)/2;
    para->y0=(in->height-(out_height-1)*para->y_stride)/2;
    
    mTensorRedefine(out,in->batch,in->channel,out_height,out_width,NULL);
    if(morn_network_flag == MORN_TRAIN)
        mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);

    #ifndef MORN_USE_CL
    if(morn_network_flag == MORN_TRAIN)
    #endif
    {
        MHandle *hdl=mHandle(out,TensorMaxPool);
        struct HandleTensorMaxPool *handle = (struct HandleTensorMaxPool *)(hdl->handle);
        
        if(handle->locate != NULL) mTensorRelease(handle->locate);
        handle->locate =mTensorCreate(in->batch,in->channel,out_height,out_width,NULL,out->device);
        
        hdl->valid = 1;
    }
}

#ifdef MORN_USE_CL
char *tensor_max_pool_forward = mString(
__kernel void tensor_max_pool_forward(__global const float* in,__global int *locate,__global float* out,int in_height,int in_width,int pool_height,int pool_width,int y_stride,int x_stride,int y0,int x0)
{
    const int height= get_global_size(1);
    const int width = get_global_size(2);
    const int c = get_global_id(0);
    const int y = get_global_id(1);
    const int x = get_global_id(2);
    const int idx = c*height*width+y*width+x;
    const int in_idx = c*in_height*in_width;
    
    const int h = y*y_stride+y0;
    const int w = x*x_stride+x0;

    out[idx]=-1000000;
    for(int j=h;j<h+pool_height;j++)
    {
        if((j>=0)&&(j<in_height))
        {
            for(int i=w;i<w+pool_width;i++)
            {
                if((i>=0)||(i<in_width))
                {
                    int l = j*in_width+i;
                    if(in[in_idx+l]>out[idx])
                    {
                        locate[idx] = l;
                           out[idx] = in[in_idx+l];
                    }
                }
            }
        }
    }
});
char *tensor_max_pool_backward = mString(
__kernel void tensor_max_pool_backward(__global const float* out,__global const int *locate,__global float *res,int res_height,int res_width)
{
    const int height= get_global_size(1);
    const int width = get_global_size(2);
    const int c = get_global_id(0);
    const int y = get_global_id(1);
    const int x = get_global_id(2);
    const int idx = c*height*width+y*width+x;
    const int res_idx = c*res_height*res_width+locate[idx];
    res[res_idx]=out[idx];
});
#endif

void mTensorMaxPoolForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out= layer->tns;

    TensorMaxPoolSet(layer);
    
    int in_height = in->height;int in_width = in->width;
    
    int pool_height = para->pool_height;
    int pool_width  = para->pool_width ;
    
    int in_size =  in_height*  in_width;
    int out_size=out->height*out->width;

    // MMemoryBlock *mem = mTensorMemory(out,0);
    // printf("mem->size = %d\n",mem->size);
    // printf("out_size=%d\n",out_size);
    int channel= in->channel;
    struct HandleTensorMaxPool *handle=NULL;
    
    #ifdef MORN_USE_CL
        MHandle *hdl=mHandle(out,TensorMaxPool);
        handle = (struct HandleTensorMaxPool *)(hdl->handle);
        MTensor *locate = handle->locate;
        for(int b=0;b<in->batch;b++)
        {
            mCLFunction(tensor_max_pool_forward,CLSIZE(channel,out->height,out->width),CLIN(mTensorMemory(in,b)),CLOUT(mTensorMemory(locate,b)),CLOUT(mTensorMemory(in,b)),
                CLPARA(&in_height,sizeof(int)),CLPARA(&in_width,sizeof(int)),CLPARA(&pool_height,sizeof(int)),CLPARA(&pool_width,sizeof(int)),
                CLPARA(&(para->y_stride),sizeof(int)),CLPARA(&(para->x_stride),sizeof(int)),CLPARA(&(para->y0),sizeof(int)),CLPARA(&(para->x0),sizeof(int)));
        }
    #else
        if(morn_network_flag == MORN_TRAIN)
        {
            MHandle *hdl=mHandle(out,TensorMaxPool);
            handle = (struct HandleTensorMaxPool *)(hdl->handle);
        }
        for(int b=0;b<in->batch;b++)for(int c=0;c<channel;c++)
        {
            int *p_locate = NULL;
            if(morn_network_flag == MORN_TRAIN)
                p_locate=(int *)(handle->locate->data[b])+c*out_size;
            float *p_out =out->data[b]+c*out_size;
            float *p_in  = in->data[b]+c* in_size;
        
            int m=0;
            for(int h=para->y0-pool_height/2;h<in_height-pool_height/2;h+=para->y_stride)
            for(int w=para->x0-pool_width /2;w<in_width -pool_width /2;w+=para->x_stride)
            {
                int l = h*in_width+w;
                p_out[m] = p_in[l];
                
                for(int j=h;j<h+pool_height;j++)
                {
                    if((j<0)||(j>=in_height)) continue;
                    for(int i=w;i<w+pool_width;i++)
                    {
                        if((i<0)||(i>=in_width)) continue;
                        if(p_in[j*in_width+i]>p_out[m])
                        {
                            l = j*in_width+i;
                            p_out[m] = p_in[l];
                        }
                    }
                }
                
                if(p_locate!=NULL) p_locate[m] = l;
                m=m+1; 
            }
        }
    #endif

    // printf("mem->data[%d]=%f\n",out_size*channel,((float *)(mem->data))[out_size*channel]);
    // exit(0);
    // printf("\nout=\n");for(int ii=300;ii<500;ii++) printf("%f,",out->data[0][ii]);
    
    layer->state = MORN_FORWARD;
}

void mTensorMaxPoolBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    if(para->res_valid==0) return;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    int in_height =res->height;int in_width =res->width;
    int in_size =  in_height* in_width;
    int out_size=out->height*out->width;
    
    int channel=res->channel;
   
    MHandle *hdl=mHandle(layer->tns,TensorMaxPool);
    struct HandleTensorMaxPool *handle = (struct HandleTensorMaxPool *)(hdl->handle);
    mException((hdl->valid == 0),EXIT,"no forward operate");
    MTensor *locate = handle->locate;
    
    if(para->prev->state == MORN_FORWARD)
    {
        for(int b=0;b<res->batch;b++)
        {
            MMemoryBlock *mem = mTensorMemory(res,b);
            memset(res->data[b],0,res->height*res->width*res->channel*sizeof(float));
            mem->flag = MORN_HOST;
        }
        para->prev->state = MORN_BACKWARD;
    }

    // printf("\nmaxpooldelta=\n");for(int ii=200;ii<400;ii++) printf("%f,",out->data[0][ii]);
    #ifdef MORN_USE_CL
    for(int b=0;b<res->batch;b++)
    {
        mCLFunction(CLSIZE(channel,out->height,out->width),CLIN(mTensorMemory(out,b)),CLIN(mTensorMemory(locate,b)),CLOUT(mTensorMemory(res,b)),
            CLPARA(&in_height,sizeof(int)),CLPARA(&in_width,sizeof(int)));
    }
    #else
    for(int b=0;b<res->batch;b++)for(int c=0;c<channel;c++)
    {
        int *p_locate=(int *)(locate->data[b])+c*out_size;;
        float *p_out =out->data[b]+c*out_size;
        float *p_res =res->data[b]+c* in_size;
        
        int m=0;
        for(int h=para->y0;h<in_height;h+=para->y_stride)for(int w=para->x0;w<in_width;w+=para->x_stride)
        {
            p_res[p_locate[m]] += p_out[m];
            m=m+1;
        }
    }
    #endif
    // printf("\nmaxpooldeltaout=\n");for(int ii=200;ii<400;ii++) printf("%f,",res->data[0][ii]);
}

void TensorAvgPoolSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    mException(strcmp("AvgPool",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    int out_height = in->height/para->y_stride;
    int out_width  = in->width /para->x_stride;
    
    para->x0=(in->width -(out_width -1)*para->x_stride)/2;
    para->y0=(in->height-(out_height-1)*para->y_stride)/2;
    
    mTensorRedefine(out,in->batch,in->channel,out_height,out_width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
}

void mTensorAvgPoolForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorAvgPoolSet(layer);
    
    int in_height = in->height;int in_width = in->width;
    int in_size  = in_height*in_width;
    
    int out_size =(in_height/para->y_stride)*(in_width/para->x_stride);
    
    int pool_height=para->pool_height;int pool_width =para->pool_width;
    float pool_size=(float)(pool_height*pool_width);
    
    int channel= in->channel;

    for(int b=0;b<in->batch;b++)for(int c=0;c<channel;c++)
    {
        float *p_out =out->data[b] +c*out_size;
        float *p_in  = in->data[b] +c* in_size;

        int m=0;
        int x,y;
        for(int h=para->y0-pool_height/2;h<in_height-pool_height/2;h+=para->y_stride)
        for(int w=para->x0-pool_width /2;w<in_width -pool_width /2;w+=para->x_stride)
        {
            float sum=0.0;
            for(int j=h;j<h+pool_height;j++)
            {
                if(j<0) y=0;else if(j>in_height) y=in_height-1;else y=j;
                for(int i=w;i<w+pool_width;i++)
                {
                    if(i<0) x=0;else if(i>in_width) x=in_width-1;else x=i;
                    sum+=p_in[y*in_width+x];
                }
            }
            p_out[m] = sum/pool_size;
            m=m+1; 
        }
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorAvgPoolBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    if(para->res_valid==0) return;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    int in_height = res->height;int in_width = res->width;
    int in_size  = in_height*in_width;
    int out_size =out->height*out->width;
    
    int channel= out->channel;
    
    int pool_height=para->pool_height;int pool_width =para->pool_width;
    float pool_size=(float)(pool_height*pool_width);
    
    
    if(para->prev->state == MORN_FORWARD)
    {
        for(int b=0;b<res->batch;b++)
            memset(res->data[b],0,in_size*res->channel*sizeof(float));
        para->prev->state = MORN_BACKWARD;
    }
    
    for(int b=0;b<out->batch;b++)for(int c=0;c<channel;c++)
    {
        float *p_out=out->data[b]+c*out_size;
        float *p_res=res->data[b]+c* in_size;
        int m=0;int x,y;
        for(int h=para->y0-pool_height/2;h<in_height-pool_height/2;h+=para->y_stride)
        for(int w=para->x0-pool_width /2;w<in_width -pool_width /2;w+=para->x_stride)
        {
            for(int j=h;j<h+pool_height;j++)
            {
                if(j<0) y=0;else if(j>in_height) y=in_height-1;else y=j;
                for(int i=w;i<w+pool_width;i++)
                {
                    if(i<0) x=0;else if(i>=in_width) x=in_width-1;else x=i;
                    p_res[y*in_width+x] += p_out[m]/pool_size;
                }
            }
            m=m+1;
        }
    }
}

struct HandleTensorRandPool
{
    int *locate;
};
void endTensorRandPool(void *info)
{
    struct HandleTensorRandPool *handle = (struct HandleTensorRandPool *)info;
    if(handle->locate!=NULL) mFree(handle->locate);
}
#define HASH_TensorRandPool 0xc4caa999

void TensorRandPoolSet(MLayer *layer)
{
    if(layer->state !=DFLT) return;
    mException(strcmp("RandPool",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    int out_height = in->height/para->y_stride;
    int out_width  = in->width /para->x_stride;
    
    para->x0=(in->width -(out_width -1)*para->x_stride)/2;
    para->y0=(in->height-(out_height-1)*para->y_stride)/2;
    
    mTensorRedefine(out,in->batch,in->channel,out_height,out_width,NULL);
    
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    
        MHandle *hdl=mHandle(out,TensorRandPool);
        struct HandleTensorRandPool *handle = (struct HandleTensorRandPool *)(hdl->handle);
    
        int data_size = in->batch*in->channel*out_height*out_width;
        if(handle->locate != NULL) mFree(handle->locate);
        handle->locate =(int *)mMalloc(data_size*sizeof(int));
        hdl->valid = 1;
    }
}

void mTensorRandPoolForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;

    TensorRandPoolSet(layer);
    
    int in_height = in->height;int in_width = in->width;
    int in_size  =  in_height*  in_width;
    int out_size =out->height*out->width;
    
    int pool_height= para->pool_height;
    int pool_width = para->pool_width ;
    float pool_size=(float)(pool_height*pool_width);
    
    int channel= in->channel;
    
    MHandle *hdl=mHandle(out,TensorRandPool);
    struct HandleTensorRandPool *handle = (struct HandleTensorRandPool *)(hdl->handle);
    
    for(int b=0;b<in->batch;b++)for(int c=0;c<channel;c++)
    {
        int *p_locate = NULL;
        if(morn_network_flag == MORN_TRAIN)
            p_locate = handle->locate + b*channel*out_size+c*out_size;
            
        float *p_out =out->data[b]+c*out_size;
        float *p_in  = in->data[b]+c* in_size;
        int m=0;
        for(int h=para->y0-pool_height/2;h<in_height-pool_height/2;h+=para->y_stride)
        for(int w=para->x0-pool_width /2;w<in_width -pool_width /2;w+=para->x_stride)
        {
            int rand = mRand(0,pool_size);
            int j = rand/pool_width + h;if(j<0) j=0; else if(j>=in_height) j=in_height-1;
            int i = rand%pool_width + w;if(i<0) i=0; else if(i>=in_width ) i=in_width -1;
            if(p_locate!=NULL) p_locate[m] = j*in_width+i;
            p_out[m] = p_in[j*in_width+i];
            m=m+1; 
        }
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorRandPoolBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorPoolPara *para = (struct TensorPoolPara *)(layer->para);
    if(para->res_valid==0) return;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    int in_height = res->height;int in_width = res->width;
    int in_size =  in_height*  in_width;
    int out_size=out->height*out->width;
    
    int channel= out->channel;
    
    MHandle *hdl=mHandle(layer->tns,TensorRandPool);
    struct HandleTensorRandPool *handle = (struct HandleTensorRandPool *)(hdl->handle);
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    if(para->prev->state == MORN_FORWARD)
    {
        for(int b=0;b<res->batch;b++)
            memset(res->data[b],0,res->height*res->width*res->channel*sizeof(float));
        para->prev->state = MORN_BACKWARD;
    }
    
    for(int b=0;b<out->batch;b++)for(int c=0;c<channel;c++)
    {
        int *p_locate=handle->locate + b*channel*out_size+c*out_size;
        float *p_out =out->data[b]+c*out_size;
        float *p_res =res->data[b]+c* in_size;
        
        int m=0;
        for(int h=para->y0;h<in_height;h+=para->y_stride)for(int w=para->x0;w<in_width;w+=para->x_stride)
        {
            p_res[p_locate[m]] += p_out[m];
            m=m+1;
        }
    }
}
