#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cblas.h>
#include "morn_Tensor.h"

void DeconvTensorToMatData(MTensor *tns,int bc,float *mdata,int knl_height,int knl_width,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width*x_stride;
    int out_height=height*y_stride;
    int mwidth = knl_height*knl_width*channel+1;
    int mheight= out_height*out_width;
    
    memset(mdata,0,mwidth*mheight*sizeof(float));
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int i,j,c;
    for(j=0;j<mheight;j++)
    {
        int n=j/out_width-knl_height/2;
        int m=j%out_width-knl_width/2;
        for(i=0;i<mwidth-1;i+=channel)
        {
            int h= i/(knl_width*channel)         +n;if(h%y_stride!=0) continue;h=h/y_stride;
            int w=(i%(knl_width*channel))/channel+m;if(w%x_stride!=0) continue;w=w/x_stride;
            
            if(h<0)h=0;else if(h>=height)h=height-1;
            if(w<0)w=0;else if(w>= width)w= width-1;
            
            for(c=0;c<channel;c++)
                mdata[(j*mwidth)+i+c]=tdata[c*tsize+h*width+w];
        }
        mdata[(j*mwidth)+mwidth-1]=1.0f;
    }
}

void DeconvMatDataToTensor(float *mdata,MTensor *tns,int bc,int knl_height,int knl_width,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width*x_stride;
    int out_height=height*y_stride;
    
    int mwidth = knl_height*knl_width*channel+1;
    int mheight= out_height*out_width;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int i,j,c;
    for(j=0;j<mheight;j++)
    {
        int n=j/out_width-knl_height/2;
        int m=j%out_width-knl_width/2;
        for(i=0;i<mwidth-1;i+=channel)
        {
            int h= i/(knl_width*channel)         +n;if(h%y_stride!=0) continue;h=h/y_stride;
            int w=(i%(knl_width*channel))/channel+m;if(w%x_stride!=0) continue;w=w/x_stride;
            
            if(h<0)h=0;else if(h>=height)h=height-1;
            if(w<0)w=0;else if(w>= width)w= width-1;
            
            for(c=0;c<channel;c++)
                tdata[c*tsize+h*width+w]+=mdata[(j*mwidth)+i+c];
        }
        mdata[(j*mwidth)+mwidth-1]=1.0f;
    }
}

struct TensorDeconvPara
{
    MLayer *prev;
    
    int knl_num;
    int knl_height;
    int knl_width;
    
    int x_stride;
    int y_stride;
    
    int res_valid;
    
    float rate;
    float decay;
    float momentum;
};
void *mTensorDeconvPara(MFile *ini,char *name)
{
    struct TensorDeconvPara *para = mMalloc(sizeof(struct TensorDeconvPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"knl_num");
    if(value != NULL) para->knl_num= atoi(value);else para->knl_num= 1; 
    
    value = mINIRead(ini,name,"knl_height");
    if(value != NULL) para->knl_height= atoi(value);else para->knl_height= 1; 
    
    value = mINIRead(ini,name,"knl_width");
    if(value != NULL) para->knl_width= atoi(value);else para->knl_width= 1; 
    
    value = mINIRead(ini,name,"x_stride");
    if(value != NULL) para->x_stride= atoi(value);else para->x_stride= 1;
    
    value = mINIRead(ini,name,"y_stride");
    if(value != NULL) para->y_stride= atoi(value);else para->y_stride= 1;
    
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
        
struct HandleTensorDeconv
{
    float *mat;
    float *kernel;
    float *update;
};
void endTensorDeconv(void *info)
{
    struct HandleTensorDeconv *handle = info;
    if(handle->mat   != NULL) mFree(handle->mat);
    if(handle->kernel!= NULL) mFree(handle->kernel);
    if(handle->update!= NULL) mFree(handle->update);
}
#define HASH_TensorDeconv 0x9087d39c
void TensorDeconvSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorDeconvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorDeconv,hdl);
    struct HandleTensorDeconv *handle = hdl->handle;
    
    int out_height= in->height*para->y_stride;
    int out_width = in->width *para->x_stride;
    // printf("out_height is %d,out_width is %d\n",out_height,out_width);
    int mheight = (out_height*out_width);
    int mwidth = para->knl_height*para->knl_width*in->channel+1;
    int data_size = para->knl_num*mwidth;
    
    mTensorRedefine(out,in->batch,para->knl_num,out_height,out_width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);

        if(handle->update != NULL) mFree(handle->update);
        handle->update =mMalloc(data_size*sizeof(float));
        memset(handle->update,0,data_size*sizeof(float));
    }
    
    // printf("mheight is %d,mwidth is %d\n",mheight,mwidth);
    // printf("data_size is %d\n",data_size);

    if(handle->kernel !=NULL) mFree(handle->kernel);
    handle->kernel = mMalloc(data_size*sizeof(float));
    
    char name[32]; void *p_data;
    sprintf(name,"%s_kernel",layer->name);
    if(morn_network_parafile==NULL)
    {
        float scale = sqrt(2.0f/mwidth);
        for(int i=0;i<data_size;i++)
            handle->kernel[i] = scale*mNormalRand(0.0f,1.0f);
            // handle->kernel[i] = (float)mRand(-16384,16383)/16384.0f;
        printf("handle->kernel[0] is %f\n",handle->kernel[0]);
        printf("handle->kernel[5] is %f\n",handle->kernel[5]);
    }
    else
    {
        p_data = handle->kernel;
        mMORNRead(morn_network_parafile,name,&p_data,1,data_size*sizeof(float));
        printf("handle->kernel[0] is %f\n",handle->kernel[0]);
        printf("handle->kernel[5] is %f\n",handle->kernel[5]);
    }
    
    if(handle->mat!=NULL) mFree(handle->mat);
    handle->mat = mMalloc(mheight*mwidth*sizeof(float));
    
    hdl->valid = 1;
}

void mTensorDeconvForward(MLayer *layer)
{
    // printf("deconvconvconvconvconvconvconvconvconvconvdeconv\n");
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Deconv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorDeconvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorDeconvSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorDeconv,hdl);
    struct HandleTensorDeconv *handle = hdl->handle;
   
    int mheight = (out->height*out->width);
    int mwidth = para->knl_height*para->knl_width*in->channel+1;
    
    float *kernel_data= handle->kernel;
    float *in_data = handle->mat;
    
    // printf("aaaaaaaaaaaaaaaaaaaaamheight is %d,mwidth is %d\n",mheight,mwidth);
    
    for(int b=0;b<in->batch;b++)
    {
        DeconvTensorToMatData(in,b,in_data,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        in_data[mwidth-1]=1.0f;
    
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                    para->knl_num,mheight,mwidth,
                    1.0f,
                    kernel_data,mwidth,
                        in_data,mwidth,
               0.0f,   out_data,mheight);
        // TensorDeconvForward(in_data,out_data,kernel_data,para);
    }
    
    layer->state = MORN_FORWARD;
    
    // if(morn_network_time==0)
    // {
        
        // TensorImage(layer->tns,0,0,"test44_conv0.bmp");
        // TensorImage(layer->tns,0,1,"test44_conv1.bmp");
        // TensorImage(layer->tns,0,2,"test44_conv2.bmp");
    // }
    
    // printf("convconvconvconvconvconvconvconvconvconvconv\n");
}

void mTensorDeconvBackward(MLayer *layer)
{
    // printf("convconvconvconvconvconvconvconvconvconvconv\n");
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Deconv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorDeconvPara *para = layer->para;
    
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorDeconv,hdl);
    struct HandleTensorDeconv *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    int mheight = (out->height*out->width);
    int mwidth = para->knl_height*para->knl_width*in->channel+1;
    
    float *kernel_data= handle->kernel;
    float *update_data= handle->update;
    float *    in_data= handle->mat;
    float *   res_data= handle->mat;
    
    mNetworkParaWrite(layer,"kernel",kernel_data,para->knl_num*mwidth*sizeof(float));

    for(int b=0;b<in->batch;b++)
    {
        DeconvTensorToMatData(in,b,in_data,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        in_data[mwidth-1]=1.0f;
    
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                    para->knl_num,mwidth,mheight,
                    1.0f,
                       out_data,mheight,
                        in_data,mwidth,
                    (b==0)?para->momentum:1.0f,
                    update_data,mwidth);
    }
    cblas_saxpby(para->knl_num*mwidth,
                 (0.0f-(para->rate/(float)(in->batch))),update_data,1, 
                 (1.0f-(para->decay*para->rate))       ,kernel_data,1);
    
    if(para->res_valid==0) return;
    
    if(para->prev->state == MORN_FORWARD)
    {
        for(int b=0;b<res->batch;b++) 
            memset(res->data[b],0,in->height*in->width*in->channel*sizeof(float));
        para->prev->state = MORN_BACKWARD;
    }
    
    for(int b=0;b<in->batch;b++)
    {
        float *out_data = out->data[b];
        
        cblas_sgemm(CblasRowMajor,CblasTrans,CblasNoTrans,
                    mheight,mwidth,para->knl_num,
                    1.0f,
                       out_data,mheight,
                    kernel_data,mwidth,
                0.0,   res_data,mwidth);
        
        DeconvMatDataToTensor(res_data,res,b,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
    
        // for(int i=0;i<10;i++)printf("out_res is %8.3f,in_res is %8.3f\n",out_res[i],para->res->data[b][i]);
    }
    
    // printf("convconvconvconvconvconvconvconvconvconvconv\n");
}





