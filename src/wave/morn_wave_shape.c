/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_wave.h"

#define MAX_PEAK_NUM 32

void mWavePeak(MWave *wavSrc,int peak_width,float peak_height,int *peak_locate[],int peak_num[])
{
    int wav_size;
    
    float *data;
    
    int i,j,k;
    
    int n;
    int num;
    
    int flag;
    
    mException((INVALID_WAVE(wavSrc)),EXIT,"invalid input");
    
    if(peak_height == MORN_DEFAULT)
        peak_height = 0.0f;
    mException((peak_height < 0.0f),EXIT,"invalid input");
    
    if(peak_width == MORN_DEFAULT)
        peak_width = 0;
    mException((peak_width < 0),EXIT,"invalid input");    
    
    wav_size = wavSrc->size;
    
    n = ((peak_width)>>1);
    
    num = 0;
    
    for(j=0;j<wavSrc->channel;j++)
    {
        data = wavSrc->data[j];
        for(i=n+1;i<wav_size-n-1;i++)
        {
            if((data[i]-data[i-n-1]>peak_height)&&(data[i]-data[i+n+1]>peak_height))
            {
                flag = 1;
                for(k=i-n;k<=i+n;k++)
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

/////////////////////////////////////////////////////////
// 接口功能:
//  寻找波形上的最高峰
//
// 参数：
//  (I)wavSrc(NO) - 输入的功率谱
//  (I)peak_width(NO) - 峰的最大宽度
//  (I)peak_height(NO) - 峰的最小高度
//  (O)peak_locate(NO) - 计算得到的各通道的最高峰的位置
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWaveMainPeak(MWave *wavSrc,int peak_width,float peak_height,int *peak_locate)
{
    int *peaklocate[MORN_MAX_WAVE_CN];
    int peak_num[MORN_MAX_WAVE_CN];
    int value;
    int value_max;
    int cn;
    
    int i,j;
    
    mException((INVALID_WAVE(wavSrc))||(INVALID_POINTER(peak_locate)),EXIT,"invalid input");
    
    cn = wavSrc->channel;
    
    for(i=0;i<cn;i++)    
        peaklocate[i] = (int *)malloc(MAX_PEAK_NUM*sizeof(int));
    
    mWavePeak(wavSrc,peak_width,peak_height,peaklocate,peak_num);
    
    for(i=0;i<cn;i++)
    {
        peak_locate[i] = -1;
        value_max = 0.0f;
        for(j=0;j<peak_num[i];j++)
        {
            value = wavSrc->data[i][peaklocate[i][j]];
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

/////////////////////////////////////////////////////////
// 接口功能:
//  在功率谱上寻找声音的主频率
//
// 参数：
//  (I)ps(NO) - 输入的功率谱
//  (O)peak_locate(wavFFT) - 计算得到的各通道的主频率的位置
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWaveMainFrequency(MWave *ps,int *peak_locate)
{
    int n;
    int i;
    int cn;
    int l;
    
    mException((INVALID_WAVE(ps))||(INVALID_POINTER(peak_locate)),EXIT,"invalid input");
    int wav_size = ps->size;
    mException(((int)mInfoGet(&(ps->info),"wave_type") != MORN_WAVE_PS),EXIT,"invalid input");

    MWave *wavSrc;
    int peak_height;
    float normalize_value = mInfoGet(&(ps->info),"normalize");
    if(mIsNan(normalize_value))
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
    for(cn=0;cn<wavSrc->channel;cn++)
    {
        memset(flag,1,wav_size*sizeof(char));

        float *data = wavSrc->data[cn];
        n = 0;
        for(i=2;i<wav_size-2;i++)
        {
            // if(i==40)
                // printf("%f,%f,%f,%f,%f\n",data[i-2],data[i-1],data[i],data[i+1],data[i+2]);
            
            if((data[i]>data[i-1])&&(data[i]>data[i+1])&&(data[i]-data[i-2]>peak_height)&&(data[i]-data[i+2]>peak_height))
            {
                // printf("%d,",i);
                peak[n] = i;
                height[n] = data[i];
    
                for(l=i+i;l<wav_size-1;l=l+i)
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
        for(i=1;i<n;i++)
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
        for(i=1;i<src->size;i++)
            num = num +(((data[i])^(data[i-1]))<0);
        
        zcr[cn] = ((float)num)/((float)src->size);
    }
}
