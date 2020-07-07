/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_wave.h"

void mWaveHalfResample(MWave *src,MWave *dst)
{
    int src_size;
    int dst_size;
    
    int i;
    
    int cn;
                 
    float w[5]={0.632537035f,-0.200145897f,0.107834221f,-0.064897519f,0.039258993f/*,-0.022279798,0.010658657*/};

    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    src_size = src->size;    
    dst_size = src_size/2;
    
    if(INVALID_POINTER(dst)) dst = src;
    else mWaveRedefine(dst,src->channel,dst_size,dst->data);
    
    float frequency = mInfoGet(&(src->info),"frequency");
    if(!mIsNan(frequency)) mInfoSet(&(src->info),"frequency",frequency/2.0f);

    for(cn=0;cn<src->channel;cn++)
    {        
        dst->data[cn][0] = src->data[cn][0];
        dst->data[cn][1] = src->data[cn][2];
        dst->data[cn][2] = src->data[cn][4];
        dst->data[cn][3] = src->data[cn][6];
        dst->data[cn][4] = src->data[cn][8];
        
        for(i=5;i<dst_size-5;i++)
        {
            dst->data[cn][i] = (src->data[cn][(i<<1)] + 
                             +(src->data[cn][(i<<1)+1] + src->data[cn][(i<<1)-1])*w[0]
                             +(src->data[cn][(i<<1)+3] + src->data[cn][(i<<1)-3])*w[1]
                             +(src->data[cn][(i<<1)+5] + src->data[cn][(i<<1)-5])*w[2]
                             +(src->data[cn][(i<<1)+7] + src->data[cn][(i<<1)-7])*w[3]
                             +(src->data[cn][(i<<1)+9] + src->data[cn][(i<<1)-9])*w[4])/2.029f;
                             // +(src->data[cn][(i<<1)+11] + src->data[cn][(i<<1)-11])*w[5]
                             // +(src->data[cn][(i<<1)+13] + src->data[cn][(i<<1)-13])*w[6])/2.029;
        }
        dst->data[cn][i]   = src->data[cn][(i<<1)];
        dst->data[cn][i+1] = src->data[cn][(i<<1)+2];
        dst->data[cn][i+2] = src->data[cn][(i<<1)+4];
        dst->data[cn][i+3] = src->data[cn][(i<<1)+6];
        dst->data[cn][i+4] = src->data[cn][(i<<1)+8];
    }
}

static float g_resample_w[11] = {0.0f};
static float g_resample_wsum = 0.0f;
static float g_resample_n = 0.0f;

void mWaveIntegerResample(MWave *src,MWave *dst,int n)
{
    int i,j;
    if(n==2) {mWaveHalfResample(src,dst);return;}
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    if(INVALID_POINTER(dst)) dst = src;
    if((dst==src)&&(n<=0)) return;
    if((n<=1)&&(src != dst)){mWaveCopy(src,dst); return;}
    
    int src_size = src->size;
    int dst_size = (int)(((float)src_size)/((float)n));    
    mException(((n <= 0)||(dst_size<=1)),EXIT,"invalid input");
    
    dst->info = src->info;
    float frequency = mInfoGet(&(src->info),"frequency");
    if(!mIsNan(frequency)) mInfoSet(&(src->info),"frequency",frequency/(float)n);
    
    mWaveRedefine(dst,src->channel,dst_size,dst->data);

    if(n != g_resample_n)
    {
        double l = 0;
        g_resample_wsum = 0.0f;
        for(i=1;i<=10;i++)
        {
            l = l+(double)(MORN_PI/(double)n);
            g_resample_w[i] = (float)(sin(l)/l);  
            g_resample_wsum = g_resample_wsum + g_resample_w[i];            
        }
        g_resample_wsum = g_resample_wsum+g_resample_wsum+1.0f;
        g_resample_n = n;
    }
        
    for(int cn=0;cn<src->channel;cn++)
    {
        dst->data[cn][0] = src->data[cn][0];
        dst->data[cn][1] = src->data[cn][n];
        dst->data[cn][2] = src->data[cn][n+n];
        
        for(i=3;i<dst_size-3;i++)
        {
            int locate = n*i;
            
            float result = src->data[cn][locate];
            for(j=1;j<=10;j++)
                result = result + (src->data[cn][locate-j] + src->data[cn][locate+j])*g_resample_w[j];
            
            dst->data[cn][i] = result/g_resample_wsum;
        }
        
        dst->data[cn][i]   = src->data[cn][n*i];
        dst->data[cn][i+1] = src->data[cn][n*i+n];
        dst->data[cn][i+2] = src->data[cn][n*i+n+n];
    }
}

static float g_sin_value[51] = {
0.000000000f,0.031410759f,0.062790520f,0.094108313f,0.125333234f,0.156434465f,0.187381315f,0.218143241f,0.248689887f,0.278991106f,
0.309016994f,0.338737920f,0.368124553f,0.397147891f,0.425779292f,0.453990500f,0.481753674f,0.509041416f,0.535826795f,0.562083378f,
0.587785252f,0.612907054f,0.637423990f,0.661311865f,0.684547106f,0.707106781f,0.728968627f,0.750111070f,0.770513243f,0.790155012f,
0.809016994f,0.827080574f,0.844327926f,0.860742027f,0.876306680f,0.891006524f,0.904827052f,0.917754626f,0.929776486f,0.940880769f,
0.951056516f,0.960293686f,0.968583161f,0.975916762f,0.982287251f,0.987688341f,0.992114701f,0.995561965f,0.998026728f,0.999506560f,
1.0f};

#define SINC(in,out) {\
    float aa;\
    int n;\
    float x;\
    \
    x = in;\
    if(x == 0.0f)\
        out = MORN_PI;\
    else\
    {\
        aa = x/2.0f;\
        aa =(aa>0.0f)?aa:(0.0f-aa);\
        aa = aa - (float)((int)aa);\
        n = (int)(aa*200.0f+0.5f);\
        if(n<=50)\
            out = g_sin_value[n];\
        else if(n<=100)\
            out = g_sin_value[100-n];\
        else if(n<= 150)\
            out = -g_sin_value[n-100];\
        else\
            out = -g_sin_value[200-n];\
        \
        out = out/x;\
    }\
}

void mWaveResample(MWave *src,MWave *dst,int src_rate,int dst_rate)
{
    int i,j;
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    
    if(src_rate <= 0) 
    {
        float frequency = mInfoGet(&(src->info),"frequency");
        if(!mIsNan(frequency)) src_rate = (int)frequency;
        mException((src_rate<=0),EXIT,"no frequency info");
    }
    
    if(dst_rate <= 0) 
    {
        float frequency = mInfoGet(&(dst->info),"frequency");
        if(!mIsNan(frequency)) dst_rate = (int)frequency;
        if(dst_rate<=0) dst_rate=src_rate;
    }
    
    if(INVALID_POINTER(dst)) dst = src;
    
    if(dst_rate==src_rate)
    {
        if(dst!=src) 
        {
            mWaveCopy(src,dst);
            dst->info = src->info;
            mInfoSet(&(dst->info),"frequency",(float)dst_rate);
        }
        return;
    }
    
    mException((dst_rate <= 0),EXIT,"invalid input");
    mException((src_rate < dst_rate),EXIT,"invalid input");
    float k = (float)src_rate/(float)dst_rate;
    if(mIsInteger(k))
    {
        mWaveIntegerResample(src,dst,(int)k);
        return;
    }
    
    int src_size = src->size;
    int dst_size = (int)(((float)src_size)/k);
    mException((dst_size <= 1),EXIT,"invalid operate");    
    mWaveRedefine(dst,src->channel,dst_size,dst->data);
    dst->info = src->info;
    mInfoSet(&(dst->info),"frequency",(float)dst_rate);
    
    for(int cn=0;cn<src->channel;cn++)
    {
        dst->data[cn][0] = src->data[cn][(int)(k+0.5f)];
        dst->data[cn][1] = src->data[cn][(int)(k+k+0.5f)];
        dst->data[cn][2] = src->data[cn][(int)(k+k+k+0.5f)];
        dst->data[cn][3] = src->data[cn][(int)(k+k+k+k+0.5f)];
        dst->data[cn][4] = src->data[cn][(int)(k+k+k+k+k+0.5f)];
        dst->data[cn][5] = src->data[cn][(int)(k+k+k+k+k+k+0.5f)];
        dst->data[cn][6] = src->data[cn][(int)(k+k+k+k+k+k+k+0.5f)];
        dst->data[cn][7] = src->data[cn][(int)(k+k+k+k+k+k+k+k+0.5f)];
        
        float l = 7*k;
        for(i=8;i<dst_size-8;i++)
        {
            l = l+k;
            int n1 = (int)l;
            int n2 = n1+1;
            
            float result = 0.0f;
            float sum = 0.0f;
            float data;
            for(j=0;j<8;j++)
            {
                SINC(((l-(float)n1)/k),data);
                result = result + src->data[cn][n1]*data;
                sum = sum + data;
                n1 = n1-1;
                
                SINC((((float)n2-l)/k),data);
                result = result + src->data[cn][n2]*data;
                sum = sum + data;
                n2 = n2+1;
            }
            dst->data[cn][i] = result/sum;
        }
        
        dst->data[cn][i]   = src->data[cn][(int)(k*i+0.5f)];
        dst->data[cn][i+1] = src->data[cn][(int)(k*(i+1)+0.5f)];
        dst->data[cn][i+2] = src->data[cn][(int)(k*(i+2)+0.5f)];
        dst->data[cn][i+3] = src->data[cn][(int)(k*(i+3)+0.5f)];
        dst->data[cn][i+4] = src->data[cn][(int)(k*(i+4)+0.5f)];
        dst->data[cn][i+5] = src->data[cn][(int)(k*(i+5)+0.5f)];
        dst->data[cn][i+6] = src->data[cn][(int)(k*(i+6)+0.5f)];
        dst->data[cn][i+7] = src->data[cn][(int)(k*(i+7)+0.5f)];
    }
}

