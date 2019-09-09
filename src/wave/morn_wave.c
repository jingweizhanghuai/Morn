#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Wave.h"

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
    struct HandleWaveCreate *handle = info;
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
    
    MHandle *hdl; ObjectHandle(wave,WaveCreate,hdl);
    struct HandleWaveCreate *handle = hdl->handle;
    handle->wave = wave;
    
    if((size == 0)||(cn==0))
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        memset(wave->data,0,MORN_MAX_WAVE_CN*sizeof(float *));
    }
    else if(INVALID_POINTER(data))
    {
        size = size +32;
        if(handle->memory == NULL) handle->memory = mMemoryCreate(cn,size*sizeof(float));
        mMemoryIndex(handle->memory,cn,size*sizeof(float),(void **)(handle->index));
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
    struct HandleWaveCreate *handle = ((MHandle *)(src->handle->data[0]))->handle;
    if(same_size&&(data==NULL)&&(handle->size >0)) return;
    mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");
    
    mException((cn>MORN_MAX_WAVE_CN),EXIT,"invalid input");
    if(flag) data=NULL;
    handle->size = 0;
    
    if((cn<=0)||(size<=0)) {memset(src->data,0,MORN_MAX_WAVE_CN*sizeof(float *));return;}
    
    if(data!=NULL) {memcpy(src->data,data,cn*sizeof(float *));return;}
    
    if((size > handle->size)||(cn > handle->channel))
    {
        size = size +32;
        if(handle->memory == NULL) handle->memory = mMemoryCreate(cn,size*sizeof(float));
        mMemoryIndex(handle->memory,cn,size*sizeof(float),(void **)(handle->index));
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

/*
void mWaveThreshold(MWave *src,MWave *dst,MThreshold *thresh)
{
    int cn,i;
    
    mException((INVALID_WAVE(src)||INVALID_POINTER(thresh)),EXIT,"invalid input");    
    
    if(INVALID_POINTER(dst))
        dst = src;
    else
    {
        mWaveRedefine(dst,src->size,src->channel);
        dst->info = src->info;
    }

    for(cn=0;cn<src->channel;cn++)
        for(i=0;i<src->size;i++)
            dst->data[cn][i] = ThresholdData((src->data[cn][i]),thresh,f32);
}

    

/////////////////////////////////////////////////////////
// 接口功能:
//  波形阈值截断
//
// 参数：
//  (I)src(NO) - 输入的源波形
//  (O)dst(src) - 算得的结果波形
//  (I)thresh1(-1.0) - 幅值的下阈值
//  (I)thresh2(1.0) - 幅值的上阈值
//  
// 返回值：
//  无
/////////////////////////////////////////////////////////

void mWaveThreshold(MWave *src,MWave *dst,float thresh1,float thresh2)
{
    int wav_size;
    int cn,i;
    float th1,th2;
    
    mException((INVALID_WAVE(src)),"invalid input",EXIT);    
    wav_size = src->size;
    
    if(INVALID_POINTER(dst))
        dst = src;
    dst->info = src->info;
    mWaveRedefine(dst,wav_size,src->channel);
    
    if(thresh1 < thresh2)
    {
        th1 = thresh1;
        th2 = thresh2;
    }
    else
    {
        th2 = thresh1;
        th1 = thresh2;
    }
    if(th2 <0.0)
        th2 = 0.0-th2;
    
    for(cn = 0;cn<src->channel;cn++)
        for(i=0;i<wav_size;i++)
        {
            if(src->data[cn][i]>th2)
                dst->data[cn][i] = th2;
            else if(src->data[cn][i]<th1)
                dst->data[cn][i] = th1;
            else
                dst->data[cn][i] = src->data[cn][i];
        }
        
    if(th2 == 0.0-th1)
        dst->info.normalize_value = th2;
    else
        dst->info.normalize_value = MORN_NOT_NORMALIZED;
}
*/
    
    




