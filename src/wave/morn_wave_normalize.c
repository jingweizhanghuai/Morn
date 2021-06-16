/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_wave.h"

void mWavePSNormalize(MWave *src,MWave *dst,float norm_value)
{
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type=-1;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"invalid input");
    
    int wav_size = src->size;
    if(norm_value == MORN_DEFAULT)
        norm_value = (float)wav_size;
    
    float **data_src = src->data;
    float **data_dst;
    if(INVALID_POINTER(dst))
    {
        data_dst = src->data;
        mPropertyWrite(src,"normalize",&norm_value,sizeof(float));
    }
    else
    {
        mWaveRedefine(dst,src->channel,wav_size,dst->data);
        data_dst = dst->data;
        // dst->info = src->info;
        mPropertyWrite(src,"normalize",&norm_value,sizeof(float));
    }
    
    for(int cn = 0;cn<src->channel;cn++)
    {
        float sum = 0.0f;
        for(int i=0;i<wav_size;i++)
            sum = sum + data_src[cn][i];
        
        if(sum > 0.0f)
        {
            float k = norm_value/sum;
        
            for(int i=0;i<wav_size;i++)
                data_dst[cn][i] = data_src[cn][i]*k;
        }
    }
}

void mWaveNormalize(MWave *src,MWave *dst,float norm_value)
{
    int wav_size;
    float max;
    float k;
    int i,j;
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    
    wav_size = src->size;
    if(INVALID_POINTER(dst))
        dst = src; 
    
    if(dst != src)
    {
        mWaveRedefine(dst,src->channel,wav_size,dst->data);
        // dst->info = src->info;
    }
    
    if(norm_value < 0)
        norm_value = 1.0f;
    
    for(j=0;j<src->channel;j++)
    {
        max = 0;
        for(i=0;i<wav_size;i++)
            max = (ABS(src->data[j][i])>max)?ABS(src->data[j][i]):max;
        
        k = norm_value/((float)max);
        
        for(i=0;i<wav_size;i++)
            dst->data[j][i] = (src->data[j][i])*k;
    }    
    mPropertyWrite(src,"normalize",&norm_value,sizeof(float));
}
