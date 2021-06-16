/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_wave.h"

void mWavABSMean(MWave *src,float *mean);

static int WaveThresh(float *data,int size,float thresh)
{
    float *p;
    float sum;
    int i;
    
    thresh = thresh *128.0f;
    
    p = data;
    while(p+128 <= data+size)
    {
        sum = 0.0f;
        for(i=0;i<128;i++)
            sum = sum + p[i];
        
        if(sum > thresh)
            return 1;
        
        p = p + 128;
    }
    
    if(data+size > p+32)
    {
        p = data + (size-128);
        
        sum = 0.0f;
        for(i=0;i<128;i++)
            sum = sum + p[i];
        
        if(sum > thresh)
            return 1;
    }
    
    return 0;
}


int mWaveActive(MWave *src,float thresh)
{
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type!= MORN_WAVE_TD),EXIT,"please check input wave type");
    
    if(thresh <0) thresh = 0.05f;

    for(int i=0;i<src->channel;i++)
        if(WaveThresh(src->data[i],src->size,thresh))
            return 1;
    
    return 0;
}

void mWaveBackground(MWave *src,float *background)
{
    if(src->size <128) mWavABSMean(src,background);
    
    mException(INVALID_WAVE(src)||INVALID_POINTER(background),EXIT,"invalid input");
    int wave_type;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type!= MORN_WAVE_TD),EXIT,"please check input wave type");
    
    for(int cn=0;cn<src->channel;cn++)
    {
        background[cn] = 1.0f;
        float *p = src->data[cn];
        while(p+128<=src->data[cn] + src->size)
        {
            float mean = 0.0f;for(int i=0;i<128;i++) {mean = mean + p[i];} mean = mean/128.0f;
            background[cn] = MIN(background[cn],mean);
            p=p+128;
        }
        
        if(src->data[cn] + src->size > p+32)
        {
            p = src->data[cn] + (src->size-128);
            float mean = 0.0f;for(int i=0;i<128;i++) {mean = mean + p[i];} mean = mean/128.0f;
            
            background[cn] = MIN(background[cn],mean);
        }
    }
}

struct HandleWaveAdaptiveActive
{
    float background[MORN_MAX_WAVE_CN][32];
    int n;
};
// void beginWaveAdaptiveActive(void *handle) {memset(handle,0,sizeof(struct HandleWaveAdaptiveActive));}
void endWaveAdaptiveActive(void *handle) {mFree(handle);}
#define HASH_WaveAdaptiveActive 0xa26ad0a2
int mWaveAdaptiveActive(MWave *src,float sensibility,float thresh)
{
    float background[MORN_MAX_WAVE_CN];
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type!= MORN_WAVE_TD),EXIT,"please check input wave type");
    
    if(thresh == MORN_DEFAULT)
        thresh = 0.02f; 
    if(sensibility == MORN_DEFAULT)
        sensibility = 0.5f;
    
    MHandle *hdl=mHandle(src,WaveAdaptiveActive);
    struct HandleWaveAdaptiveActive *handle = (struct HandleWaveAdaptiveActive *)(hdl->handle);
    if(hdl->valid == 0)
    {
        memset(handle,0,sizeof(struct HandleWaveAdaptiveActive));
        hdl->valid = 1;
    }
    
    // HandleGet(src,WaveAdaptiveActive,handle);
    // if(handle == NULL)
    // {
        // handle = mMalloc(sizeof(struct HandleWaveAdaptiveActive));        
        // HandleSet(src,WaveAdaptiveActive,handle);
    // }
    
    int active = 0;
    for(int cn=0;cn<src->channel;cn++)
    {
        background[cn] = 0.0f;
        for(int i=0;i<32;i++)
            background[cn] = background[cn] + handle->background[cn][i];
        background[cn] = background[cn]*(2.5f-sensibility)/16.0f;
        
        // printf("mean is %f,background is %f\n",mean[0],background[cn]);
        
        if(WaveThresh(src->data[cn],src->size,MIN(thresh,background[cn])))
        {
            active = 1;
            break;
        }
    }
    
    mWaveBackground(src,background);    
    for(int cn=0;cn<src->channel;cn++)
        handle->background[cn][handle->n] = background[cn];
    handle->n = handle->n +1;
    if(handle->n == 16)
        handle->n = 0;
    
    return active;
}

void mWaveBurst(MWave *src,float *burst)
{
    float *p;
    int i,cn;
    float mean;
    
    if(src->size <128)
        mWavABSMean(src,burst);
    
    mException(INVALID_WAVE(src)||INVALID_POINTER(burst),EXIT,"invalid input");
    int wave_type;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type!= MORN_WAVE_TD),EXIT,"please check input wave type");
    
    for(cn=0;cn<src->channel;cn++)
    {
        burst[cn] = 0.0f;
        p = src->data[cn];        
        while(p+128<=src->data[cn] + src->size)
        {
            mean = 0.0f;
            for(i=0;i<128;i++)
                mean = mean + p[i];
            mean = mean/128.0f;
            
            burst[cn] = MAX(burst[cn],mean);
            p=p+128;
        }
        
        if(src->data[cn] + src->size > p+32)
        {
            p = src->data[cn] + (src->size-128);
            mean = 0.0f;
            for(i=0;i<128;i++)
                mean = mean + p[i];
            mean = mean/128.0f;
            
            burst[cn] = MAX(burst[cn],mean);
        }
    }
}

struct HandleWaveAdaptiveLoud
{
    float burst[MORN_MAX_WAVE_CN][32];
    int n;
};
// void beginWaveAdaptiveLoud(void *handle) {memset(handle,0,sizeof(struct HandleWaveAdaptiveLoud));}
void endWaveAdaptiveLoud(void *handle) {mFree(handle);}
#define HASH_WaveAdaptiveLoud 0xa26ad0a2
int mWaveAdaptiveLoud(MWave *src,float sensibility,float thresh)
{
    int i;
    int cn;
    int loud;
    
    float burst[MORN_MAX_WAVE_CN];
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type!= MORN_WAVE_TD),EXIT,"please check input wave type");
    
    if(thresh == MORN_DEFAULT)
        thresh = 0.02f; 
    if(sensibility == MORN_DEFAULT)
        sensibility = 0.5f;
    
    MHandle *hdl=mHandle(src,WaveAdaptiveLoud);
    struct HandleWaveAdaptiveLoud *handle = (struct HandleWaveAdaptiveLoud *)(hdl->handle);
    if(hdl->valid == 0)
    {
        memset(handle,0,sizeof(struct HandleWaveAdaptiveLoud));
        hdl->valid = 1;
    }

    // HandleGet(src,WaveAdaptiveLoud,handle);
    // if(handle == NULL)
    // {
        // handle = mMalloc(sizeof(struct HandleWaveAdaptiveLoud));        
        // HandleSet(src,WaveAdaptiveLoud,handle);
    // }
    
    loud = 0;
    for(cn=0;cn<src->channel;cn++)
    {
        burst[cn] = 0.0f;
        for(i=0;i<32;i++)
            burst[cn] = burst[cn] + handle->burst[cn][i];
        burst[cn] = burst[cn]*(1.7f-sensibility)/16.0f;
        
        // printf("mean is %f,burst is %f\n",mean[0],burst[cn]);
        
        if(WaveThresh(src->data[cn],src->size,MAX(thresh,burst[cn])))
        {
            loud = 1;
            break;
        }
    }
    
    mWaveBurst(src,burst);
    for(cn=0;cn<src->channel;cn++)
        handle->burst[cn][handle->n] = burst[cn];
    handle->n = handle->n +1;
    if(handle->n == 16)
        handle->n = 0;
    
    return loud;
}



/*
void mGetActive(MWave *src,MWave *dst[],int *dst_num)
{
    float background;
    int wav_size;
    int segment_size;
    int frame_size;
    int frame_num;
    
    float sum;
    float max;
    
    int *flag;
    int valid;
    
    float *data;
    float *mean;
    int i,j,n,cn;
    
    mException(((int)src<=0),"invalid input",EXIT);
    mException((src->info.wave_type != MORN_WAVE_TD),"please check input wave type",NULL);
    
    wav_size = src->size;    
    segment_size = dst[0]->size;
    if(wav_size<segment_size*2)
    {
        *dst_num = 0;
        mException(1,"invalid operate",{return;});
    }
    
    frame_size = segment_size/16;
    frame_num = wav_size/frame_size;
    // printf("frame_size is %d\n",frame_size);
    // printf("frame_num is %d\n",frame_num);
    
    flag = (int *)mMalloc(frame_num * sizeof(int));
    flag[0] = 0;
    flag[1] = 0;
    flag[2] = 0;
    flag[3] = 0;
    for(j=4;j<frame_num-4;j++)
        flag[j] = 1;
    flag[j] = 0;
    flag[j+1] = 0;
    flag[j+2] = 0;
    flag[j+3] = 0;
    
    mean = (float *)mMalloc(frame_num*sizeof(float));
    
    // printf("aaaaaaaaaaa %d\n",src->channel);
    for(cn=0;cn<src->channel;cn++)
    {
        data = src->data[cn];
        background = 10000.0;
        max = 0.0;
        for(j=0;j<frame_num;j++)
        {
            sum = 0.0;
            for(i=0;i<frame_size;i++)
                sum = (data[i]>0)?(sum+data[i]):(sum-data[i]);
            mean[j] = sum/((float)frame_size);
            
            // printf("time is %f: mean is %f\n",j*0.016,mean[j]);
            
            background = (mean[j]<background)?mean[j]:background;
            max = (mean[j]>max)?mean[j]:max;
            
            data = data +frame_size;
        }
        
        background = background*2.0;
        // printf("background is %f\n",background);
        // printf("max is %f\n",max);
        if(background < 0.1*max)
            background = 0.1*max;
        if(background>0.03)
            background = 0.03;
        // printf("background is %f\n",background);
        
 
        for(j=4;j<frame_num-4;j++)
        {
            if(flag[j] == 1)
            {
                
                if(mean[j] < background)
                    flag[j] = 0;
                else
                {
                    sum = mean[j-4] + mean[j-3] + mean[j-2] + mean[j-1];
                    if(mean[j] < (sum/8.0))
                    {
                        flag[j] = 0;
                        continue;
                    }
                    sum = mean[j+1] + mean[j+2] + mean[j+3] + mean[j+4];
                    if(mean[j] < (sum/8.0))
                    {
                        flag[j] = 0;
                        continue;
                    }
                }
            }
            // printf("time is %f: flag[%d] is %d\n",j*0.016,j,flag[j]);
        }
    }
    
    valid = flag[4] + flag[5] + flag[6] + flag[7] + flag[8] + flag[9] + flag[10] + flag[11]
          + flag[12] + flag[13] + flag[14] + flag[15] + flag[16] + flag[17] + flag[18] + flag[19];
    n = 0;
    for(j=4;j<frame_num-20;)
    {
        
        if(valid == 16)
        {
            
            for(cn=0;cn<src->channel;cn++)
                memcpy(dst[n]->data[cn],(src->data[cn] + j*frame_size),segment_size*sizeof(float));
            // printf("time is %f: j is %d,valid is %d,n is %d\n",j*0.016,j,valid,n);

            n = n+1;
            if((*dst>0)&&(n==*dst_num))
                break;
            
            valid = 8;
            for(i=j+16;i<j+24;i++)
                valid = valid + flag[i]; 

            j = j + 8;
        }
        else
        {
            valid = valid - flag[j] + flag[j+16];
            j = j+1;
        }
    }
    
    *dst_num = n;
    
    mFree(flag);
    mFree(mean);   
}
*/

void mWaveGetActive(MWave *src,int min_size,MWave *dst[],int *dst_num)
{
    float background;
    int wav_size;
    int frame_size;
    int frame_num;
    
    float sum;
    float frame_max,frame_min,frame_mean;
    
    int thresh_num;
    
    int *flag;
    int valid;
    
    float *data;
    float *mean;
    int i,j,n,cn;
    
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    int wave_type;mPropertyRead(src,"wave_type",&wave_type);
    mException((wave_type!= MORN_WAVE_TD),EXIT,"please check input wave type");
    
    wav_size = src->size;
    if(min_size <=0)
        min_size = 2048;    
    frame_size = min_size/16;
    if(frame_size > 128)
        frame_size = 128;
    
    thresh_num = (min_size/frame_size);
    
    frame_num = wav_size/frame_size;
    
    flag = (int *)mMalloc(frame_num * sizeof(int));
    flag[0] = 0;
    flag[1] = 0;
    flag[2] = 0;
    flag[3] = 0;
    for(j=4;j<frame_num-4;j++)
        flag[j] = 1;
    flag[j] = 0;
    flag[j+1] = 0;
    flag[j+2] = 0;
    flag[j+3] = 0;
    
    mean = (float *)mMalloc(frame_num*sizeof(float));
    
    for(cn=0;cn<src->channel;cn++)
    {
        data = src->data[cn];
        frame_min = 1.0f;
        frame_max = 0.0f;
        frame_mean = 0.0f;
        for(j=0;j<frame_num;j++)
        {
            sum = 0.0f;
            for(i=0;i<frame_size;i++)
                sum = (data[i]>0)?(sum+data[i]):(sum-data[i]);
            mean[j] = sum/((float)frame_size);
            
            // printf("time is %f: mean is %f\n",j*0.016,mean[j]);
            
            frame_min = (mean[j]<frame_min)?mean[j]:frame_min;
            frame_max = (mean[j]>frame_max)?mean[j]:frame_max;
            frame_mean = frame_mean + mean[j];
            
            data = data +frame_size;
        }
        frame_mean = frame_mean/(float)frame_num;
        
        
        // printf("frame_max is %f\n",frame_max);
        // printf("frame_min is %f\n",frame_min);
        // printf("frame_mean is %f\n",frame_mean);
        
        background = frame_min*2.0f;
        if(background < 0.05f*frame_max)
            background = 0.05f*frame_max;
        if(background < 0.1f*frame_mean)
            background = 0.1f*frame_mean;
        if(background>0.02f)
            background = 0.02f;
        if(background<0.008f)
            background = 0.008f;
        // printf("background is %f\n",background);
        
        for(j=4;j<frame_num-4;j++)
        {
            if(flag[j] == 1)
            {
                if(mean[j] < background)
                    flag[j] = 0;
                else
                {
                    sum = mean[j-4] + mean[j-3] + mean[j-2] + mean[j-1];
                    if(mean[j] < (sum/12.0f))
                    {
                        flag[j] = 0;
                        continue;
                    }
                    sum = mean[j+1] + mean[j+2] + mean[j+3] + mean[j+4];
                    if(mean[j] < (sum/12.0f))
                    {
                        flag[j] = 0;
                        continue;
                    }
                }
            }
            // printf("time is %f: flag[%d] is %d\n",j*0.016,j,flag[j]);
        }
    }
        
    valid = 0;
    n = 0;
    for(j=4;j<frame_num-4;j=j+1)
    {
        if(flag[j] == 1)
            valid = valid + 1;
        else
        {
            if(valid > thresh_num)
            {
                // printf("%d: time is %f,long is %f\n",n,(j-valid)*0.016,valid*0.016);
                
                mWaveRedefine(dst[n],src->channel,valid * frame_size,dst[n]->data);
                for(cn=0;cn<src->channel;cn++)
                    memcpy(dst[n]->data[cn],(src->data[cn] + (j-valid)*frame_size),dst[n]->size*sizeof(float));
                
                // dst[n]->info = src->info;
                n = n+1;
                if(n+1 == *dst_num)
                    break;
                
            }
            valid = 0;
        }
    }
    if(valid > thresh_num)
    {
        mWaveRedefine(dst[n],src->channel,valid * frame_size,dst[n]->data);
        for(cn=0;cn<src->channel;cn++)
            memcpy(dst[n]->data[cn],(src->data[cn] + (j-valid)*frame_size),dst[n]->size*sizeof(float));
        
        // dst[n]->info = src->info;
        n = n+1;
    }

    *dst_num = n;
    
    mFree(flag);
    mFree(mean);   
}
