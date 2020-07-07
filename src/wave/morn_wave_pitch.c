/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_wave.h"

void mWaveSelfCorrelation(MWave *src,int check_length,int check_step,float **self_corr)
{
    int size;
    int i,j;
    int cn;
    int n;
    float *check;
    
    mException(INVALID_WAVE(src),EXIT,"invalid operate");
    mException(INVALID_POINTER(self_corr),EXIT,"invalid input");
    
    if(check_step <= 0)
        check_step = 1;
    
    if(check_length<=0)
    {
        float frequency = mInfoGet(&(src->info),"frequency");
        mException(mIsNan(frequency),EXIT,"invalid check length");
        check_length = (int)(frequency/50.0f);
        if(check_length > (src->size>>1))
            check_length = src->size>>1;
    }
    check_length = check_length/check_step;
    mException((check_length<=0),EXIT,"invalid operate");        
    
    size = src->size - check_length*check_step;
    mException((size<=0),EXIT,"invalid operate");
    
    printf("size is %d\n",size);
    
    for(cn = 0;cn<src->channel;cn++)
    {
        n = 0;
        for(j=0;j<check_length;j++)
        {
            check = src->data[cn] + n;
            self_corr[cn][j] = 0.0;
            
            for(i=0;i<size;i++)
                self_corr[cn][j] = self_corr[cn][j] + (src->data[cn][i]*check[i]);
            
            n = n+check_step;
        }
    }
}

void mWavePitch(MWave *src,int check_length,int *pitch,float *correlation)
{
    int diff=0;
    float corr;
    float corr_max;
    
    float *data;
    
    int i,j,cn;
    
    mException(INVALID_WAVE(src),EXIT,"invalid input");
    
    int wave_size = src->size;
    
    if(check_length<=0)
    {
        float frequency = mInfoGet(&(src->info),"frequency");
        mException(mIsNan(frequency),EXIT,"invalid check length");
        check_length = (int)(frequency/50.0f);
        if(check_length > (src->size>>1))
            check_length = src->size>>1;
    }
    
    mException((check_length > (wave_size>>1)),EXIT,"invalid operate");
    
    wave_size = wave_size - check_length;
    
    for(cn=0;cn<src->channel;cn++)
    {
        data = src->data[cn];
        corr_max = 0.0;
        for(j=0;j<check_length;j++)
        {
            corr = 0;
            for(i=0;i<wave_size-check_length;i++)
                corr = corr + data[i]*data[i+j];
            
            if(corr >corr_max)
            {
                diff = j;
                corr_max = corr;
            }
        }
        
        if(!INVALID_POINTER(correlation))
            correlation[cn] = corr_max;
        if(!INVALID_POINTER(pitch))
            pitch[cn] = diff;
    }
}
