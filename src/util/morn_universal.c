#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "morn_Math.h"
 
#ifdef __GNUC__
int morn_timer_n = -1;
struct timeval morn_timer_begin[16];
struct timeval morn_timer_end[16];
// float mTimerUse()
// {
    // float use = (morn_timer_end.tv_sec - morn_timer_begin.tv_sec)*1000.0f + (morn_timer_end.tv_usec - morn_timer_begin.tv_usec)/1000.0f;
    // mException((use<0.0f),EXIT,"invalid timer");
    // return use;
// }
#else
int morn_clock_n = -1;
int morn_clock_begin[16];
int morn_clock_end[16];
// float mTimerUse()
// {
    // return ((float)(morn_clock_end-morn_clock_begin))/((float)CLOCKS_PER_SEC);
// }
#endif

int morn_rand_seed = -1;
int mRand(int floor,int ceiling)
{
    // if(morn_rand_seed == -1)
    // {
        // morn_rand_seed = time(NULL)/60;
        // srand(morn_rand_seed);
    // }
    if((floor==DFLT)&&( ceiling==DFLT)) {return rand();}
    int d = ceiling-floor;
    if(d>RAND_MAX)
        return (((rand()<<15)+rand())%d)+floor;
    return (rand()%d)+floor;
}

int mCompare(const void *mem1,int size1,const void *mem2,int size2)
{
    if((size1<0)&&(size2<0)) return strcmp(mem1,mem2);
    if(size1<0) size1 = strlen(mem1);
    if(size2<0) size2 = strlen(mem2);
    int flag = memcmp(mem1,mem2,MIN(size1,size2));
    return (flag!=0)?flag:(size1-size2);
}


#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

float mInfoGet(MInfo *info,const char *name)
{
    for(int i=0;i<8;i++)
        if(strcasecmp(&(info->name[i][0]),name)==0)
            return info->value[i];
       
    return mNan();
}

void mInfoSet(MInfo *info,const char *name,float value)
{
    for(int i=0;i<8;i++)
        if(strcasecmp(&(info->name[i][0]),name)==0)
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
    struct HandleObjectCreate *handle = info;
    mException((handle->object == NULL),EXIT,"invalid object");
    mFree(handle->object);
}
#define HASH_ObjectCreate 0xbd1d8878
MObject *mObjectCreate(const void *obj)
{
    MObject *object = mMalloc(sizeof(MObject));
    object->handle = NULL;
    
    MHandle *hdl; ObjectHandle(object,ObjectCreate,hdl);
    struct HandleObjectCreate *handle = hdl->handle;
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
    struct HandleLogSet *handle = info;
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
    MHandle *hdl; ObjectHandle(file,LogSet,hdl);
    struct HandleLogSet *handle = hdl->handle;
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
         handle->f = fopen(file->object,"w");morn_log_f = handle->f;
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

MHandle *mHandleAppend(MList *handle,int hash,int size,void (*func)(void *))
{
    MHandle *hdl = (MHandle *)mMalloc(sizeof(MHandle));
    hdl->flag = hash;
    hdl->valid = 0;
    hdl->handle = mMalloc(size);
    hdl->destruct = func;
    
    memset(hdl->handle,0,size);
    
    int num = handle->num;
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
    handle->data[num] = hdl;
    handle->num = num+1;
    
    return (MHandle *)(handle->data[num]);
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