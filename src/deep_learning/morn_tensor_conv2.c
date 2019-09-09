#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cblas.h>
#include "morn_Tensor.h"

void ConvTensorToMatData(MTensor *tns,int bc,float *mdata,int knl_height,int knl_width,int y_stride,int x_stride);
void ConvMatDataToTensor(float *mdata,MTensor *tns,int bc,int knl_height,int knl_width,int y_stride,int x_stride);

struct TensorSampleConvPara
{
    MLayer *prev;
    
    int knl_num;
    int knl_height;
    int knl_width;
    
    int x_stride;
    int y_stride;
    
    float sample_ratio;
    
    int res_valid;
    
    float rate;
    float decay;
    float momentum;
};
void *mTensorSampleConvPara(MFile *ini,char *name)
{
    struct TensorSampleConvPara *para = mMalloc(sizeof(struct TensorSampleConvPara));
   
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
    
    value = mINIRead(ini,name,"sample_ratio");
    if(value != NULL) para->sample_ratio = atof(value);else para->sample_ratio = 0.5;
    
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

struct HandleTensorSampleConv
{
    float *mat;
    float *data;
    float *kernel;
    float *update;
    int   *locate;
};
void endTensorSampleConv(void *info)
{
    struct HandleTensorSampleConv *handle = info;
    if(handle->mat   != NULL) mFree(handle->mat   );
    if(handle->data  != NULL) mFree(handle->data  );
    if(handle->kernel!= NULL) mFree(handle->kernel);
    if(handle->update!= NULL) mFree(handle->update);
    if(handle->locate!= NULL) mFree(handle->locate);
}
#define HASH_TensorSampleConv 0x8a31af2
void TensorSampleConvSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    
    struct TensorSampleConvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorSampleConv,hdl);
    struct HandleTensorSampleConv *handle = hdl->handle;
    
    int out_height= in->height/para->y_stride;
    int out_width = in->width /para->x_stride;
    int mheight = (out_height*out_width);
    int mwidth = (para->knl_height*para->knl_width*in->channel)*para->sample_ratio;
    
    int data_size = para->knl_num*(mwidth+1);
    
    mTensorRedefine(out,in->batch,para->knl_num,out_height,out_width,NULL);
    if(morn_network_flag == MORN_TRAIN)
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
   
        if(handle->update != NULL) mFree(handle->update);
        handle->update =mMalloc(data_size*sizeof(float));
        memset(handle->update,0,data_size*sizeof(float));
    }
    
    if(handle->kernel !=NULL) mFree(handle->kernel);
    handle->kernel = mMalloc(data_size*sizeof(float));
    
    if(morn_network_parafile==NULL)
    {
        float scale = sqrt(2.0f/mwidth);
        for(int i=0;i<data_size;i++)
            handle->kernel[i] = mNormalRand(0.0f,1.0f)*scale;
    }
    else
    {
        mNetworkParaRead(layer,"kernel",handle->kernel,data_size*sizeof(float));
    }
    
    int matwidth = para->knl_height*para->knl_width*in->channel+1;
    if(handle->mat!=NULL) mFree(handle->mat);
    handle->mat = mMalloc(mheight*matwidth*sizeof(float));
    
    if(handle->data!=NULL) mFree(handle->data);
    handle->data= mMalloc(mheight*(mwidth+1)*sizeof(float));
    
    if(handle->locate!=NULL) mFree(handle->locate);
    handle->locate = mMalloc(mwidth*para->knl_num*sizeof(int));
    if(morn_network_parafile==NULL)
    {
        char *flag = mMalloc(para->knl_height*para->knl_width*in->channel*sizeof(char));
        for(int j=0;j<para->knl_num;j++)
        {
            memset(flag,1,para->knl_height*para->knl_width*in->channel*sizeof(char));
            int *locate = handle->locate+j*mwidth;
            
            for(int i=0;i<mwidth;)
            {
                locate[i]=mRand(0,para->knl_height*para->knl_width*in->channel);
                if(flag[i]==0) continue;
                flag[i]=0; i++;
            }
            
            mAscSortS32(locate,NULL,locate,NULL,mwidth);
        }
        mFree(flag);
    }
    else
    {
        mNetworkParaRead(layer,"locate",handle->locate,mwidth*para->knl_num*sizeof(int));
    }
    
    hdl->valid = 1;
}

void mTensorSampleConvForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("SampleConv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorSampleConvPara *para = layer->para;
    
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorSampleConvSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorSampleConv,hdl);
    struct HandleTensorSampleConv *handle = hdl->handle;
    
    int mheight = (out->height*out->width);
    int mwidth = (para->knl_height*para->knl_width*in->channel)*para->sample_ratio;
    int matwidth = para->knl_height*para->knl_width*in->channel+1;
    
    float *mat_data = handle->mat;
    float *in_data = handle->data;
    
    for(int b=0;b<in->batch;b++)
    {
        ConvTensorToMatData(in,b,mat_data,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
        
        for(int k=0;k<para->knl_num;k++)
        {
            int   *locate = handle->locate+k* mwidth;
            float *kernel = handle->kernel+k*(mwidth+1);
            float *out_data = out->data[b]+k* mheight;
            for(int j=0;j<mheight;j++)
            {
                for(int i=0;i<mwidth;i++) in_data[j*mwidth+i]=mat_data[j*matwidth+locate[i]];
                in_data[j*mwidth+mwidth]=1.0f;
            }
        
            cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                        1,mheight,mwidth+1,
                        1.0f,
                          kernel,mwidth+1,
                         in_data,mwidth+1,
                   0.0f,out_data,mheight);
        }
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorSampleConvBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("SampleConv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorSampleConvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorSampleConv,hdl);
    struct HandleTensorSampleConv *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    int mheight = (out->height*out->width);
    int mwidth = (para->knl_height*para->knl_width*in->channel)*para->sample_ratio;
    int matwidth = para->knl_height*para->knl_width*in->channel+1;
    
    float *mat_data = handle->mat;
    float *in_data  = handle->data;
    float *res_data = handle->data;
    
    mNetworkParaWrite(layer,"locate",handle->locate,mwidth*para->knl_num*sizeof(int));
    mNetworkParaWrite(layer,"kernel",handle->kernel,para->knl_num*mwidth*sizeof(float));
    
    for(int b=0;b<out->batch;b++)
    {
        ConvTensorToMatData(in,b,mat_data,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
        
        for(int k=0;k<para->knl_num;k++)
        {
            int   *locate = handle->locate+k* mwidth;
            float *update = handle->update+k*(mwidth+1);
            float *out_data = out->data[b]+k* mheight;
            for(int j=0;j<mheight;j++)
            {
                for(int i=0;i<mwidth;i++) in_data[j*mwidth+i]=mat_data[j*matwidth+locate[i]];
                in_data[j*mwidth+mwidth]=1.0f;
            }
        
            cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                        1,mwidth,mheight,
                        1.0f/mheight,
                        out_data,mheight,
                        in_data,mwidth,
                        (b==0)?para->momentum:1.0f,
                        update,mwidth);
                    
        }
    }
    
    cblas_saxpby(para->knl_num*mwidth,
                 (0.0f-(para->rate/(float)(in->batch))),handle->update,1, 
                 (1.0f-(para->decay*para->rate))       ,handle->kernel,1);
    
    if(para->res_valid==0) return;
    
    if(para->prev->state == MORN_FORWARD)
    {
        for(int b=0;b<res->batch;b++) 
            memset(res->data[b],0,in->height*in->width*in->channel*sizeof(float));
        para->prev->state = MORN_BACKWARD;
    }
    
    for(int b=0;b<in->batch;b++)
    {
        memset(mat_data,0,matwidth*mheight*sizeof(float));
        
        for(int k=0;k<para->knl_num;k++)
        {
            int   *locate = handle->locate+k* mwidth;
            float *kernel = handle->kernel+k*(mwidth+1);
            float *out_data = out->data[b]+k* mheight;
            
            cblas_sgemm(CblasRowMajor,CblasTrans,CblasNoTrans,
                        mheight,mwidth,1,
                        1.0f,
                        out_data,mheight,
                          kernel,mwidth,
                    0.0,res_data,mwidth);
                    
            for(int j=0;j<mheight;j++)
            {
                for(int i=0;i<mwidth;i++) mat_data[j*matwidth+locate[i]]+=res_data[j*mwidth+i];
            }        
                    
        }
        
        ConvMatDataToTensor(mat_data,res,b,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
    }
}

void DirConvTensorToMatData(MTensor *tns,int bc,float *mdata,int knl_r,int knl_dir,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width/x_stride;
    int out_height=height/y_stride;
    int mwidth = (knl_r+knl_r+1)*channel+1;
    int mheight= out_height*out_width;
    
    int sx=0,sy=0;
         if(knl_dir==0) {sx=0;sy= 1;}
    else if(knl_dir==1) {sx=1;sy= 1;}
    else if(knl_dir==2) {sx=1;sy= 0;}
    else if(knl_dir==3) {sx=1;sy=-1;}
    
    int x0=(width -(out_width -1)*x_stride)/2;
    int y0=(height-(out_height-1)*y_stride)/2;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int i,j,c;
    for(j=out_width;j<mheight;j++)
    {
        int n=y0+j/out_width*y_stride-knl_r*sy;
        int m=x0+j%out_width*x_stride-knl_r*sx;
        for(i=0;i<mwidth-1;i+=channel)
        {
            int h=n;if(h<0)h=0;else if(h>=height)h=height-1;
            int w=m;if(w<0)w=0;else if(w>= width)w= width-1;
            for(c=0;c<channel;c++) mdata[(j*mwidth)+i+c]=tdata[c*tsize+h*width+w];
            n=n+sy;m=m+sx;
        }
        mdata[(j*mwidth)+mwidth-1]=1.0f;
    }
}
                
void DirConvMatDataToTensor(float *mdata,MTensor *tns,int bc,int knl_r,int knl_dir,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width/x_stride;
    int out_height=height/y_stride;
    int mwidth = (knl_r+knl_r+1)*channel+1;
    int mheight= out_height*out_width;
    
    int sx=0,sy=0;
         if(knl_dir==0) {sx=0;sy= 1;}
    else if(knl_dir==1) {sx=1;sy= 1;}
    else if(knl_dir==2) {sx=1;sy= 0;}
    else if(knl_dir==3) {sx=1;sy=-1;}
    
    int x0=(width -(out_width -1)*x_stride)/2;
    int y0=(height-(out_height-1)*y_stride)/2;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int i,j,c;
    for(j=0;j<mheight;j++)
    {
        int n=y0+j/out_width*y_stride-knl_r*sy;
        int m=x0+j%out_width*x_stride-knl_r*sx;
        for(i=0;i<mwidth-1;i+=channel)
        {
            int h=n;if(h<0)h=0;else if(h>=height)h=height-1;
            int w=m;if(w<0)w=0;else if(w>= width)w= width-1;
            for(c=0;c<channel;c++) tdata[c*tsize+h*width+w]+=mdata[(j*mwidth)+i+c];
            n=n+sy;m=m+sx;
        }
    }
}

struct TensorDirConvPara
{
    MLayer *prev;
    
    int knl_num;
    int knl_r;
    
    int dir;
    
    int x_stride;
    int y_stride;
    
    int res_valid;
    
    float rate;
    float decay;
    float momentum;
};
void *mTensorDirConvPara(MFile *ini,char *name)
{
    struct TensorDirConvPara *para = mMalloc(sizeof(struct TensorDirConvPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"knl_num");
    if(value != NULL) para->knl_num= atoi(value);else para->knl_num= 1; 
    
    value = mINIRead(ini,name,"knl_r");
    if(value != NULL) para->knl_r= atoi(value);else para->knl_r= 1; 
    
    value = mINIRead(ini,name,"dir");
    mException((value==NULL),EXIT,"invalid convolution direction");
         if((strcmp(value, "u-d" )==0)||(strcmp(value, "d-u" )==0)) para->dir=0;     
    else if((strcmp(value,"lu-rd")==0)||(strcmp(value,"rd-lu")==0)) para->dir=1;
    else if((strcmp(value, "l-r" )==0)||(strcmp(value, "r-l" )==0)) para->dir=2;
    else if((strcmp(value,"ru-ld")==0)||(strcmp(value,"ld-ru")==0)) para->dir=3;
    else mException(1,EXIT,"invalid convolution direction");
    
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

struct HandleTensorDirConv
{
    float *mat;
    float *kernel;
    float *update;
};
void endTensorDirConv(void *info)
{
    struct HandleTensorDirConv *handle = info;
    if(handle->mat   != NULL) mFree(handle->mat);
    if(handle->kernel!= NULL) mFree(handle->kernel);
    if(handle->update!= NULL) mFree(handle->update);
}
#define HASH_TensorDirConv 0x9e2674b3
void TensorDirConvSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    
    struct TensorDirConvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorDirConv,hdl);
    struct HandleTensorDirConv *handle = hdl->handle;
    
    int out_height= in->height/para->y_stride;
    int out_width = in->width /para->x_stride;
    int mheight = (out_height*out_width);
    int mwidth = (para->knl_r+para->knl_r+1)*in->channel+1;
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
    
    if(handle->kernel !=NULL) mFree(handle->kernel);
    handle->kernel = mMalloc(data_size*sizeof(float));
    
    if(morn_network_parafile==NULL)
    {
        float scale = sqrt(2.0f/mwidth);
        for(int i=0;i<data_size;i++)
            handle->kernel[i] = mNormalRand(0.0f,1.0f)*scale;
    }
    else
    {
        mNetworkParaRead(layer,"kernel",handle->kernel,data_size*sizeof(float));
    }
    
    if(handle->mat!=NULL) mFree(handle->mat);
    handle->mat = mMalloc(mheight*mwidth*sizeof(float));
    
    hdl->valid = 1;
}

void mTensorDirConvForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("DirConv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorDirConvPara *para = layer->para;
    
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorDirConvSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorDirConv,hdl);
    struct HandleTensorDirConv *handle = hdl->handle;
    
    int mheight = (out->height*out->width);
    int mwidth = (para->knl_r+para->knl_r+1)*in->channel+1;
    
    float *kernel_data= handle->kernel;
    float *in_data = handle->mat;
    
    for(int b=0;b<in->batch;b++)
    {
        DirConvTensorToMatData(in,b,in_data,para->knl_r,para->dir,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                    para->knl_num,mheight,mwidth,
                    1.0f,
                    kernel_data,mwidth,
                        in_data,mwidth,
               0.0f,   out_data,mheight);
    }
    
    layer->state = MORN_FORWARD;
}

void mTensorDirConvBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("DirConv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorDirConvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorDirConv,hdl);
    struct HandleTensorDirConv *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    int mheight = (out->height*out->width);
    int mwidth = (para->knl_r+para->knl_r+1)*in->channel+1;
    
    float *kernel_data= handle->kernel;
    float *update_data= handle->update;
    float *    in_data= handle->mat;
    float *   res_data= handle->mat;
    
    mNetworkParaWrite(layer,"kernel",kernel_data,para->knl_num*mwidth*sizeof(float));
    
    for(int b=0;b<out->batch;b++)
    {
        DirConvTensorToMatData(in,b,in_data,para->knl_r,para->dir,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                    para->knl_num,mwidth,mheight,
                    1.0f/mheight,
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
        
        DirConvMatDataToTensor(res_data,res,b,para->knl_r,para->dir,para->y_stride,para->x_stride);
    }
}







