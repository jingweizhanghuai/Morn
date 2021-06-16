/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_wave.h"

typedef struct TriangleFilter
{
    int l;
    int h;
}TriangleFilter;

#define TRAINGLE_PARA(low,med,high,para) {\
    if(low==med)\
        para.l = med-1;\
    else\
        para.l = low;\
    \
    if(med==high)\
        para.h = med+1;\
    else\
        para.h = high;\
}

#define USE_FILTER(wave,filter,feature) {\
    int n1,n2;\
    int k;\
    float sum;\
    n1 = filter.l;\
    n2 = filter.h-1;\
    sum = 0;\
    for(k=1;n2>n1;k++)\
    {\
        sum = sum + (wave->data[cn][n1] + wave->data[cn][n2])*k;\
        n1 = n1+1;\
        n2 = n2-1;\
    }\
    if(n1 == n2)\
    {\
        sum = sum + wave->data[cn][n1]*k;\
        feature = sum/((float)(k*k));\
    }\
    else\
    {\
        feature = sum/((float)(k*(k-1)));\
    }\
}

static TriangleFilter g_uniform_para[128];
static int g_uniform_low_frequency = 0;
static int g_uniform_high_frequency = 0;
static int g_uniform_num = 0;

void GetUniformPara()
{
    int i;
    int frq[130];
    float frequency;
    float step;
    
    step = ((float)(g_uniform_high_frequency - g_uniform_low_frequency))/((float)(g_uniform_num+1));
    frequency = (float)g_uniform_low_frequency;
    
    frq[0] = g_uniform_low_frequency;
    for(i=1;i<=g_uniform_num;i++)
    {
        frequency = frequency + step;
        frq[i] = (int)(frequency + 0.5f);
    }
    frq[i] = g_uniform_high_frequency;
    
    for(i=0;i<g_uniform_num;i++)
        TRAINGLE_PARA(frq[i],frq[i+1],frq[i+2],g_uniform_para[i]);
}

void mUniformFilterBank(MWave *src, int Start, int End, int NumUniformFilter, float** UniformFilterFeature)
{
    int i;
    int cn;
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type = -1;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"please check input wave type");
    mException((INVALID_POINTER(UniformFilterFeature)),EXIT,"invalid input");
    if(Start == MORN_DEFAULT)
        Start = 0;
    mException((Start < 0),EXIT,"invalid input");
    if(End == MORN_DEFAULT)
        End = src->size;
    mException((End > src->size),EXIT,"invalid input");
    mException((NumUniformFilter > 128)||(NumUniformFilter <0),EXIT,"invalid operate");    
    
    if((g_uniform_low_frequency != Start)||(g_uniform_high_frequency != End)||(g_uniform_num != NumUniformFilter))
    {
        g_uniform_low_frequency = Start;
        g_uniform_high_frequency = End;
        g_uniform_num = NumUniformFilter;
        GetUniformPara();
    }
    
    for(cn = 0;cn<src->channel;cn++)
        for(i=0;i<NumUniformFilter;i++)
            USE_FILTER(src,g_uniform_para[i],UniformFilterFeature[cn][i]);
}

static TriangleFilter g_mel_para[128];
static int g_mel_low_frequency = 0;
static int g_mel_high_frequency = 0;
static int g_mel_num = 0;

#define HZ2MEL(hz) (float)(1127.0f*log(1.0f + ((double)(hz))/700.0))
#define MEL2HZ(mel) (float)(700.0f*exp(((double)(mel))/1127.0) - 700.0f)

void GetMelPara(int sample_rate,int wave_size)
{
    int i;
    float h2w_low,h2w_high,h2w;
    float hz;
    int mel[130];
        
    h2w_low=HZ2MEL(g_mel_low_frequency*((float)sample_rate)/((float)wave_size));
    h2w_high=HZ2MEL(g_mel_high_frequency*((float)sample_rate)/((float)wave_size));
    h2w=(h2w_high-h2w_low)/(float)(g_mel_num+1);
    
    mel[0] = g_mel_low_frequency;
    hz = h2w_low;
    for(i=1;i<=g_mel_num;i++)
    {
        hz = hz + h2w;
        mel[i] = (int)(MEL2HZ(hz)*((float)wave_size)/((float)sample_rate) + 0.5f);
        // printf("hz is %f,mel[i] is %d\n",hz,mel[i]);
    }
    mel[i] = g_mel_high_frequency;
    
    for(i=0;i<g_mel_num;i++)
    {
        TRAINGLE_PARA(mel[i],mel[i+1],mel[i+2],g_mel_para[i]);
        // printf("%d---- %d,%d\n",i,g_mel_para[i].l,g_mel_para[i].h);
    }
}

void mWaveMelFilterBank(MWave *src, int Start, int End, int NumMelFilter, float** MelFilterFeature)
{
    int i;
    int cn;
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type = -1;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"please check input wave type");
    mException((INVALID_POINTER(MelFilterFeature)),EXIT,"invalid input");
    float frequency=DFLT; mException(mPropertyRead(src,"frequency",&frequency)==NULL,EXIT,"invalid input");
    if(Start== DFLT) Start= 0;        mException((Start < 0      ),EXIT,"invalid input");
    if(End  == DFLT) End  = src->size;mException((End > src->size),EXIT,"invalid input");
    
    mException((NumMelFilter > 128)||(NumMelFilter <0),EXIT,"invalid operate");    
    
    if((g_mel_low_frequency != Start)||(g_mel_high_frequency != End)||(g_mel_num != NumMelFilter))
    {
        g_mel_low_frequency = Start;
        g_mel_high_frequency = End;
        g_mel_num = NumMelFilter;
        GetMelPara((int)frequency,src->size);
    }
    
    for(cn = 0;cn<src->channel;cn++)
        for(i=0;i<NumMelFilter;i++)
            USE_FILTER(src,g_mel_para[i],MelFilterFeature[cn][i]);
}

void mWaveCentroid(MWave *src,float *centroid)
{
    int i;
    int cn;
    float sum1,sum2;
    
    mException(((INVALID_WAVE(src))||(INVALID_POINTER(centroid))),EXIT,"invalid input");
    int wave_type = -1;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"please check input wave type");
    
    for(cn=0;cn<src->channel;cn++)
    {
        sum1 = 0.0f;
        sum2 = 0.0f;
        for(i=0;i<src->size;i++)
        {
            sum1 = sum1 + src->data[cn][i];
            sum2 = sum2 + ((float)i)*(src->data[cn][i]);
        }
        centroid[cn] = sum2/sum1;
    }
}

void mWaveBandwidth(MWave *src,float *centroid,float *bandwise)
{
    
    int i;
    int cn;
    float sum1,sum2;
    float centr;
    
    mException(((INVALID_WAVE(src))||(INVALID_POINTER(bandwise))),EXIT,"invalid input");
    int wave_type = -1;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"please check input wave type");
    
    for(cn=0;cn<src->channel;cn++)
    {
        sum1 = 0.0f;
        sum2 = 0.0f;
        for(i=0;i<src->size;i++)
        {
            sum1 = sum1 + src->data[cn][i];
            sum2 = sum2 + ((float)i)*(src->data[cn][i]);
        }
        centr = sum2/sum1;
        
        if(!INVALID_POINTER(centroid))
            centroid[cn] = centr;
    
        sum2 = 0.0f;
        for(i=0;i<src->size;i++)
            sum2 = sum2 + (i-centr)*(i-centr)*(src->data[cn][i]);

        bandwise[cn] = sum2/sum1;
    }
}

void mWaveSubbandDistribution(MWave *src,int *start,int *end,int sub_num,float *distribution[])
{
    int i,j;
    int cn;
    
    float sum1,sum2;
    
    mException(((INVALID_WAVE(src))||(INVALID_POINTER(distribution))),EXIT,"invalid input");
    int wave_type = -1;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type != MORN_WAVE_PS),EXIT,"please check input wave type");
    
    if(sub_num <0)
        sub_num = 1;
    
    for(cn=0;cn<src->channel;cn++)
    {
        float normalize_value=DFLT; mPropertyRead(src,"normalize",&normalize_value);
        if((normalize_value>0)&&(normalize_value != MORN_NOT_NORMALIZED))
            sum1 = normalize_value;
        else
        {
            sum1 = 0.0f;
            for(i=0;i<src->size;i++)
                sum1 = sum1 + src->data[cn][i];
        }
        
        for(j=0;j<sub_num;j++)
        {
            sum2 = 0.0;
            for(i=start[j];i<=end[j];i++)
                sum2 = sum2 + src->data[cn][i];
            
            distribution[cn][j] = sum2/sum1;
        }
    }
}
