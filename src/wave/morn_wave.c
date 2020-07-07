/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_wave.h"

struct HandleWaveCreate
{
    MWave *wave;
    int channel;
    int size;
    float *index[MORN_MAX_WAVE_CN];
    MMemory *memory;
};
void endWaveCreate(void *info)
{
    struct HandleWaveCreate *handle = (struct HandleWaveCreate *)info;
    mException((handle->wave == NULL),EXIT,"invalid wave");
    
    if(handle->memory != NULL)
        mMemoryRelease(handle->memory);
    
    mFree(handle->wave);
}
#define HASH_WaveCreate 0xa08b9c64
MWave *mWaveCreate(int cn,int size,float **data)
{
    if(size <0) size = 0;
    if(cn <0)   cn = 0;
    mException((cn>MORN_MAX_WAVE_CN),EXIT,"invalid input");
    
    MWave *wave = (MWave *)mMalloc(sizeof(MWave));
    memset(wave,0,sizeof(MWave));
    
    wave->size = size;
    wave->channel = cn;

    wave->handle = mHandleCreate();
    MHandle *hdl=mHandle(wave,WaveCreate);
    struct HandleWaveCreate *handle = (struct HandleWaveCreate *)(hdl->handle);
    handle->wave = wave;
    
    if((size == 0)||(cn==0))
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        memset(wave->data,0,MORN_MAX_WAVE_CN*sizeof(float *));
    }
    else if(INVALID_POINTER(data))
    {
        size = size +32;
        void **index[MORN_MAX_WAVE_CN];for(int i=0;i<cn;i++) index[i]=(void **)(&(handle->index[i]));
        
        if(handle->memory == NULL) handle->memory = mMemoryCreate(cn,size*sizeof(float),MORN_HOST_CPU);
        mMemoryIndex(handle->memory,1,size*sizeof(float),index,cn);
        handle->size = size;
        handle->channel = cn;
        
        for(int k=0;k<cn;k++)
            wave->data[k] = handle->index[k]+16;
    }
    else
        memcpy(wave->data,data,sizeof(float *)*cn);
    
    return wave;
}

void mWaveRelease(MWave *wave)
{
    mException(INVALID_POINTER(wave),EXIT,"invalid input");
    
    if(!INVALID_POINTER(wave->handle))
        mHandleRelease(wave->handle);
}

void mWaveRedefine(MWave *src,int cn,int size,float **data)
{   
    mException((INVALID_POINTER(src)),EXIT,"invalid input");
    
    if(size <= 0) size = src->size;
    if(cn   <= 0) cn   = src->channel;
    if((cn!=src->channel)||(size!=src->size)) mHandleReset(src->handle);
    
    int same_size = ((size <= src->size)&&(cn <= src->channel));
    int reuse = (data==src->data);
    int flag = (src->size)&&(src->channel);
    
    src->size = size;
    src->channel = cn;
    
    if(same_size&&reuse) return;
    struct HandleWaveCreate *handle = (struct HandleWaveCreate *)(((MHandle *)(src->handle->data[0]))->handle);
    if(same_size&&(data==NULL)&&(handle->size >0)) return;
    mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");
    
    mException((cn>MORN_MAX_WAVE_CN),EXIT,"invalid input");
    
    handle->size = 0;
    if((cn<=0)||(size<=0)) 
    {
        mException((data!=NULL)&&(!reuse),EXIT,"invalid input");
        memset(src->data,0,MORN_MAX_WAVE_CN*sizeof(float *));
        return;
    }

    if(reuse) data=NULL;
    
    if(data!=NULL) {memcpy(src->data,data,cn*sizeof(float *));return;}
    
    if((size > handle->size)||(cn > handle->channel))
    {
        size = size +32;
        void **index[MORN_MAX_WAVE_CN];for(int i=0;i<cn;i++) index[i]=(void **)(&(handle->index[i]));
        if(handle->memory == NULL) handle->memory = mMemoryCreate(cn,size*sizeof(float),MORN_HOST_CPU);
        else mMemoryRedefine(handle->memory,cn,size*sizeof(float),MORN_HOST_CPU);
        mMemoryIndex(handle->memory,1,size*sizeof(float),index,cn);
        handle->size = size;
        handle->channel = cn;
    }
    
    for(int k=0;k<cn;k++)
        src->data[k] = handle->index[k]+16;
}

void mWaveCut(MWave *src,MWave *dst,int locate,int size)
{
    int cn;
    
    mException(INVALID_WAVE(src),EXIT,"invalid input");
    
    if(locate < 0)
        locate = 0;
    if(size < 0)
    {
        if(!INVALID_WAVE(dst))
            size = dst->size;
        else
            size = src->size -locate;
    }
    
    if(INVALID_POINTER(dst))
        dst = src;
    
    mException((locate+size>src->size),EXIT,"invalid input");
    
    if((locate == 0)&&(size == src->size)&&(dst==src))
        return;

    if(dst != src)
    {
        mWaveRedefine(dst,src->channel,size,dst->data);
        dst->info = src->info;
    }
    
    for(cn=0;cn<src->channel;cn++)
        memcpy(dst->data[cn],src->data[cn]+locate,size*sizeof(float));
}

void mWavMean(MWave *src,float *mean)
{
    int wav_size;
    int i,j;
    float sum;
        
    mException((INVALID_WAVE(src))||(INVALID_POINTER(mean)),EXIT,"invalid input");
    
    sum = 0.0;    
    wav_size = src->size;
    for(j=0;j<src->channel;j++)
    {
        for(i=0;i<wav_size;i++)
            sum = sum + src->data[j][i];
        mean[j] = sum/((float)wav_size);
    }
}

void mWavABSMean(MWave *src,float *mean)
{
    int wav_size;
    int i,j;
    float sum;
    float **data;
    
    mException((INVALID_WAVE(src))||(INVALID_POINTER(mean)),EXIT,"invalid input");
    
    data = src->data;
    sum = 0.0;
    wav_size = src->size;
    for(j=0;j<src->channel;j++)
    {
        for(i=0;i<wav_size;i++)
            sum = (data[j][i]>0)?(sum+data[j][i]):(sum-data[j][i]);
        
        mean[j] = sum/((float)wav_size);
    }
}

void mWavSquarMean(MWave *src,float *mean)
{
    int wav_size;
    int i,j;
    float sum;
    float **data;
    
    mException((INVALID_WAVE(src))||(INVALID_POINTER(mean)),EXIT,"invalid input");
    
    sum = 0.0;
    data = src->data;
    wav_size = src->size;
    for(j=0;j<src->channel;j++)
    {
        for(i=0;i<wav_size;i++)
            sum = sum + data[j][i]*data[j][i];
        mean[j] = sum/((float)wav_size);
    }
}

void mWaveAdd(MWave *src1,MWave *src2,MWave *dst)
{
    int wav_size;    
    int cn,i;
        
    mException((INVALID_WAVE(src1))||(INVALID_WAVE(src2)),EXIT,"invalid input");
    wav_size = (src1->size<src2->size)?src1->size:src2->size;
    mException((src2->channel != src1->channel)&&(src2->channel!=1),EXIT,"invalid input");

    if(INVALID_POINTER(dst))
        dst = src1;
    dst->info = src1->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src1->channel,wav_size,dst->data);
    
    if(src2->channel == 0)
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = src1->data[cn][i] + src2->data[cn][0];
    }
    else
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = src1->data[cn][i] + src2->data[cn][i];
    }
}

void mWaveSub(MWave *src1,MWave *src2,MWave *dst)
{
    int wav_size;    
    int cn,i;
        
    mException((INVALID_WAVE(src1))||(INVALID_WAVE(src2)),EXIT,"invalid input");
    wav_size = (src1->size<src2->size)?src1->size:src2->size;
    mException((src2->channel != src1->channel)&&(src2->channel !=1),EXIT,"invalid input");

    if(INVALID_POINTER(dst))
        dst = src1;
    dst->info = src1->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src1->channel,wav_size,dst->data);
    
    if(src2->channel == 1)
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = src1->data[cn][i] - src2->data[cn][0];
    }
    else
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = src1->data[cn][i] - src2->data[cn][i];
    }
}

void mWaveAverage(MWave *src1,MWave *src2,MWave *dst)
{
    int wav_size;    
    int cn,i;
        
    mException((INVALID_WAVE(src1))||(INVALID_WAVE(src2)),EXIT,"invalid input");
    wav_size = (src1->size<src2->size)?src1->size:src2->size;
    mException((src2->channel != src1->channel)&&(src2->channel != 1),EXIT,"invalid input");

    if(INVALID_POINTER(dst))
        dst = src1;
    dst->info = src1->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src1->channel,wav_size,dst->data);
    
    if(src2->channel == 1)
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = (src1->data[cn][i] + src2->data[cn][0])/2.0f;
    }
    else
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = (src1->data[cn][i] + src2->data[cn][i])/2.0f;
    }
}

void mWaveWeightedAverage(MWave *src1,MWave *src2,MWave *dst,float weight1,float weight2)
{
    int wav_size;    
    int cn,i;
        
    mException((INVALID_WAVE(src1))||(INVALID_WAVE(src2)),EXIT,"invalid input");
    wav_size = (src1->size<src2->size)?src1->size:src2->size;
    mException((src2->channel != src1->channel)&&(src2->channel !=1),EXIT,"invalid input");

    if((weight1 == MORN_DEFAULT)&&(weight2 == MORN_DEFAULT))
    {
        mWaveAverage(src1,src2,dst);
        return;
    }
    else if((weight1 == MORN_DEFAULT)&&(weight2 < 1.0f)&&(weight2 > 0.0f))
        weight1 = 1.0f - weight2;
    else if((weight2 == MORN_DEFAULT)&&(weight1 < 1.0f)&&(weight1 > 0.0f))
        weight2 = 1.0f - weight1;
    else if((weight1 == MORN_DEFAULT)||(weight2 == MORN_DEFAULT))
        mException(1,EXIT,"invalid input");
    
    if(INVALID_POINTER(dst))
        dst = src1;
    dst->info = src1->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src1->channel,wav_size,dst->data);
    
    if(src2->channel == 1)
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = (src1->data[cn][i]*weight1 + src2->data[cn][0]*weight2)/(weight1+weight2);
    }
    else
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = (src1->data[cn][i]*weight1 + src2->data[cn][i]*weight2)/(weight1+weight2);
    }
}

void mWaveScale(MWave *src,MWave *dst,float k)
{
    int wav_size;
    int cn,i;
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    wav_size = src->size;
    
    if(INVALID_POINTER(dst))
        dst = src;
    dst->info = src->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src->channel,wav_size,dst->data);
    
    for(cn = 0;cn<src->channel;cn++)
        for(i=0;i<wav_size;i++)
            dst->data[cn][i] = src->data[cn][i]*k;
}

void mWaveMul(MWave *src1,MWave *src2,MWave *dst)
{
    int wav_size;
    int cn,i;
    
    mException((INVALID_WAVE(src1))||(INVALID_WAVE(src2)),EXIT,"invalid input");
    wav_size = (src1->size<src2->size)?src1->size:src2->size;
    mException((src2->channel != src1->channel)&&(src2->channel !=1),EXIT,"invalid input");

    if(INVALID_POINTER(dst))
        dst = src1;
    dst->info = src1->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src1->channel,wav_size,dst->data);
    
    if(src2->channel == 1)
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = src1->data[cn][i] * src2->data[0][i];
    }
    else
    {
        for(cn = 0;cn<src1->channel;cn++)
            for(i=0;i<wav_size;i++)
                dst->data[cn][i] = src1->data[cn][i] * src2->data[cn][i];
    }
}

void mWaveDiv(MWave *src1,MWave *src2,MWave *dst)
{
    int wav_size;
    int cn,i;
    
    mException((INVALID_WAVE(src1))||(INVALID_WAVE(src2)),EXIT,"invalid input"); 
    wav_size = (src1->size<src2->size)?src1->size:src2->size;
    mException((src2->channel != src1->channel)&&(src2->channel !=1),EXIT,"invalid input");

    if(INVALID_POINTER(dst))
        dst = src1;
    dst->info = src1->info;
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    mWaveRedefine(dst,src1->channel,wav_size,dst->data);
  
    for(cn = 0;cn<src1->channel;cn++)
        for(i=0;i<wav_size;i++)
            dst->data[cn][i] = src1->data[cn][i] / src2->data[cn][i];
}

void mWaveOperate(MWave *src,MWave *dst,float (*func)(float,void *),void *para)
{
    int i;
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) dst = src;
    else mWaveRedefine(dst,src->channel,src->size,dst->data);
    
    for(int cn=0;cn<src->channel;cn++)
    {
        #pragma omp parallel for
        for(i=0;i<src->size;i++)
            dst->data[cn][i] = func(src->data[cn][i],para);
    }
}

