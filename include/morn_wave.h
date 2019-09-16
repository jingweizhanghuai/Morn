/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _MORN_WAVE_H_
#define _MORN_WAVE_H_

#include "morn_math.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_MAX_WAVE_CN 32

#define MORN_WAVE_TD          0
#define MORN_WAVE_FD          1
#define MORN_WAVE_PS          2

#define MORN_NOT_NORMALIZED   0

typedef struct MWave {
    int channel;
    int size;
    
    float *data[MORN_MAX_WAVE_CN];
    
    MList *handle;
    MInfo info;
    
    void *reserve;
}MWave;

#define INVALID_WAVE(Wave) ((((Wave) ==NULL)||((intptr_t)(Wave) == DFLT))?1:(((Wave)->data == NULL)||((intptr_t)((Wave)->data) == DFLT)\
                                                                      ||((Wave)->channel <= 0)\
                                                                      ||((Wave)->size <= 0)))

MWave *mWaveCreate(int channel,int size,float **data);
void mWaveRelease(MWave *wave);
void mWaveRedefine(MWave *src,int channel,int size,float **data);
#define mWaveReset(Wave) mHandleReset(Wave->handle)
#define mWaveDataSet(Wave,Data,Num,Type) {\
    int I;\
    float K;\
    \
    mWaveRedefine(Wave,1,Num,Wave->data); \
    if(strcmp(#Type,"float")==0)\
        Wave->data[0] = (float *)Data;\
    else\
    {\
        if(strcmp(#Type,"short") == 0)\
            K = 32768.0;\
        else if(strcmp(#Type,"char") == 0)\
            K = 256.0;\
        else\
            mException(1,"invalid type",EXIT);\
        \
        for(I=0;I<Num;I++)\
            Wave->data[0][I] = ((float)(Data[I]))/K;\
    }\
}

void mWaveCut(MWave *src,MWave *dst,int locate,int size);
#define mWaveCopy(Src,Dst) mWaveCut(Src,Dst,0,Src->size);
#define mWaveExchange(Src,Dst) mObjectExchange(Src,Dst,MWave)

void mWavMean(MWave *src,float *mean);
void mWavABSMean(MWave *src,float *mean);
void mWavSqarMean(MWave *src,float *mean);
#define MORN_WAVE_ABS_AMPLITUDE 0
#define MORN_WAVE_AMPLITUDE 1
#define MORN_WAVE_SQUAR_AMPLITUDE 2
#define mWaveAmplitudeMean(Src,Mean,Mode) {\
    if(Mode<=0)\
        mWavABSMean(Src,Mean);\
    else if(Mode == MORN_WAVE_AMPLITUDE)\
        mWavMean(Src,Mean);\
    else if(Mode == MORN_WAVE_SQUAR_AMPLITUDE)\
        mWavSqarMean(Src,Mean);\
    else\
        mException(1,"invalid amplitude mode",EXIT);\
}

void mWaveAdd(MWave *src1,MWave *src2,MWave *dst);
void mWaveSub(MWave *src1,MWave *src2,MWave *dst);
void mWaveMul(MWave *src1,MWave *src2,MWave *dst);
void mWaveDiv(MWave *src1,MWave *src2,MWave *dst);
void mWaveAverage(MWave *src1,MWave *src2,MWave *dst);
#define MORN_WAVE_ADD 0
#define MORN_WAVE_SUB 1
#define MORN_WAVE_MUL 2
#define MORN_WAVE_DIV 3
#define MORN_WAVE_AVERAGE 4
#define mWaveArithmetic(Src1,Src2,Dst,mode) {\
    if(Mode <=0)\
        mWaveAdd(Src1,Src2,Dst);\
    else if(Mode == MORN_WAVE_SUB)\
        mWaveSub(Src1,Src2,Dst);\
    else if(Mode == MORN_WAVE_MUL)\
        mWaveMul(Src1,Src2,Dst);\
    else if(Mode == MORN_WAVE_DIV)\
        mWaveDiv(Src1,Src2,Dst);\
    else if(Mode == MORN_WAVE_AVERAGE)\
        mWaveAverage(Src1,Src2,Dst);\
    else\
        mException(1,"invalid arithmetic mode",EXIT);\
}
void mWaveScale(MWave *src,MWave *dst,float k);
void mWaveWeightedAverage(MWave *src1,MWave *src2,MWave *dst,float weight1,float weight2);

int mWAVRead(MObject *file,MWave *dst);
int mWAVWrite(MObject *file,MWave *src);

int mWaveFrameRead(MWave *wave,float **read,int frame_size,float frame_overlap);
int mWaveFrameWrite(MWave *wave,float **write,int frame_size,float frame_overlap);
#define mWaveRead(wave,read,size) mWaveFrameRead(wave,read,size,0.0)
#define mWaveWrite(wave,write,size) mWaveFrameWrite(wave,write,size,0.0)

void mWaveResample(MWave *src,MWave *dst,int src_rate,int dst_rate);

void mWaveHanningWin(MWave *win);
void mWaveHammingWin(MWave *win);
void mWaveFejerWin(MWave *win);
void mWaveBlackmanWin(MWave *win);
void mWaveWindow(MWave *src,MWave *dst,void (*win_func)(MWave *));

void mWaveFFT(MWave *wavSrc,MWave *wavFFT);
#define MORN_SQUAR_POWERS    1
#define MORN_POWERS          2
#define MORN_LOG_POWERS      3
void mWavePowerSpectrum(MWave *wavFFT,MWave *wavPS,int mode);


int mWaveActive(MWave *src,float thresh);
void mWaveBackground(MWave *src,float *background);
int mWaveAdaptiveActive(MWave *src,float sensibility,float thresh);
int mLoudSoundCheck(MWave *src,float sensibility,float thresh);
// void mGetActive(MWave *src,MWave *dst[],int *dst_num);

void mWavePSNormalize(MWave *src,MWave *dst,float norm_value);
void mWaveNormalize(MWave *src,MWave *dst,float norm_value);

void mUniformFilterBank(MWave *src, int Start, int End, int NumUniformFilter, float** UniformFilterFeature);
void mWaveMelFilterBank(MWave *src, int Start, int End, int NumMelFilter, float** MelFilterFeature);

#define MORN_WIN_HANNING     1
#define MORN_WIN_HAMMING     2
#define MORN_WIN_FEJER       3
#define MORN_WIN_BLACKMAN    4

#define MORN_SQUAR_POWERS    1
#define MORN_POWERS          2
#define MORN_LOG_POWERS      3


typedef struct MWaveIndex {
    int size;
    
    int frame_size;
    int frame_num;
    float frame_overlap;
    
    int *index;
}MWaveIndex;




MWaveIndex *mGetFrameIndex(int wav_size,int div_num,int div_size,float div_overlap);
void mReleaseWaveIndex(MWaveIndex *index);
void mWaveFrame(MWave *src,MWaveIndex *index,MWave *dst,int n);
void mWaveFrameMat(MWave *src,MWaveIndex *index,MWave *dst);


void *mWaveRecordOpen(int sample_rate);
void mWaveRecord(void *handle,MWave *dst);
void mWaveRecordClose(void *handle);

#ifdef __cplusplus
}
#endif

#endif
