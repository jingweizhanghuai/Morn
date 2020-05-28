/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "morn_math.h"

__thread void *morn_test=NULL;

#ifdef _MSC_VER
__thread int morn_clock_n = -1;
__thread int morn_clock_begin[16];
__thread int morn_clock_end[16];
#define stricmp _stricmp
#else
__thread int morn_timer_n = -1;
__thread struct timeval morn_timer_begin[16];
__thread struct timeval morn_timer_end[16];
#define stricmp strcasecmp
#endif

__thread char morn_filename[256];

int morn_rand_seed = -1;
int mRand(int floor,int ceiling)
{
    if(morn_rand_seed == -1)
    {
        morn_rand_seed = time(NULL)/60;
        srand(morn_rand_seed);
    }
    if((floor==DFLT)&&( ceiling==DFLT)) {return rand();}
    if(floor==ceiling) return floor;
    int d = ceiling-floor;
    if(d>RAND_MAX)
        return (((rand()<<15)+rand())%d)+floor;
    return (rand()%d)+floor;
}

float mNormalRand(float mean,float delta)
{
    float u = mRand(1,32768)/32768.0f;
    float v = mRand(0,32767)/32767.0f;
    
    float out = sqrt(0.0-2.0*log(u))*cos(2*3.14159265358979f*v);
    return (out*delta+mean);
}

int mCompare(const void *mem1,int size1,const void *mem2,int size2)
{
    if((size1<0)&&(size2<0)) return strcmp((char *)mem1,(char *)mem2);
    if(size1<0) size1 = strlen((char *)mem1);
    if(size2<0) size2 = strlen((char *)mem2);
    int flag = memcmp(mem1,mem2,MIN(size1,size2));
    return (flag!=0)?flag:(size1-size2);
}

float mInfoGet(MInfo *info,const char *name)
{
    for(int i=0;i<8;i++)
        if(stricmp(&(info->name[i][0]),name)==0)
            return info->value[i];
      
    return mNan();
}

void mInfoSet(MInfo *info,const char *name,float value)
{
    for(int i=0;i<8;i++)
        if(stricmp(&(info->name[i][0]),name)==0)
        {
            info->value[i] = value;
            return;
        }
        
    for(int i=0;i<8;i++)
        if(info->name[i][0] == 0)
        {
            strcpy(&(info->name[i][0]),name);
            info->value[i] = value;
            return;
        }
    mException(1,EXIT,"no enough space for %s\n",name);
}

struct HandleObjectCreate
{
    MObject *object;
};
void endObjectCreate(void *info)
{
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)info;
    mException((handle->object == NULL),EXIT,"invalid object");
    mFree(handle->object);
}
#define HASH_ObjectCreate 0xbd1d8878
MObject *mObjectCreate(const void *obj)
{
    MObject *object = (MObject *)mMalloc(sizeof(MObject));
    object->handle = NULL;

    object->handle = mHandleCreate();
    MHandle *hdl=mHandle(object,ObjectCreate);
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(hdl->handle);
    handle->object = object;
    
    object->object = (void *)obj;
    return object;
}
 
void mObjectRelease(MObject *object)
{
    if(!INVALID_POINTER(object->handle))
        mHandleRelease(object->handle);
}

void mObjectRedefine(MObject *object,const void *obj)
{
    if(object->object != obj)
    {
        object->object = (void *)obj;
        mHandleReset(object->handle);
    }
}
 
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
int morn_log_level = MORN_LOG_INFO;
FILE *morn_log_f = NULL;
int *morn_log_count;
struct HandleLogSet
{
    FILE *f;
    int count;
};
void endLogSet(void *info)
{
    struct HandleLogSet *handle = (struct HandleLogSet *)info;
    if(morn_log_f==handle->f)
    {
        morn_log_f = NULL;
        morn_log_count = 0;
        morn_log_level = MORN_LOG_INFO;
    }
    if(handle->f != NULL)
        fclose(handle->f);
}
#define HASH_LogSet 0xda00cd5b
void mLogSet(MFile *file,int level)
{
    MHandle *hdl=mHandle(file,LogSet);
    struct HandleLogSet *handle = (struct HandleLogSet *)(hdl->handle);
    level = MAX(level,MORN_LOG_INFO);
    morn_log_level = level;
    if(hdl->valid!=0)
    {
        mException((morn_log_f!=handle->f)||(morn_log_count!=&(handle->count)),EXIT,"invalid set");
        return;
    }
    hdl->valid = 1;
    if(morn_log_f==NULL)
    {
         handle->f = fopen(file->filename,"w");morn_log_f = handle->f;
         handle->count = 0; morn_log_count = &(handle->count);
    }
}

MList *mHandleCreate(void)
{
    MList *handle = (MList *)mMalloc(sizeof(MList));
    handle->num = 0;
    handle->data = NULL;
   
    return handle;
}

void mHandleRelease(MList *handle)
{
    if(INVALID_POINTER(handle)) return;
    
    MHandle *hdl;
    for(int i=1;i<handle->num;i++)
    {
        hdl = (MHandle *)(handle->data[i]);
        
        (hdl->destruct)(hdl->handle);
        mFree(hdl->handle);
        mFree(hdl);
    }
    hdl = (MHandle *)(handle->data[0]);
    (hdl->destruct)(hdl->handle);
    mFree(hdl->handle);
    mFree(hdl);
    
    mFree(handle->data);
    mFree(handle);
}

void mHandleReset(MList *handle) 
{
    if(INVALID_POINTER(handle)) return;
    for(int i=1;i<handle->num;i++)
    {
        MHandle *hdl = (MHandle *)(handle->data[i]);
        hdl->valid = 0;
    }
}

MHandle *GetHandle(MList *handle,int size,unsigned int hash,void (*end)(void *))
{
    mException((handle==NULL)||(size<=0)||(end==NULL),EXIT,"invalid input");
    int num = handle->num;
    for(int i=0;i<num;i++)
    {
        MHandle *handle_data = (MHandle *)(handle->data[i]);
        if(handle_data->flag == hash) return handle_data;
    }
    
    MHandle *Handle_context = (MHandle *)mMalloc(sizeof(MHandle));
    Handle_context->flag    = hash;
    Handle_context->valid   = 0;
    Handle_context->handle  =mMalloc(size);memset(Handle_context->handle,0,size);
    Handle_context->destruct= end;
    if(num%16 == 0)
    {
        void **handle_buff = (void **)mMalloc((num+16)*sizeof(void *));
        if(num>0)
        {
            memcpy(handle_buff,handle->data,num*sizeof(void *));
            mFree(handle->data);
        }
        handle->data = handle_buff;
    }
    handle->data[num] = Handle_context;
    handle->num = num+1;
    
    return (MHandle *)(handle->data[num]);
}
