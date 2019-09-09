#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cblas.h>
#include "morn_Tensor.h"

void ConvTensorToMatData(MTensor *tns,int bc,float *mdata,int knl_height,int knl_width,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width/x_stride;
    int out_height=height/y_stride;
    int mwidth = knl_height*knl_width*channel+1;
    int mheight= out_height*out_width;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int x0=(width -(out_width -1)*x_stride)/2;
    int y0=(height-(out_height-1)*y_stride)/2;
    
    int i,j,c;
    for(j=0;j<out_width;j++)
    {
        int n=y0           -knl_height/2;
        int m=x0+j*x_stride-knl_width /2;
        for(i=0;i<mwidth-1;i+=channel)
        {
            int h= i/(knl_width*channel)         +n;if(h<0)h=0;else if(h>=height)h=height-1;
            int w=(i%(knl_width*channel))/channel+m;if(w<0)w=0;else if(w>= width)w= width-1;
            for(c=0;c<channel;c++)
            {
                mdata[(j*mwidth)+i+c]=tdata[c*tsize+h*width+w];
                // if((i==0)&&(j==0)&&(c==0))
                    // printf("wwwwwwwwwwwww h is %d,w is %d,idx is %d,tns is %f\n"
                           // ,h,w,c*tsize+h*width+w,tdata[c*tsize+h*width+w]);
            }
        }
        mdata[(j*mwidth)+mwidth-1]=1.0f;
    }
    // printf("wwwwwwwwwwwwwww mwidth is %d,mdata[0] is %f\n",mwidth,mdata[0]);
    for(j=out_width;j<mheight;j++)
    {
        int num=MAX(0,(knl_height-y_stride))*knl_width*channel;
        if(num>0) memcpy(mdata+j*mwidth,mdata+(j-out_width)*mwidth+y_stride*knl_width*channel,num*sizeof(float));
       
        int n=y0+j/out_width*y_stride-knl_height/2;
        int m=x0+j%out_width*x_stride-knl_width /2;
        for(i=num;i<mwidth-1;i+=channel)
        {
            int h= i/(knl_width*channel)         +n;if(h<0)h=0;else if(h>=height)h=height-1;
            int w=(i%(knl_width*channel))/channel+m;if(w<0)w=0;else if(w>= width)w= width-1;
            for(c=0;c<channel;c++)
                mdata[(j*mwidth)+i+c]=tdata[c*tsize+h*width+w];
        }
        mdata[(j*mwidth)+mwidth-1]=1.0f;
    }
}

void ConvMatDataToTensor(float *mdata,MTensor *tns,int bc,int knl_height,int knl_width,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width/x_stride;
    int out_height=height/y_stride;
    int mwidth = knl_height*knl_width*channel+1;
    int mheight= out_height*out_width;
    
    // printf("wwwwwwwwwwwwwwwwww mwidth is %d\n",mwidth);
    
    int x0=(width -(out_width -1)*x_stride)/2;
    int y0=(height-(out_height-1)*y_stride)/2;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int i,j,c;
    for(j=0;j<mheight;j++)
    {
        int n=y0+j/out_width*y_stride-knl_height/2;
        int m=x0+j%out_width*x_stride-knl_width /2;
        for(i=0;i<mwidth-1;i+=channel)
        {
            int h= i/(knl_width*channel)         +n;if(h<0)h=0;else if(h>=height)h=height-1;
            int w=(i%(knl_width*channel))/channel+m;if(w<0)w=0;else if(w>= width)w= width-1;
            for(c=0;c<channel;c++)
                tdata[c*tsize+h*width+w]+=mdata[(j*mwidth)+i+c];
        }
    }
}

struct TensorConvPara
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
void *mTensorConvPara(MFile *ini,char *name)
{
    struct TensorConvPara *para = mMalloc(sizeof(struct TensorConvPara));
   
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

struct HandleTensorConv
{
    float *mat;
    float *kernel;
    float *update;
};
void endTensorConv(void *info)
{
    struct HandleTensorConv *handle = info;
    if(handle->mat   != NULL) mFree(handle->mat);
    if(handle->kernel!= NULL) mFree(handle->kernel);
    if(handle->update!= NULL) mFree(handle->update);
}
#define HASH_TensorConv 0x9087d39c
void TensorConvSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    
    struct TensorConvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorConv,hdl);
    struct HandleTensorConv *handle = hdl->handle;
    
    int out_height= in->height/para->y_stride;
    int out_width = in->width /para->x_stride;
    // printf("out_height is %d,out_width is %d\n",out_height,out_width);
    int mheight = (out_height*out_width);
    int mwidth = para->knl_height*para->knl_width*in->channel+1;
    int data_size = para->knl_num*mwidth;
    printf("%s:in->height is %d,in->width is %d\n",layer->name,in->height,in->width);
    printf("%s:out_height is %d,out_width is %d\n",layer->name,out_height,out_width);
    
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
        printf("data_size is %d\n",data_size);
        printf("a handle->kernel[0] is %f\n",handle->kernel[0]);
        printf("a handle->kernel[5] is %f\n",handle->kernel[5]);
    }
    else
    {
        mNetworkParaRead(layer,"kernel",handle->kernel,data_size*sizeof(float));
        printf("handle->kernel[0] is %f\n",handle->kernel[0]);
        printf("handle->kernel[5] is %f\n",handle->kernel[5]);
    }
    
    if(handle->mat!=NULL) mFree(handle->mat);
    handle->mat = mMalloc(mheight*mwidth*sizeof(float));
    
    hdl->valid = 1;
}

void mTensorConvForward(MLayer *layer)
{
    // printf("%s\n",layer->name);
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Conv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorConvPara *para = layer->para;
    
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorConvSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorConv,hdl);
    struct HandleTensorConv *handle = hdl->handle;
    
    int mheight = (out->height*out->width);
    int mwidth = para->knl_height*para->knl_width*in->channel+1;
    
    float *kernel_data= handle->kernel;
    float *in_data = handle->mat;
    
    // printf("aaaaaaaaaaaaaaaaaaaaamheight is %d,mwidth is %d\n",mheight,mwidth);
    
    for(int b=0;b<in->batch;b++)
    {
        ConvTensorToMatData(in,b,in_data,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                    para->knl_num,mheight,mwidth,
                    1.0f,
                    kernel_data,mwidth,
                        in_data,mwidth,
               0.0f,   out_data,mheight);
    }
    
    layer->state = MORN_FORWARD;
    return;
    
    // printf("convconvconvconvconvconvconvconvconvconvconv\n");
}

void mTensorConvBackward(MLayer *layer)
{
    // printf("%s\n",layer->name);
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Conv",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorConvPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorConv,hdl);
    struct HandleTensorConv *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    int mheight = (out->height*out->width);
    int mwidth = para->knl_height*para->knl_width*in->channel+1;
    
    float *kernel_data= handle->kernel;
    float *update_data= handle->update;
    float *    in_data= handle->mat;
    float *   res_data= handle->mat;
    
    // if(strcmp(layer->name,"conv1")==0)
    // {
        // printf("\nb1 kernel is ");for(int i=0;i<10;i++) printf("%f,",kernel_data[i]);printf("\n");
    // }
    
    mNetworkParaWrite(layer,"kernel",kernel_data,para->knl_num*mwidth*sizeof(float));
    
    for(int b=0;b<out->batch;b++)
    {
        ConvTensorToMatData(in,b,in_data,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        // for(int i=0;i<para->knl_num*mheight;i++)
        // {
            // if(mIsInf(out_data[i])) {printf("%d,%d,eeeeeeeeee\n",b,i);exit(0);}
        // }
        
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                    para->knl_num,mwidth,mheight,
                    1.0f/mheight,
                       out_data,mheight,
                        in_data,mwidth,
                    (b==0)?para->momentum:1.0f,
                    update_data,mwidth);
                    
        // if((strcmp(layer->name,"conv100")==0))
        // {
            // printf("b update_data is:");for(int i=0;i<10;i++)printf("%f,",update_data[i]);printf("\n");
            // printf("b     in_data is:");for(int i=0;i<10;i++)printf("%f,",in_data[i]);printf("\n");
            // printf("b    out_data is:");for(int i=0;i<10;i++)printf("%f,",out_data[i]);printf("\n");
        // }
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
        
        ConvMatDataToTensor(res_data,res,b,para->knl_height,para->knl_width,para->y_stride,para->x_stride);
    }
    
    // printf("convconvconvconvconvconvconvconvconvconvconv\n");
}

void GroupConvTensorToMatData(MTensor *tns,int bc,float *mdata,int knl_channel,int knl_height,int knl_width,int c_stride,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width  =  width/x_stride;
    int out_height = height/y_stride;
    int out_channel=(channel-knl_channel/2+1)/c_stride;
    int mwidth = knl_height*knl_width*knl_channel+1;
    int mheight= out_height*out_width;
    
    int x0=(width -(out_width -1)*x_stride)/2;
    int y0=(height-(out_height-1)*y_stride)/2;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int k,i,j,c;
    for(k=0;k<out_channel;k++)
    {
        for(j=0;j<out_width;j++)
        {
            int n=y0           -knl_height/2;
            int m=x0+j*x_stride-knl_width /2;
            for(i=0;i<mwidth-1;i+=knl_channel)
            {
                int h= i/(knl_width*knl_channel)             +n;if(h<0)h=0;else if(h>=height)h=height-1;
                int w=(i%(knl_width*knl_channel))/knl_channel+m;if(w<0)w=0;else if(w>= width)w= width-1;
                for(c=0;c<knl_channel;c++)
                    mdata[(k*mheight+j)*mwidth+i+c]=tdata[MIN((c+k*c_stride),channel-1)*tsize+h*width+w];
            }
            mdata[(k*mheight+j)*mwidth+mwidth-1]=1.0f;
        }
        for(j=out_width;j<mheight;j++)
        {
            int num=MAX(0,(knl_height-y_stride))*knl_width*knl_channel;
            if(num>0) memcpy(mdata+(k*mheight+j)*mwidth,mdata+((k*mheight+j)-out_width)*mwidth+y_stride*knl_width*knl_channel,num*sizeof(float));
            
            int n=y0+j/out_width*y_stride-knl_height/2;
            int m=x0+j%out_width*x_stride-knl_width /2;
            for(i=num;i<mwidth-1;i+=knl_channel)
            {
                int h= i/(knl_width*knl_channel)             +n;if(h<0)h=0;else if(h>=height)h=height-1;
                int w=(i%(knl_width*knl_channel))/knl_channel+m;if(w<0)w=0;else if(w>= width)w= width-1;
                for(c=0;c<knl_channel;c++)
                    mdata[(k*mheight+j)*mwidth+i+c]=tdata[MIN((c+k*c_stride),channel-1)*tsize+h*width+w];
            }
            mdata[(k*mheight+j)*mwidth+mwidth-1]=1.0f;
        }
    }
}

void GroupConvMatDataToTensor(float *mdata,MTensor *tns,int bc,int knl_channel,int knl_height,int knl_width,int c_stride,int y_stride,int x_stride)
{
    int height = tns->height;
    int width  = tns->width;
    int channel= tns->channel;
    
    int out_width = width/x_stride;
    int out_height=height/y_stride;
    int out_channel=(channel-knl_channel/2+1)/c_stride;
    int mwidth = knl_height*knl_width*knl_channel+1;
    int mheight= out_height*out_width;
    
    int x0=(width -(out_width -1)*x_stride)/2;
    int y0=(height-(out_height-1)*y_stride)/2;
    
    float *tdata = tns->data[bc];
    int tsize = tns->height*tns->width;
    
    int k,i,j,c;
    for(k=0;k<out_channel;k++)for(j=0;j<mheight;j++)
    {
        int n=y0+j/out_width*y_stride+knl_height/2+1-knl_height;
        int m=x0+j%out_width*x_stride+knl_width /2+1-knl_width ;
        for(i=0;i<mwidth-1;i+=knl_channel)
        {
            int h= i/(knl_width*knl_channel)             +n;if(h<0)h=0;else if(h>=height)h=height-1;
            int w=(i%(knl_width*knl_channel))/knl_channel+m;if(w<0)w=0;else if(w>= width)w= width-1;
            for(c=0;c<knl_channel;c++)
                tdata[MIN((c+k*c_stride),channel-1)*tsize+h*width+w]+=mdata[(k*mheight+j)*mwidth+i+c];
        }
    }
}

struct TensorGroupConvPara
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
    
    int knl_channel;
    int c_stride;
};
void *mTensorGroupConvPara(MFile *ini,char *name)
{
    struct TensorGroupConvPara *para = mMalloc(sizeof(struct TensorConvPara));
   
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    value = mINIRead(ini,name,"knl_num");
    if(value != NULL) para->knl_num= atoi(value);else para->knl_num= 1;
    
    value = mINIRead(ini,name,"knl_channel");
    if(value != NULL) para->knl_channel= atoi(value);else para->knl_channel= DFLT;
    
    value = mINIRead(ini,name,"knl_height");
    if(value != NULL) para->knl_height= atoi(value);else para->knl_height= 1; 
    
    value = mINIRead(ini,name,"knl_width");
    if(value != NULL) para->knl_width= atoi(value);else para->knl_width= 1; 
    
    value = mINIRead(ini,name,"c_stride");
    if(value != NULL) para->c_stride= atoi(value);else para->c_stride= para->knl_channel;
    
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

struct HandleTensorGroupConv
{
    float *mat;
    float *kernel;
    float *update;
};
void endTensorGroupConv(void *info)
{
    struct HandleTensorGroupConv *handle = info;
    if(handle->mat   != NULL) mFree(handle->mat);
    if(handle->kernel!= NULL) mFree(handle->kernel);
    if(handle->update!= NULL) mFree(handle->update);
}
#define HASH_TensorGroupConv 0x82866393
void TensorGroupConvSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    
    struct TensorGroupConvPara *para = layer->para;
    if(para->knl_channel<=0) 
    {
        layer->type_index=mTensorRegisterIndex("Conv");
        return;
    }
    
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->tns;
    
    MHandle *hdl; ObjectHandle(out,TensorGroupConv,hdl);
    struct HandleTensorGroupConv *handle = hdl->handle;
    
    int out_height= in->height/para->y_stride;
    int out_width = in->width /para->x_stride;
    int out_channel=(in->channel-para->knl_channel/2+1)/para->c_stride;
    // printf("out_height is %d,out_width is %d\n",out_height,out_width);
    int mheight = (out_height*out_width*out_channel);
    int mwidth = para->knl_height*para->knl_width*para->knl_channel+1;
    int data_size = para->knl_num*mwidth;
    
    mTensorRedefine(out,in->batch,para->knl_num*out_channel,out_height,out_width,NULL);
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
            handle->kernel[i] = scale*mNormalRand(0.0f,1.0f);
        printf("handle->kernel[0] is %f\n",handle->kernel[0]);
        printf("handle->kernel[5] is %f\n",handle->kernel[5]);
    }
    else
    {
        mNetworkParaRead(layer,"kernel",handle->kernel,data_size*sizeof(float));
        printf("handle->kernel[0] is %f\n",handle->kernel[0]);
        printf("handle->kernel[5] is %f\n",handle->kernel[5]);
    }
    
    if(handle->mat!=NULL) mFree(handle->mat);
    handle->mat = mMalloc(mheight*mwidth*sizeof(float));
    
    hdl->valid = 1;
}

void mTensorGroupConvForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorGroupConvPara *para = layer->para;
    if(para->knl_channel<=0) return mTensorConvForward(layer);
    
    mException(strcmp("GroupConv",mLayerType(layer)),EXIT,"invalid layer type");
    
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    
    TensorGroupConvSet(layer);
    
    MHandle *hdl; ObjectHandle(out,TensorGroupConv,hdl);
    struct HandleTensorGroupConv *handle = hdl->handle;
    
    int out_channel=(in->channel-para->knl_channel/2+1)/para->c_stride;
    int mheight = (out->height*out->width*out_channel);
    int mwidth = para->knl_height*para->knl_width*para->knl_channel+1;
    
    float *kernel_data= handle->kernel;
    float *in_data = handle->mat;
    
    // printf("aaaaaaaaaaaaaaaaaaaaamheight is %d,mwidth is %d\n",mheight,mwidth);
    
    for(int b=0;b<in->batch;b++)
    {
        GroupConvTensorToMatData(in,b,in_data,para->knl_channel,para->knl_height,para->knl_width,para->c_stride,para->y_stride,para->x_stride);
        float *out_data = out->data[b];
        
        in_data[mwidth-1]=1.0f;
    
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,
                    para->knl_num,mheight,mwidth,
                    1.0f,
                    kernel_data,mwidth,
                        in_data,mwidth,
               0.0f,   out_data,mheight);
        // TensorConvForward(in_data,out_data,kernel_data,para);
    }
    
    layer->state = MORN_FORWARD;
    
    // if(morn_network_time==0)
    // {
        // TensorImage(layer->tns,0,0,"test44_conv0.bmp");
        // TensorImage(layer->tns,0,1,"test44_conv1.bmp");
        // TensorImage(layer->tns,0,2,"test44_conv2.bmp");
    // }
}

void mTensorGroupConvBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    struct TensorGroupConvPara *para = layer->para;
    if(para->knl_channel<=0) return mTensorConvBackward(layer);
    
    mException(strcmp("GroupConv",mLayerType(layer)),EXIT,"invalid layer type");
    
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->res;
    
    MHandle *hdl; ObjectHandle(layer->tns,TensorGroupConv,hdl);
    struct HandleTensorGroupConv *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"no forward operate");
    
    int out_channel=(in->channel-para->knl_channel/2+1)/para->c_stride;
    int mheight = (out->height*out->width*out_channel);
    int mwidth = para->knl_height*para->knl_width*para->knl_channel+1;
    
    float *kernel_data= handle->kernel;
    float *update_data= handle->update;
    float *    in_data= handle->mat;
    float *   res_data= handle->mat;
    
    mNetworkParaWrite(layer,"kernel",kernel_data,para->knl_num*mwidth*sizeof(float));
    
    for(int b=0;b<in->batch;b++)
    {
        GroupConvTensorToMatData(in,b,in_data,para->knl_channel,para->knl_height,para->knl_width,para->c_stride,para->y_stride,para->x_stride);
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
        
        GroupConvMatDataToTensor(res_data,res,b,para->knl_channel,para->knl_height,para->knl_width,para->c_stride,para->y_stride,para->x_stride);
    
        // for(int i=0;i<10;i++)printf("out_res is %8.3f,in_res is %8.3f\n",out_res[i],para->res->data[b][i]);
    }
    
    // printf("convconvconvconvconvconvconvconvconvconvconv\n");
}



                
  








    