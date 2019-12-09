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
