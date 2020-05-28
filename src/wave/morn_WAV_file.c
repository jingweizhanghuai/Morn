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

#define fread(Data,Size,Num,Fl) mException((fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error");

struct MWAVInfo {
    FILE *f;
    
    int sample_rate;
    int channel;
    int data_num;
    int data_type;
    
    MWave *wave;
};

struct MWAVHeader {
    short type;
    short channel;
    int frequency;
    int bps;
    short datanum;
    short databits;
};

void WAVInfo(struct MWAVInfo *info)
{
    struct MWAVHeader fmt;
    
    char name[5];name[4] = 0;
    
    info->sample_rate = 0;
    info->channel = 0;
    info->data_num = 0;
    
    fread(name,4,1,info->f);
    mException((memcmp(name,"RIFF",4)!=0),EXIT,"RIFF error!");
    
    int riff_size;
    fread(&riff_size,4,1,info->f);
    riff_size = riff_size + 8;

    fread(name,4,1,info->f);
    mException((memcmp(name,"WAVE",4)!=0),EXIT,"WAVE error!");
        
    int locate = ftell(info->f);
    mException((riff_size-locate<8),EXIT,"WAVE error!");
    
    while(1)
    {
        fread(name,4,1,info->f);
        int block_size; 
        fread(&block_size,4,1,info->f);
        
        locate = locate + 8;
        mException((riff_size-locate<block_size),EXIT,"file error");
        
        if(memcmp(name,"fmt ",4)==0)
        {            
            fread(&fmt,block_size,1,info->f);
            // printf("type is %d\n",fmt.type);
            // printf("channel is %d\n",fmt.channel);
            // printf("frequency is %d\n",fmt.frequency);
            // printf("bps is %d\n",fmt.bps);
            // printf("datanum is %d\n",fmt.datanum);
            // printf("databits is %d\n",fmt.databits);
            mException((fmt.channel<=0)||(fmt.channel>2),EXIT,"WAV channel error!");
            mException((fmt.datanum * fmt.frequency != fmt.bps),EXIT,"file error");
            
            info->channel = fmt.channel;
            info->sample_rate = fmt.frequency;
            info->data_type = fmt.databits;
        }
        else if(memcmp(name,"data",4)==0)
        {
            info->data_num = (block_size/fmt.datanum);
            
            return;
        }
        else
            fseek(info->f,block_size,SEEK_CUR);
        
        locate = ftell(info->f);
        if((riff_size-locate)<8)
            break;
    }
}

void WAVLoad(MWave *dst,const char *filename)
{
    mException((dst==NULL),EXIT,"invalid input");
    
    struct MWAVInfo info;
    info.f = fopen(filename,"rb");
    mException((info.f == NULL),EXIT,"file cannot open");
    WAVInfo(&info);
    
    int cn = info.channel;int data_num = info.data_num;
    mWaveRedefine(dst,cn,data_num,dst->data);
    mInfoSet(&(dst->info),"frequency",info.sample_rate);
    
    int i,n;
    if(info.data_type == 16)
    {
        short *data_16 = (short *)mMalloc(data_num*cn*sizeof(short));
        fread(data_16,data_num*cn*sizeof(short),1,info.f);
        for(i=0,n=0;i<data_num;i++,n=n+cn)for(int c=0;c<cn;c++)
            dst->data[c][i] = ((float)(data_16[n+c]))/32768.0f;
        mFree(data_16);
    }
    else if(info.data_type == 8)
    {
        char *data_8 = (char *)mMalloc(data_num*cn*sizeof(char));
        fread(data_8,data_num*cn*sizeof(char),1,info.f);
        for(i=0,n=0;i<data_num;i++,n=n+cn)for(int c=0;c<cn;c++)
            dst->data[c][i] = ((float)data_8[n+c])/128.0f;
        mFree(data_8);
    }
    else if(info.data_type == 32)
    {
        if(cn == 1) {fread(dst->data[0],(dst->size*sizeof(float)),1,info.f);}
        else
        {
            float *data_32 = (float *)mMalloc(data_num*cn*sizeof(float));
            fread(data_32,data_num*cn*sizeof(float),1,info.f);
            for(i=0,n=0;i<data_num;i++,n=n+cn)for(int c=0;c<cn;c++)
                dst->data[cn][i] = data_32[n+cn];
            mFree(data_32);
        }
    }
    else
        mException(1,EXIT,"file error");
    
    fclose(info.f);
}

void WAVSave(MWave *src,const char *filename)
{
    mException(INVALID_WAVE(src),EXIT,"invalid input");
    mException((src->channel >2),EXIT,"invalid input");
    
    FILE *f = fopen(filename,"rb");
    mException((f == NULL),EXIT,"file cannot open");

    int datasize = src->size*sizeof(short)*src->channel;
    float frequency = mInfoGet(&(src->info),"frequency");
    mException(mIsNan(frequency),EXIT,"invalid input wave sample rate");
    
    struct MWAVHeader fmt;
    fmt.type = 1;
    fmt.channel = src->channel;
    fmt.frequency = frequency;
    fmt.bps = frequency*((src->channel)<<1);
    fmt.datanum = (src->channel)<<1;
    fmt.databits = 16;
    
    int data;
    fwrite("RIFF",1,4,f);
    data = datasize+36;
    fwrite(&data,4,1,f);
    fwrite("WAVE",1,4,f);
    fwrite("fmt ",1,4,f);
    data = sizeof(struct MWAVHeader);
    fwrite(&data,4,1,f);
    fwrite(&fmt,sizeof(struct MWAVHeader),1,f);
    fwrite("data",1,4,f);
    fwrite(&datasize,4,1,f);
    
    short *buff = (short *)mMalloc(datasize);
    int i,n;
    for(i=0,n=0;i<src->size;i++,n=n+src->channel)for(int c=0;c<src->channel;c++)
    {
             if(src->data[c][i]>= 1.0f) buff[n+c] = 32767;
        else if(src->data[c][i]<=-1.0f) buff[n+c] =-32768;
        else                            buff[n+c] =(short)(src->data[c][i]*32768.0f);
    }
    fwrite(buff,datasize,1,f);
    mFree(buff);
    
    fclose(f);
}

#define HandleWAVRead MWAVInfo
#define HASH_WAVRead 0x4805f593
void endWAVRead(void *info) 
{
    struct MWAVInfo *handle = (struct MWAVInfo *)info;
    fclose(handle->f);
    if(handle->wave!=NULL) mFree(handle->wave);
}
int mWAVRead(MFile *file,MWave *dst)
{
    mException((file==NULL)||(dst==NULL),EXIT,"invalid input");
    
    MHandle *hdl=mHandle(file,WAVRead);
    struct HandleWAVRead *handle = (struct HandleWAVRead *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->f!=NULL)fclose(handle->f);
        handle->f = fopen(file->filename,"rb");
        WAVInfo(handle);
        hdl->valid = 1;
    }
    
    MWave *wave;int data_num;
    float frequency = mInfoGet(&(dst->info),"frequency");
    if(mIsNan(frequency)) {frequency=handle->sample_rate;mInfoSet(&(dst->info),"frequency",handle->sample_rate);}
    if(frequency == handle->sample_rate) 
    {
        wave = dst;
        data_num = dst->size;
    }
    else
    {
        data_num = dst->size*handle->sample_rate/frequency;
        if(handle->wave==NULL) handle->wave =mWaveCreate(handle->channel,data_num,NULL);
        else     mWaveRedefine(handle->wave,handle->channel,data_num,NULL);
        
        wave = handle->wave;
    }
    int cn = wave->channel;
    
    if(handle->data_num<data_num) return MORN_FAIL;
    
    int i,n;
    if(handle->data_type == 16)
    {
        short *data_16 = (short *)mMalloc(data_num*cn*sizeof(short));
        fread(data_16,data_num*cn*sizeof(short),1,handle->f);
        for(i=0,n=0;i<data_num;i++,n=n+cn)for(int c=0;c<cn;c++)
            wave->data[c][i] = ((float)data_16[n+c])/32768.0f;
        mFree(data_16);
    }
    else if(handle->data_type == 8)
    {
        char *data_8 = (char *)mMalloc(data_num*cn*sizeof(char));
        fread(data_8,data_num*cn*sizeof(char),1,handle->f);
        for(i=0,n=0;i<data_num;i++,n=n+cn)for(int c=0;c<cn;c++)
            wave->data[c][i] = ((float)data_8[n+c])/128.0f;
        mFree(data_8);
    }
    else if(handle->data_type == 32)
    {
        if(cn == 1) {fread(wave->data[0],(wave->size*sizeof(float)),1,handle->f);}
        else
        {
            float *data_32 = (float *)mMalloc(data_num*cn*sizeof(float));
            fread(data_32,data_num*cn*sizeof(float),1,handle->f);
            for(i=0,n=0;i<data_num;i++,n=n+cn)for(int c=0;c<cn;c++)
                wave->data[cn][i] = data_32[n+cn];
            mFree(data_32);
        }
    }
    else
        mException(1,EXIT,"file error");
    
    if(wave!=dst) mWaveResample(wave,dst,handle->sample_rate,DFLT);
    
    handle->data_num -= data_num;
    return MORN_SUCCESS;
}

#define HandleWAVWrite MWAVInfo
#define HASH_WAVWrite 0x51626730
void endWAVWrite(void *info)
{
    struct MWAVInfo *handle = (struct MWAVInfo *)info;
    
    FILE *f = handle->f;
    fseek(f,0,SEEK_SET);
    
    if(handle->data_num <= 0) {fclose(handle->f);return;}
    
    int datasize = handle->data_num*2*handle->channel;
    struct MWAVHeader fmt;
    
    fmt.type = 1;
    fmt.channel = handle->channel;
    fmt.frequency = handle->sample_rate;
    fmt.bps = handle->sample_rate*((handle->channel)<<1);
    fmt.datanum = (handle->channel)<<1;
    fmt.databits = 16;
    
    int data;
    fwrite("RIFF",1,4,f);
    data = datasize+36;
    fwrite(&data,4,1,f);
    fwrite("WAVE",1,4,f);
    fwrite("fmt ",1,4,f);
    data = sizeof(struct MWAVHeader);
    fwrite(&data,4,1,f);
    fwrite(&fmt,sizeof(struct MWAVHeader),1,f);
    fwrite("data",1,4,f);
    fwrite(&datasize,4,1,f);
    
    fclose(handle->f);
    
    if(handle->wave!=NULL) mFree(handle->wave);
}
int mWAVWrite(MObject *file,MWave *src)
{
    mException(INVALID_WAVE(src),EXIT,"invalid input");
    mException((src->channel >2),EXIT,"invalid input");
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    
    float frequency = mInfoGet(&(src->info),"frequency");
    
    MHandle *hdl=mHandle(file,WAVWrite);
    struct MWAVInfo *handle = (struct MWAVInfo *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->f = fopen(file->filename,"wb");
        mException((handle->f == NULL),EXIT,"file cannot open");
        
        mException(mIsNan(frequency),EXIT,"invalid input wave sample rate");
        handle->channel = src->channel;
        handle->sample_rate = frequency;
        handle->data_num = 0;
        handle->data_type = 16;
        
        fseek(handle->f,44,SEEK_SET);
        
        hdl->valid = 1;
    }
    
    if(src->channel<handle->channel) return MORN_FAIL;
    
    if((!mIsNan(frequency))&&(frequency!=handle->sample_rate))
    {
        if(handle->wave==NULL) handle->wave = mWaveCreate(handle->channel,DFLT,NULL);
        mWaveResample(src,handle->wave,DFLT,DFLT);
        src = handle->wave;
    }
    
    short *buff = (short *)mMalloc(src->size*src->channel*sizeof(float));
    int i,n;
    for(i=0,n=0;i<src->size;i++,n=n+src->channel)for(int c=0;c<src->channel;c++)
    {
             if(src->data[c][i]>= 1.0f) buff[n+c] = 32767;
        else if(src->data[c][i]<=-1.0f) buff[n+c] =-32768;
        else                            buff[n+c] =(short)(src->data[c][i]*32768.0f);
    }
    fwrite(buff,src->size*src->channel*sizeof(float),1,handle->f);
    mFree(buff);
    
    handle->data_num+=src->size;
    return MORN_SUCCESS;
}




#if 0

struct MWaveRWInfo {
    float *buff[MORN_MAX_WAVE_CN];
    int buff_vol;
    int buff_size; 
    
    int buff_locate;
    int wave_locate;
}MWaveRWInfo;

#define HandleWaveFrameRead MWaveRWInfo
#define HandleWaveFrameWrite MWaveRWInfo
#define HASH_WaveFrameWrite 0x94cb619c
#define HASH_WaveFrameRead  0xcbb923a7

// void beginWaveFrameRead(void *handle)
// {
    // struct MWaveRWInfo *hdl;
    // hdl = (struct MWaveRWInfo *)handle;
    // hdl->buff_size = 0;
    // hdl->buff_locate = 0;
    // hdl->wave_locate = 0;
// }
void endWaveFrameRead(void *handle)
{
    int i;
    for(i=0;i<MORN_MAX_WAVE_CN;i++)
    {
        if(((struct MWaveRWInfo *)handle)->buff[i] != NULL)
            mFree(((struct MWaveRWInfo *)handle)->buff[i]);
    }
    mFree(handle);
}

int mWaveFrameRead(MWave *src,float **read,int frame_size,float frame_overlap)
{
    int size_need;
    int size1,size2;
    int step;
    int size_overlap;
    
    float *wave_data[MORN_MAX_WAVE_CN];
    float *buff_data[MORN_MAX_WAVE_CN];
    
    int cn;
    
    float *p;
    
    mException(INVALID_WAVE(src),EXIT,"invalid input");
    // mException((frame_size>src->size),EXIT,"invalid input");
    
    if(frame_overlap < 0.0f)
        frame_overlap = 0.0f;
    
    mException((frame_overlap >=1.0f),EXIT,"invalid input");
    
    size_need = frame_size;
    size_overlap = (int)(((float)frame_size) * frame_overlap + 0.5f);
    step = frame_size- size_overlap;
    
    MHandle *hdl; ObjectHandle(src,WaveFrameRead,hdl);
    struct MWaveRWInfo *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->buff_size = 0;
        handle->buff_locate = 0;
        handle->wave_locate = 0;
        
        hdl->valid = 1;
    }

    // HandleGet(src,WaveFrameRead,handle);
    // if(handle == NULL)
    // {
        // handle = (struct MWaveRWInfo *)mMalloc(sizeof(struct MWaveRWInfo));
        // memset(handle,0,sizeof(struct MWaveRWInfo));
        // HandleSet(src,WaveFrameRead,handle);
    // }

    if(handle->wave_locate >= MAX(1,size_overlap))
    {
        handle->wave_locate = handle->wave_locate - size_overlap;
        size1 = 0;
    }
    else
    {
        size1 = MIN(size_need,handle->buff_size);
    }
    
    for(cn=0;cn<src->channel;cn++)
    {
        wave_data[cn] = src->data[cn] + handle->wave_locate;
        buff_data[cn] = handle->buff[cn]+ handle->buff_locate;
    }
    
    size_need = size_need - size1;
    size2 = 0;
    if(size_need >0)
    {
        size2 = MIN(size_need,src->size-handle->wave_locate);
        size_need = size_need - size2;
    }
    
    // printf("handle->wave_locate is %d,size1 is %d,size2 is %d,size_need is %d\n",handle->wave_locate,size1,size2,size_need);
    
    if(size_need >0)
    {
        if(size2 == 0)
        {
            handle->wave_locate = 0;
            return 1;
        }
        else if(size1>0)
        {
            if(handle->buff_locate + handle->buff_size + size2 < handle->buff_vol)
            {
                for(cn=0;cn<src->channel;cn++)
                    memcpy(buff_data[cn]+handle->buff_size,wave_data[cn],size2*sizeof(float));
            }
            else if(handle->buff_size + size2 < handle->buff_vol)
            {
                for(cn=0;cn<src->channel;cn++)
                {
                    memcpy(handle->buff[cn],buff_data[cn],handle->buff_size*sizeof(float));
                    memcpy(handle->buff[cn] + handle->buff_size,wave_data[cn],size2*sizeof(float));
                    handle->buff_locate = 0;
                }
            }
            else
            {
                handle->buff_vol = frame_size+frame_size;
                for(cn=0;cn<src->channel;cn++)
                {
                    p = (float *)mMalloc(handle->buff_vol*sizeof(float));
                    
                    memcpy(p,buff_data[cn],handle->buff_size*sizeof(float));
                    memcpy(p + handle->buff_size,wave_data[cn],size2*sizeof(float));
                    
                    mFree(handle->buff[cn]);
                    handle->buff[cn] = p;
                }
                handle->buff_locate = 0;
            }
           
            handle->wave_locate = 0;
            handle->buff_size = handle->buff_size + size2;
            return 1;
        }
        else
        {
            if(handle->buff_vol <size2)
            {
                handle->buff_vol = frame_size + frame_size;
                for(cn=0;cn<src->channel;cn++)
                {
                    if(handle->buff[cn] != NULL)
                        mFree(handle->buff[cn]);
                    handle->buff[cn] = (float *)mMalloc(handle->buff_vol*sizeof(float));
                }
            }
            
            for(cn=0;cn<src->channel;cn++)
                memcpy(handle->buff[cn],wave_data[cn],size2*sizeof(float));
            
            handle->buff_locate = 0;
            handle->wave_locate = 0;
            handle->buff_size = size2;

            return 1;
        }
    }
    else if(size1 == 0)
    {
        for(cn=0;cn<src->channel;cn++)
        {
            read[cn] = wave_data[cn];
        }
        
        handle->buff_locate = 0;
        handle->wave_locate = handle->wave_locate + frame_size;
        handle->buff_size = 0;
        
        return 0;
    }
    else
    {
        for(cn=0;cn<src->channel;cn++)
        {
            memcpy(buff_data[cn]+handle->buff_size,wave_data[cn],size2*sizeof(float));
            read[cn] = buff_data[cn];
        }
        
        handle->buff_locate = handle->buff_locate + step;
        handle->wave_locate = handle->wave_locate + size2;
        handle->buff_size = handle->buff_size + size2 - step;
        
        return 0;
    }
}

// void beginWaveFrameWrite(void *handle)
// {
    // struct MWaveRWInfo *hdl;
    // hdl = (struct MWaveRWInfo *)handle;
    // hdl->buff_size = 0;
    // hdl->buff_locate = 0;
    // hdl->wave_locate = 0;
// }
void endWaveFrameWrite(void *handle)
{
    int i;
    for(i=0;i<MORN_MAX_WAVE_CN;i++)
    {
        if(((struct MWaveRWInfo *)handle)->buff[i] != NULL)
            mFree(((struct MWaveRWInfo *)handle)->buff[i]);
    }
    mFree(handle);
}

int mWaveFrameWrite(MWave *dst,float **write,int frame_size,float frame_overlap)
{
    int size_have;
    int size1,size2;
    float *src[MORN_MAX_WAVE_CN];
    int src_size;
    
    float *wave_data[MORN_MAX_WAVE_CN];
    float *buff_data[MORN_MAX_WAVE_CN];
    
    float *p;
    
    int cn;
    
    mException(INVALID_WAVE(dst),EXIT,"invalid input");
    // mException((frame_size>dst->size),"invalid input",EXIT);
    
    if(frame_overlap < 0.0f)
        frame_overlap = 0.0f;
    
    mException((frame_overlap >=1.0f),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(dst,WaveFrameWrite,hdl);
    struct MWaveRWInfo *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->buff_size = 0;
        handle->buff_locate = 0;
        handle->wave_locate = 0;
        hdl->valid = 1;
    }
        
    // HandleGet(dst,WaveFrameWrite,handle);
    // if(handle == NULL)
    // {
        // handle = (struct MWaveRWInfo *)mMalloc(sizeof(struct MWaveRWInfo));
        // memset(handle,0,sizeof(struct MWaveRWInfo));
        // HandleSet(dst,WaveFrameWrite,handle);
    // }
    
    if(handle->buff_vol == 0)
    {
        handle->buff_vol = -1;
        src_size = frame_size;
        for(cn=0;cn<dst->channel;cn++)
            src[cn] = write[cn];
    }
    else
    {
        src_size = frame_size - (int)(((float)frame_size) * frame_overlap);
        for(cn=0;cn<dst->channel;cn++)
            src[cn] = write[cn] + (int)(((float)frame_size) * frame_overlap);
    }
    
    for(cn=0;cn<dst->channel;cn++)
    {
        wave_data[cn] = dst->data[cn] + handle->wave_locate;
        buff_data[cn] = handle->buff[cn]+ handle->buff_locate;
    }
    
    size_have = (dst->size - handle->wave_locate);
    // printf("size_have is %d,src_size is %d,handle->buff_size is %d\n",size_have,src_size,handle->buff_size);
    
    if(handle->buff_size > 0)
    {
        size1 = MIN(handle->buff_size,size_have);
        
        for(cn=0;cn<dst->channel;cn++)
        {
            memcpy(wave_data[cn],buff_data[cn],size1*sizeof(float));
            wave_data[cn] = wave_data[cn] + size1;
            buff_data[cn] = buff_data[cn] + size1;
        }
        
        handle->buff_locate = handle->buff_locate + size1;
        handle->wave_locate = handle->wave_locate + size1;
        size_have = size_have - size1;
        handle->buff_size = handle->buff_size - size1;
    }
    
    if(size_have >= src_size)
    {
        for(cn=0;cn<dst->channel;cn++)
        {
            memcpy(wave_data[cn],src[cn],src_size*sizeof(float));
        }
        handle->buff_locate = 0;
        handle->wave_locate = handle->wave_locate + src_size;
        return 0;
    }
    else
    {
        for(cn=0;cn<dst->channel;cn++)
            memcpy(wave_data[cn],src[cn],size_have*sizeof(float));
        handle->wave_locate = 0;
        size2 = (src_size-size_have);
        
        if(handle->buff_size == 0)
        {
            if(handle->buff_vol < size2)
            {
                handle->buff_vol = frame_size + frame_size;
                for(cn=0;cn<dst->channel;cn++)
                {
                    if(handle->buff[cn] != NULL)
                        mFree(handle->buff[cn]);
                    handle->buff[cn] = (float *)mMalloc(handle->buff_vol*sizeof(float));
                }
            }
            for(cn=0;cn<dst->channel;cn++)
                memcpy(handle->buff[cn],src[cn]+size_have,size2*sizeof(float));
            
            handle->buff_locate = 0;
            handle->buff_size = size2;
        }
        else
        {
            if(handle->buff_size + size2 > handle->buff_vol)
            {
                handle->buff_vol = frame_size + frame_size;
                for(cn=0;cn<dst->channel;cn++)
                {
                    p = (float *)mMalloc(handle->buff_vol*sizeof(float));
                    memcpy(p,buff_data[cn],handle->buff_size*sizeof(float));
                    memcpy(p+handle->buff_size,src[cn]+size_have,size2*sizeof(float));
                    
                    mFree(handle->buff[cn]);
                    handle->buff[cn] = p;
                }
                handle->buff_locate = 0;
            }
            else if(handle->buff_locate+size2 > handle->buff_vol)
            {
                for(cn=0;cn<dst->channel;cn++)
                {
                    memcpy(handle->buff[cn],buff_data[cn],handle->buff_size*sizeof(float));
                    memcpy(handle->buff[cn]+handle->buff_size,src[cn]+size_have,size2*sizeof(float));
                }
                handle->buff_locate = 0;
            }
            else
            {
                for(cn=0;cn<dst->channel;cn++)
                    memcpy(buff_data[cn]+handle->buff_size,src[cn]+size_have,size2*sizeof(float));
            }
            
            handle->buff_size = handle->buff_size + size2;
        }
        
        return 1;
    }
}
#endif
