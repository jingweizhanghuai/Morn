#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_Wave.h"


/////////////////////////////////////////////////////////
// 接口功能:
//  计算波形的自相关系数
//
// 参数：
//  (I)src(NO) - 待计算的波形
//  (I)check_length(src->info.frequency/50) - 检测长度
//  (I)check_step(1) - 检测步长
//  (O)self_corr(NO) - 算得的各通道的相关系数
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
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
            // printf("j is %d\n",j);
            self_corr[cn][j] = 0.0;
            
            for(i=0;i<size;i++)
                self_corr[cn][j] = self_corr[cn][j] + (src->data[cn][i]*check[i]);
            
            n = n+check_step;
        }
    }
}

/////////////////////////////////////////////////////////
// 接口功能:
//  在功率谱上寻找基音周期
//
// 参数：
//  (I)src(NO) - 待计算的波形
//  (I)check_length(src->info.frequency/50) - 检测长度
//  (O)pitch(NULL) - 测得的基音周期
//  (O)correlation(NULL) - 基音周期所对应的相关系数
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWavePitch(MWave *src,int check_length,int *pitch,float *correlation)
{
    int diff;
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
