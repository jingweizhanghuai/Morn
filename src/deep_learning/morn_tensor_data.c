/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_tensor.h"

/*
void mTrainDataGenerate(void *in ,void  (*in_func)(void *,MTensor **,void *),void *in_para ,int in_num,
                        void *out,void (*out_func)(void *,MTensor **,void *),void *out_para,int out_num,
                        char *filename)
{
    int i;
    MTensor **tns_in = mMalloc(in_num *sizeof(MTensor *));
    MTensor **tns_out= mMalloc(out_num*sizeof(MTensor *));
    
    MFile *file = mFileCreate(filename);
    
    for(i=0;i< in_num;i++) {tns_in[i] = mTensorCreate(1,DFLT,DFLT,DFLT,NULL);} in_func(in ,tns_in ,in_para );
    for(i=0;i<out_num;i++) {tns_out[i]= mTensorCreate(1,DFLT,DFLT,DFLT,NULL);}out_func(out,tns_out,out_para);
    
    for(i=0;i<in_num;i++)
    {
        char name[32]; sprintf(name,"input_%d",i);
        int size = tns_in[i]->channel*tns_in[i]->height*tns_in[i]->width;
        void *p_data = tns_in[i]->data[0];
        mMORNWrite(file,name,&p_data,1,size*sizeof(float));
        mTensorRelease(tns_in[i]);
    }
    
    for(i=0;i<out_num;i++)
    {
        char name[32]; sprintf(name,"output_%d",i);
        int size = tns_out[i]->channel*tns_out[i]->height*tns_out[i]->width;
        void *p_data = tns_out[i]->data[0];
        mMORNWrite(file,name,&p_data,1,size*sizeof(float));
        mTensorRelease(tns_out[i]);
    }
    
    mFree(tns_in);
    mFree(tns_out);
    mFileRelease(file);
}
*/

struct HandleTrainData
{
    MList *net;
    
    char *data_dir;
    MList *filelist;
    
    int batch;
    int train_batch;
    int update;
    
    int index_input;
    int index_output;
};
void endTrainData(void *info)
{
    struct HandleTrainData *handle = info;
    if(handle->filelist != NULL)
        mListRelease(handle->filelist);
}
#define HASH_TrainData 0xeef2f995
void mTrainData(MFile *ini)
{
    MHandle *hdl; ObjectHandle(ini,TrainData,hdl);
    struct HandleTrainData *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->net = mNetworkGenerate(ini);
        
        char *value;
        
        handle->batch = 1;
        value = mINIRead(ini,"para","train_batch"); 
        if(value != NULL) handle->batch = atoi(value);
        
        float coverage_ratio = 0.5f;
        value = mINIRead(ini,"para","coverage_ratio");
        if(value != NULL) coverage_ratio = atof(value);
        if(coverage_ratio < 1.0f) coverage_ratio = 1.0f/coverage_ratio;
        
        float update_ratio = 2.0f/(float)(handle->batch);
        value = mINIRead(ini,"para","update_ratio");
        if(value != NULL) update_ratio = atof(value);
        if(update_ratio >1.0f) update_ratio = 1.0/update_ratio;
        
        if(update_ratio < 1.0f) coverage_ratio = MAX(coverage_ratio,1.0f/(1.0f-update_ratio));
        handle->train_batch = (int)(((float)(handle->batch))*coverage_ratio+0.5);
        handle->update = (int)(handle->train_batch * update_ratio + 0.5);
        if(handle->update < 1) handle->update = 1;
        
        handle->index_input = mTensorRegisterIndex("Input" );
        handle->index_output= mTensorRegisterIndex("Output");
        
        for(int i=0;i<handle->net->num;i++)
        {
            MLayer *layer = handle->net->data[i];
            if((layer->type_index!=handle->index_input)&&(layer->type_index!=handle->index_output)) continue;
            
            value=mINIRead(ini,layer->name,"channel");mException((value==NULL),EXIT,"no input channel");int channel=atoi(value);
            value=mINIRead(ini,layer->name,"height" );mException((value==NULL),EXIT,"no input height" );int height =atoi(value);
            value=mINIRead(ini,layer->name,"width"  );mException((value==NULL),EXIT,"no input width"  );int width  =atoi(value);
            
            mTensorRedefine(layer->tns,handle->train_batch,channel,height,width,NULL);
            layer->tns->batch = handle->batch;
        }
        
        handle->data_dir = mINIRead(ini,"para","data_dir");
        mException((handle->data_dir==NULL),EXIT,"no train data dir");
        
        if(handle->filelist == NULL) handle->filelist = mListCreate(DFLT,NULL);
        mListClear(handle->filelist);
        mFileList(handle->filelist,handle->data_dir,"*.morn");
        
        for(int b=0;b<handle->train_batch;b++)
        {
            char filename[256];
            int n = mRand(0,handle->filelist->num);
            sprintf(filename,"%s/%s",handle->data_dir,(char *)(handle->filelist->data[n]));
            MFile *file = mFileCreate(filename);
            
            for(int i=0;i<handle->net->num;i++)
            {
                MLayer *layer = handle->net->data[i];
                if((layer->type_index!=handle->index_input)&&(layer->type_index!=handle->index_output)) continue;
                
                MTensor *tns = layer->tns;
                void *p_data = tns->data[b];
                int size = tns->channel*tns->height*tns->width;
                mMORNRead(file,layer->name,&p_data,1,size*sizeof(float));
            }
            
            mFileRelease(file);
        }
        
        hdl->valid = 1;
        
        return;
    }
    
    for(int b=0;b<handle->update;b++)
    {
        char filename[256];
        int n = mRand(0,handle->filelist->num);
        sprintf(filename,"%s/%s",handle->data_dir,(char *)(handle->filelist->data[n]));
        MFile *file = mFileCreate(filename);
        
        for(int i=0;i<handle->net->num;i++)
        {
            MLayer *layer = handle->net->data[i];
            if((layer->type_index!=handle->index_input)&&(layer->type_index!=handle->index_output)) continue;
            
            MTensor *tns = layer->tns;
            void *p_data = tns->data[b];
            int size = tns->channel*tns->height*tns->width;
            mMORNRead(file,layer->name,&p_data,1,size*sizeof(float));
        }
        
        mFileRelease(file);
    }
    
    for(int b=0;b<handle->batch;b++)
    {
        int n = mRand(0,handle->train_batch);
        for(int i=0;i<handle->net->num;i++)
        {
            MLayer *layer = handle->net->data[i];
            if((layer->type_index!=handle->index_input)&&(layer->type_index!=handle->index_output)) continue;
            
            MTensor *tns = layer->tns;
            
            float *buff = tns->data[b];
            tns->data[b] = tns->data[n];
            tns->data[n] = buff;
        }
    }
}

/*
void mTrainData(MFile *ini,char *name[],MTensor *tns[])
{
    MHandle *hdl; ObjectHandle(ini,TrainData,hdl);
    struct HandleTrainData *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->net = mNetworkGenerate(ini);
        
        char *value;
        
        handle->batch = 1;
        value = mINIRead(ini,"para","train_batch"); 
        if(value != NULL) handle->batch = atoi(value);
        
        float coverage_ratio = 0.5f;
        value = mINIRead(ini,"para","coverage_ratio");
        if(value != NULL) coverage_ratio = atof(value);
        if(coverage_ratio < 1.0f) coverage_ratio = 1.0f/coverage_ratio;
        
        float update_ratio = 2.0f/(float)(handle->batch);
        value = mINIRead(ini,"para","update_ratio");
        if(value != NULL) update_ratio = atof(value);
        if(update_ratio >1.0f) update_ratio = 1.0/update_ratio;
        
        if(update_ratio < 1.0f) coverage_ratio = MAX(coverage_ratio,1.0f/(1.0f-update_ratio));
        handle->train_batch = (int)(((float)(handle->batch))*coverage_ratio+0.5);
        handle->update = (int)(handle->train_batch * update_ratio + 0.5);
        if(handle->update < 1) handle->update = 1;
        
        handle->index_input = mTensorRegisterIndex("Input" );
        handle->index_output= mTensorRegisterIndex("Output");
        
        int n=0;
        for(int i=0;i<handle->net->num;i++)
        {
            MLayer *layer = handle->net->data[i];
            if((layer->type_index!=handle->index_input)&&(layer->type_index!=handle->index_output)) continue;
            
            strcpy(name[n],layer->name);
            
            value=mINIRead(ini,layer->name,"channel");mException((value==NULL),EXIT,"no input channel");int channel=atoi(value);
            value=mINIRead(ini,layer->name,"height" );mException((value==NULL),EXIT,"no input height" );int height =atoi(value);
            value=mINIRead(ini,layer->name,"width"  );mException((value==NULL),EXIT,"no input width"  );int width  =atoi(value);
            
            mTensorRedefine(layer->tns,handle->train_batch,channel,height,width,NULL);
            layer->tns->batch = handle->batch;
        }
        
        handle->data_dir = mINIRead(ini,"para","data_dir");
        mException((handle->data_dir==NULL),EXIT,"no train data dir");
        
        if(handle->filelist == NULL) handle->filelist = mListCreate(DFLT,NULL);
        mListClear(handle->filelist);
        mFileList(handle->filelist,handle->data_dir,"*.morn");
        
        for(int b=0;b<handle->train_batch;b++)
        {
            char filename[256];
            int n = mRand(0,handle->filelist->num);
            sprintf(filename,"%s/%s",handle->data_dir,(char *)(handle->filelist->data[n]));
            MFile *file = mFileCreate(filename);
            
            for(int i=0;i<handle->net->num;i++)
            {
                MLayer *layer = handle->net->data[i];
                if((layer->type_index!=handle->index_input)&&(layer->type_index!=handle->index_output)) continue;
                
                MTensor *tns = layer->tns;
                void *p_data = tns->data[b];
                int size = tns->channel*tns->height*tns->width;
                mMORNRead(file,layer->name,&p_data,1,size*sizeof(float));
            }
            
            mFileRelease(file);
        }
        
        hdl->valid = 1;
        
        return;
    }
}
*/

struct HandleNetworkTensor
{
    MList *net;
    
    int num;
    int *idx;
 
    int batch;
};
void endNetworkTensor(void *info)
{
    struct HandleNetworkTensor *handle = info;
    if(handle->idx != NULL) mFree(handle->idx);
}
#define HASH_NetworkTensor 0xed589636
void mNetworkTensor(MFile *ini,char *name[],MTensor *tns[])
{
    MHandle *hdl; ObjectHandle(ini,NetworkTensor,hdl);
    struct HandleNetworkTensor *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->net = mNetworkGenerate(ini);
        
        char *value;
        
        handle->batch = tns[0]->batch;
        if(morn_network_flag == MORN_PREDICT) value = mINIRead(ini,"para","predict_batch"); 
        else                                  value = mINIRead(ini,"para","train_batch"); 
        if(value != NULL) handle->batch = atoi(value);

        int index_input = mTensorRegisterIndex("Input" );
        int index_output= mTensorRegisterIndex("Output");
        
        handle->idx = mMalloc(handle->net->num*sizeof(int));
        for(int i=0;i<handle->net->num;i++)
        {
            MLayer *layer = handle->net->data[i];
            if((layer->type_index!=index_input)&&(layer->type_index!=index_output)) continue;
            
            value=mINIRead(ini,layer->name,"channel");mException((value==NULL),EXIT,"no input channel");int channel=atoi(value);
            value=mINIRead(ini,layer->name,"height" );mException((value==NULL),EXIT,"no input height" );int height =atoi(value);
            value=mINIRead(ini,layer->name,"width"  );mException((value==NULL),EXIT,"no input width"  );int width  =atoi(value);
            
            mTensorRedefine(layer->tns,0,channel,height,width,NULL);
            layer->tns->batch = handle->batch;
            
            handle->idx[handle->num] = i;handle->num+=1;
        }
        
        hdl->valid = 1;
    }
    
    for(int i=0;i<handle->num;i++)
    {
        MLayer *layer=handle->net->data[handle->idx[i]];
        int j;
        for(j=0;j<handle->num;j++) 
        {
            if(strcmp(layer->name,name[j])==0)
            {
                printf("handle->batch is %d,%d\n",handle->batch,tns[j]->batch);
                printf("layer->tns->channel is %d,%d\n",layer->tns->channel,tns[j]->channel);
                printf("layer->tns->width is %d,%d\n",layer->tns->width,tns[j]->width);
                printf("layer->tns->height is %d,%d\n",layer->tns->height,tns[j]->height);
                
                mException((handle->batch!=tns[j]->batch)||(layer->tns->channel!=tns[j]->channel)||
                       (layer->tns->width!=tns[j]->width)||(layer->tns->height !=tns[j]->height ),EXIT,"invalid input");
                mTensorRedefine(layer->tns,handle->batch,DFLT,DFLT,DFLT,tns[j]->data);
                break;
            }
        }
        mException((j==handle->num),EXIT,"no input tensor named %s",layer->name);
    }
}
