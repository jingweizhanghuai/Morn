/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_wave.h"

#define MAX_PEAK_NUM 32

void mWavePeak(MWave *wavSrc,int peak_width,float peak_height,int *peak_locate[],int peak_num[])
{
    mException((INVALID_WAVE(wavSrc)),EXIT,"invalid input");
    
    if(peak_height == MORN_DEFAULT)
        peak_height = 0.0f;
    mException((peak_height < 0.0f),EXIT,"invalid input");
    
    if(peak_width == MORN_DEFAULT)
        peak_width = 0;
    mException((peak_width < 0),EXIT,"invalid input");    
    
    int wav_size = wavSrc->size;
    
    int n = ((peak_width)>>1);
    int num = 0;
    
    for(int j=0;j<wavSrc->channel;j++)
    {
        float *data = wavSrc->data[j];
        for(int i=n+1;i<wav_size-n-1;i++)
        {
            if((data[i]-data[i-n-1]>peak_height)&&(data[i]-data[i+n+1]>peak_height))
            {
                int flag = 1;
                for(int k=i-n;k<=i+n;k++)
                    if(data[k]>data[i])
                        flag = 0;
                
                if(flag)
                {
                    peak_locate[j][num] = i;
                    num = num +1;
                    if(num == MAX_PEAK_NUM)
                        break;
                }
            }
        }
        if(!INVALID_POINTER(peak_num))
            peak_num[j] = num;
    }
}

void mWaveMainPeak(MWave *wavSrc,int peak_width,float peak_height,int *peak_locate)
{
    mException((INVALID_WAVE(wavSrc))||(INVALID_POINTER(peak_locate)),EXIT,"invalid input");
    
    int cn = wavSrc->channel;
    
    int *peaklocate[MORN_MAX_WAVE_CN];
    for(int i=0;i<cn;i++)    
        peaklocate[i] = (int *)malloc(MAX_PEAK_NUM*sizeof(int));

    int peak_num[MORN_MAX_WAVE_CN];
    mWavePeak(wavSrc,peak_width,peak_height,peaklocate,peak_num);
    
    for(int i=0;i<cn;i++)
    {
        peak_locate[i] = -1;
        int value_max = 0.0f;
        for(int j=0;j<peak_num[i];j++)
        {
            int value = wavSrc->data[i][peaklocate[i][j]];
            if(value>value_max)
            {
                value_max = value;
                peak_locate[i] = peaklocate[i][j];
            }
        }
        
        free(peaklocate[i]);
    }
}

void mWavePSNormalize(MWave *wavSrc,MWave *wavDst,float norm_value);

void mWaveMainFrequency(MWave *ps,int *peak_locate)
{
    mException((INVALID_WAVE(ps))||(INVALID_POINTER(peak_locate)),EXIT,"invalid input");
    int wav_size = ps->size;
    int wave_type;mPropertyRead(ps,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"invalid input");

    MWave *wavSrc;
    int peak_height;
    float normalize_value=0.0;
    if(mPropertyRead(ps,"normalize",&normalize_value)==NULL)
    {
        wavSrc = mWaveCreate(ps->channel,wav_size,NULL);
        mWavePSNormalize(ps,wavSrc,((ps->size)<<4));
        peak_height = 32;
    }
    else
    {
        wavSrc = ps;
        peak_height = (int)(32.0f*normalize_value/((float)((ps->size)<<4)));
    }
    
    char *flag = (char *)mMalloc(wav_size*sizeof(char));
    int peak[32];int height[32];int height_max;
    for(int cn=0;cn<wavSrc->channel;cn++)
    {
        memset(flag,1,wav_size*sizeof(char));

        float *data = wavSrc->data[cn];
        int n = 0;
        for(int i=2;i<wav_size-2;i++)
        {
            if((data[i]>data[i-1])&&(data[i]>data[i+1])&&(data[i]-data[i-2]>peak_height)&&(data[i]-data[i+2]>peak_height))
            {
                peak[n] = i;
                height[n] = data[i];
    
                for(int l=i+i;l<wav_size-1;l=l+i)
                {
                    flag[l] = 0;
                    flag[l-1] = 0;
                    flag[l+1] = 0;
                }
                
                if(flag[i])
                {
                    n = n+1;
                    if(n==32)
                        break;
                }
            }
        }
                    
        if(n==0)
        {
            peak_locate[cn] = -1;
            continue;
        }
        
        peak_locate[cn] = peak[0];
        height_max = height[0];
        for(int i=1;i<n;i++)
        {
            if(height[i]>height_max)
            {
                peak_locate[cn] = peak[i];
                height_max = height[i];
            }
        }
        if(height_max < 300)
            peak_locate[cn] = -1;
    }
    
    mFree(flag);
    
    if(wavSrc != ps)
        mWaveRelease(wavSrc);
}

void mWaveZeroCrossRatio(MWave *src,float *zcr)
{
    mException((INVALID_WAVE(src))||(INVALID_POINTER(zcr)),EXIT,"invalid input");
    mException(INVALID_WAVE(src),EXIT,"invalid operate");
    for(int cn=0;cn<src->channel;cn++)
    {
        int *data = (int *)src->data[cn];
        int num = 0;
        for(int i=1;i<src->size;i++)
            num = num +(((data[i])^(data[i-1]))<0);
        
        zcr[cn] = ((float)num)/((float)src->size);
    }
}
