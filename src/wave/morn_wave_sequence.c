#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Wave.h"

/*
typedef struct MWaveSequenceHandle
{
    MWave *wave[128];
    int length;
    int interval;
    
    int count;
    
    MMethod method[16];
}MWaveSequenceHandle;

// #define MORN_WAVE_AVERAGE        0
// #define MORN_VOLUME_AVERAGE      1
// #define MORN_VOLUME_BACKGROUND   2
// #define MORN_VOLUME_MAX          3

void *mStartWaveSequence(int sequence_length,int wave_size)
{
    int i;
    
    MWaveSequenceHandle *phandle;
    phandle = (MWaveSequenceHandle *)mMalloc(sizeof(MWaveSequenceHandle));
    
    if(sequence_length <128)
    {
        phandle->length = 128;
        phandle->interval = 1;
    }
    else
    {
        for(i=0;;i++)
        {
            phandle->length = sequence_length/i;
            if(phandle->length<128)
            {
                phandle->interval = i;
                break;
            }
        }
    }
    
    phandle->count = 0;
    
    memset((phandle->method),0,16*sizeof(MMethod));
    
    for(i=0;i<phandle->length;i++)
        phandle->wave[i] = mCreateWave(wave_size,1);
    
    return (void *)phandle;
}

struct WaveAverage
{
    MWave *average;
    MWave *sum;
};

static MWave *AverageWave(MWaveSequenceHandle *phandle)
{
    struct WaveAverage *method;
    
    MWave **wave;
    int wave_size;
    
    MWave *sum;
    MWave *average; 

    int length;
    
    int i;
    
    method = phandle->method[MORN_WAVE_AVERAGE].method;
    
    wave = phandle->wave;
    wave_size = wave[0]->size;
    
    average = method->average;
    sum = method->sum;
    
    length = phandle->length;
    
    for(i=0;i<wave_size;i++)
        sum->data[0][i] = sum->data[0][i] + wave[0]->data[0][i] - wave[length-1]->data[0][i];
    
    average->data[0][i] = sum->data[0][i]/((float)(length-1));
    
    return average;
}

static float *AverageVolume(MWaveSequenceHandle *phandle)
{
    float *mean;
    
    MWave **wave;
    int wave_size;

    int length;
    
    int i;
    
    float *result;
    
    mean = phandle->method[MORN_WAVE_AVERAGE].method;
    
    wave = phandle->wave;
    wave_size = wave[0]->size;
    
    length = phandle->length;
    
    result = mean + length;
    
    *result = 0.0;
    for(i=length-1;i>0;i--)
    {
        mean[i] = mean[i-1];
        *result = *result + mean[i];
    }
    
    mean[0] = 0.0;
    for(i=0;i<wave_size;i++)
        mean[0] = mean[0] + wave[i]->data[0][i];
    
    mean[0] = mean[0]/((float)wave_size);
    *result = *result + mean[0];
    
    *result = *result/((float)(length-1));
    return result;
}

static float *BackgroundVolume(MWaveSequenceHandle *phandle)
{
    float *mean;
    
    MWave **wave;
    int wave_size;

    int length;
    
    int i;
    
    float *result;
    
    float min;
    
    mean = phandle->method[MORN_WAVE_AVERAGE].method;
    
    wave = phandle->wave;
    wave_size = wave[0]->size;
    
    length = phandle->length;
    
    result = mean + length;
    
    min = 1.0;
    for(i=length-1;i>0;i--)
    {
        mean[i] = mean[i-1];
        min = (mean[i]<min)?mean[i]:min;
    }
    
    mean[0] = 0.0;
    for(i=0;i<wave_size;i++)
        mean[0] = mean[0] + wave[i]->data[0][i];
    
    mean[0] = mean[0]/((float)wave_size);
    min = (mean[0]<min)?mean[0]:min;
    
    *result = *result*0.7 + min*0.3;
    return result;
}

static float *MaxVolume(MWaveSequenceHandle *phandle)
{
    float *mean;
    
    MWave **wave;
    int wave_size;

    int length;
    
    int i;
    
    float *result;
    
    float max;
    
    mean = phandle->method[MORN_WAVE_AVERAGE].method;
    
    wave = phandle->wave;
    wave_size = wave[0]->size;
    
    length = phandle->length;
    
    result = mean + length;
    
    max = 1.0;
    for(i=length-1;i>0;i--)
    {
        mean[i] = mean[i-1];
        max = (mean[i]<max)?mean[i]:max;
    }
    
    mean[0] = 0.0;
    for(i=0;i<wave_size;i++)
        mean[0] = mean[0] + wave[i]->data[0][i];
    
    mean[0] = mean[0]/((float)wave_size);
    max = (mean[0]<max)?mean[0]:max;
    
    *result = max;
    return result;
}

void *mWaveSequence(void *handle,MWave *src,int method)
{
    int i;
    
    MWave **wave;
    int wave_size;
    
    int length;
    
    void *data;    
    float *result;
    
    float *p;
    
    MWaveSequenceHandle *phandle;
    mException((INVALID_POINTER(handle)),"invalid input",EXIT);
    phandle = (MWaveSequenceHandle *)handle;
    
    mException((method > 16),"invalid input",EXIT);
    
    phandle->count = phandle->count +1;
    if(phandle->count != phandle->interval)
        return;

    phandle->count = 0;
    
    wave = phandle->wave;
    wave_size = wave[0]->size;
    mException((src->size != wave_size),"invalid wave",EXIT);
    
    length = phandle->length;
    
    p = wave[length-1]->data[0];
    for(i=length-1;i>0;i--)
        wave[i]->data[0] = wave[i-1]->data[0];
    
    wave[0]->data[0] = p;
    memcpy(p,src->data[0],src->size*sizeof(float));
    
    if(phandle->method[method].mode == 0)
    {
        phandle->method[method].mode =1;
        
        if(method == MORN_WAVE_AVERAGE)
        {
            data = mMalloc(sizeof(struct WaveAverage));
            phandle->method[method].method = data;
        
            ((struct WaveAverage *)data)->average = mCreateWave(wave_size,1);
            ((struct WaveAverage *)data)->sum = mCreateWave(wave_size,1);
        }
        else if((method == MORN_VOLUME_AVERAGE)||(method == MORN_VOLUME_BACKGROUND)||(method == MORN_VOLUME_MAX))
        {
            data = mMalloc((length+1)*sizeof(float));
            phandle->method[method].method = data;
        }
        else
            mException(1,"no such method",EXIT);
    }
    
    if(method == MORN_WAVE_AVERAGE)
        return (void *)AverageWave(phandle);
    else if(method == MORN_VOLUME_AVERAGE)
        return (void *)AverageVolume(phandle);
    else if(method == MORN_VOLUME_BACKGROUND)
        return (void *)BackgroundVolume(phandle);
    else if(method == MORN_VOLUME_MAX)
        return (void *)MaxVolume(phandle);
    else
        mException(1,"no such method",EXIT);
}


void mFinishWaveSequence(void *handle)
{
    int i;
    
    void *data;

    MWaveSequenceHandle *phandle;
    mException((INVALID_POINTER(handle)),"invalid input",EXIT);
    phandle = (MWaveSequenceHandle *)handle;
    
    for(i=0;i<128;i++)
    {
        if(phandle->wave[i] != NULL)
            mReleaseWave(phandle->wave[i]);
    }
    
    if(phandle->method[MORN_WAVE_AVERAGE].mode == 1)
    {
        data = phandle->method[MORN_WAVE_AVERAGE].method;
        mReleaseWave(((struct WaveAverage *)data)->average);
        mReleaseWave(((struct WaveAverage *)data)->sum);
        mFree(data);
    }
    
    if(phandle->method[MORN_VOLUME_AVERAGE].mode == 1)
    {
        data = phandle->method[MORN_VOLUME_AVERAGE].method;
        mFree(data);
    }
    if(phandle->method[MORN_VOLUME_BACKGROUND].mode == 1)
    {
        data = phandle->method[MORN_VOLUME_BACKGROUND].method;
        mFree(data);
    }
    if(phandle->method[MORN_VOLUME_MAX].mode == 1)
    {
        data = phandle->method[MORN_VOLUME_MAX].method;
        mFree(data);
    }
    
    mFree(handle);
}
*/





