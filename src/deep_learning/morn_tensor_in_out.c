#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_Tensor.h"
#include "morn_tensor_caculate.h"

struct TensorInputPara
{
    int height;
    int width;
    int channel;
};
void *mTensorInputPara(MFile *ini,char *name)
{
    struct TensorInputPara *para = mMalloc(sizeof(struct TensorInputPara));
    
    char *value;
    value=mINIRead(ini,name,"height" );if(value!=NULL) para->height =atoi(value);else para->height = DFLT;
    value=mINIRead(ini,name,"width"  );if(value!=NULL) para->width  =atoi(value);else para->width  = DFLT;
    value=mINIRead(ini,name,"channel");if(value!=NULL) para->channel=atoi(value);else para->channel= DFLT;
    
    return para;
}

void mTensorInputForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Input",mLayerType(layer)),EXIT,"invalid layer type");
    
    struct TensorInputPara *para = layer->para;
    MTensor *out=layer->tns;

    if(para->channel!= DFLT) mException((out->channel!=para->channel),EXIT,"invalid para channel");
    if(para->height != DFLT) mException((out->height !=para->height ),EXIT,"invalid para height");
    if(para->width  != DFLT) mException((out->width  !=para->width  ),EXIT,"invalid para width");
       
    layer->state = MORN_FORWARD;
    
    // if(morn_network_time==1)TensorImage(out,0,0,"test44_input.bmp");
}

void mTensorInputBackward(MLayer *layer)
{
    if(PARA_SAVE)
    {
        mFileRelease(morn_network_parafile);
        morn_network_parafile = NULL;
    }
}

// struct TensorOutputPara
// {
    // MLayer *prev;
   
    // int height;
    // int width;
    // int channel;
    
    // float (*loss)(MLayer *,MLayer *,float *);
    // void (*dloss)(MLayer *,MLayer *);
    
    // char argv[4][128];
// };
void *mTensorOutputPara(MFile *ini,char *name)
{
    struct TensorOutputPara *para = mMalloc(sizeof(struct TensorOutputPara));
    
    char *value = mINIRead(ini,name,"prev");
    para->prev = mNetworkLayer(ini,value);
    mException((para->prev == NULL),EXIT,"invalid prev");
    
    value=mINIRead(ini,name,"height" );if(value!=NULL) para->height =atoi(value);else para->height =DFLT; 
    value=mINIRead(ini,name,"width"  );if(value!=NULL) para->width  =atoi(value);else para->width  =DFLT;
    value=mINIRead(ini,name,"channel");if(value!=NULL) para->channel=atoi(value);else para->channel=DFLT;
    
    if(morn_network_flag == MORN_PREDICT) return para;
    
    value = mINIRead(ini,name,"loss");
    if(value == NULL) value = mINIRead(ini,"para","loss");
    mException((value == NULL),EXIT,"layer %s no loss function defined",name);
    printf("loss is %s\n",value);
    printf("morn_loss_register_num is %d\n",morn_loss_register_num);
    int i;for(i=0;i<morn_loss_register_num;i++)
    {
        if(strcmp(value,morn_loss_register[i].name)==0)
        {
            para->loss = morn_loss_register[i].loss;
            para->dloss= morn_loss_register[i].dloss;
            break;
        }
    }
    mException((i==morn_loss_register_num),EXIT,"invalid loss function");
    
    value = mINIRead(ini,name,"argv0");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[0][0]),value);}
    value = mINIRead(ini,name,"argv1");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[1][0]),value);}
    value = mINIRead(ini,name,"argv2");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[2][0]),value);}
    value = mINIRead(ini,name,"argv3");
    if(value!=NULL) {mException((strlen(value)>=128),EXIT,"invalid error");strcpy(&(para->argv[3][0]),value);}

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

struct HandleTensorOutput
{
    float k;
    float b;
    
    float error[32];
    int idx;
    float sum;
};
void endTensorOutput(void *info) {NULL;}
#define HASH_TensorOutput 0xa9f05c6f

void TensorOutputSet(MLayer *layer)
{
    if(layer->state != DFLT) return;
    struct TensorOutputPara *para = layer->para;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    if(para->channel== DFLT) para->channel= in->channel;
    if(para->height == DFLT) para->height = in->height;
    if(para->width  == DFLT) para->width  = in->width;
    
    if(morn_network_flag == MORN_PREDICT)
    {
        mTensorCopy(in,out);
        mTensorRedefine(in,in->batch,in->channel,in->height,in->width,out->data);
    }
    else
    {
        if(INVALID_TENSOR(res)) mTensorRedefine(res,in->batch,in->channel,in->height,in->width,in->data);
        else                    mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
    }
}

void mTensorOutputForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Output",mLayerType(layer)),EXIT,"invalid layer type");
    
    TensorOutputSet(layer);
    
    layer->state = MORN_FORWARD;
}


void mTensorOutputBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Output",mLayerType(layer)),EXIT,"invalid layer type");
    
    struct TensorOutputPara *para = layer->para;
    MTensor *in = para->prev->tns;
    // MTensor *res= para->prev->res;
    MTensor *out=layer->tns;
    
    morn_network_error = 0.0f;
    
    // printf("para->channel is %d,in->channel is %d\n",para->channel,in->channel);
    if(para->channel!= DFLT) mException((para->channel!=in->channel),EXIT,"invalid output");
    if(para->height != DFLT) mException((para->height !=in->height ),EXIT,"invalid output");
    if(para->width  != DFLT) mException((para->width  !=in->width  ),EXIT,"invalid output");
    
    /*
    int sssum=0;
    for(int i=0;i<in->batch;i++)
    {
        int max_in=0;
        for(int j=1;j<10;j++)
            if(in->data[i][j]>in->data[i][max_in]) max_in=j;
        if(out->data[i][max_in]==1.0f) sssum+=1;
        // printf("idx is %d,out->data[i] is %p\n",idx,out->data[i]);
    
        // printf("in is %f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"
        // ,in->data[i][0],in->data[i][1],in->data[i][2],in->data[i][3],in->data[i][4]
        // ,in->data[i][5],in->data[i][6],in->data[i][7],in->data[i][8],in->data[i][9]);
        // printf("out is %f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"
        // ,out->data[i][0],out->data[i][1],out->data[i][2],out->data[i][3],out->data[i][4]
        // ,out->data[i][5],out->data[i][6],out->data[i][7],out->data[i][8],out->data[i][9]);
    }
    float ssscore = (float)sssum/(float)(in->batch);
    printf("ssscore is %f\t",ssscore);
    */
        
    // printf("in size is %d,%d,%d,%d\n",in->batch,in->height,in->width,in->channel);
    // printf("out size is %d,%d,%d,%d\n",out->batch,out->height,out->width,out->channel);
    
    float network_error = para->loss(layer,para->prev,NULL);
    
    MHandle *hdl; ObjectHandle(out,TensorOutput,hdl);
    struct HandleTensorOutput *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        for(int i=0;i<32;i++) handle->error[i] = network_error;
        handle->idx = 0;
        handle->sum = network_error*32;
        hdl->valid = 1;
    }
    handle->sum = handle->sum - handle->error[handle->idx] + network_error;
    handle->error[handle->idx] = network_error;
    handle->idx += 1; 
    if(handle->idx == 32) 
    {
        handle->sum = 0.0f;for(int i=0;i<32;i++) handle->sum+=handle->error[i];
        handle->idx = 0;
    }
    morn_network_error = handle->sum/32.0f;
    printf("network_error is %f\t",network_error);

    if(para->dloss != NULL)
    {
        para->dloss(layer,para->prev);
    }
    
    // for(int i=0;i<10;i++)
        // printf("in is %f,out is %f,res is %f\n",in->data[0][i],out->data[0][i],res->data[0][i]);
    
    if(PARA_SAVE)
    {
        sprintf(morn_network_para_filename,"%s/network_para_%d.morn",morn_network_para_dir,morn_network_time);
        printf("filename is %s\n",morn_network_para_filename);
        morn_network_parafile = mFileCreate(morn_network_para_filename);
    }
    
    para->prev->state = MORN_BACKWARD;
}
