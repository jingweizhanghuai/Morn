/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "morn_math.h"

#ifdef __GNUC__
#define stricmp strcasecmp
#endif

__thread void *morn_test=NULL;

__thread char morn_filename[256];

__thread int morn_data_type;
__thread char morn_data_buff[8];

void mNULL(int *p) {NULL;}

static __thread int morn_thread_ID = -1;
static int morn_thread_count = 0;
static pthread_mutex_t morn_thread_ID_mutex = PTHREAD_MUTEX_INITIALIZER;
int mThreadID()
{
    if(morn_thread_ID==-1)
    {
        pthread_mutex_lock(&morn_thread_ID_mutex);
        morn_thread_count +=1;
        morn_thread_ID = morn_thread_count;
        pthread_mutex_unlock(&morn_thread_ID_mutex);
    }
    return morn_thread_ID;
}

static int morn_rand_seed = -1;
int m_Rand(int floor,int ceiling)
{
    // #ifndef _DEBUG
    // if(morn_rand_seed == -1)
    // {
    //     morn_rand_seed = time(NULL);
    //     srand(morn_rand_seed);
    // }
    // #endif
    
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

char *morn_string="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int mRandString(char *str,int l1,int l2)
{
    int size;if(l1<=0) {size=l2-1;}else if(l2<=0) {size=l1;} else if(l1==l2) {size=l1;} else {size=mRand(l1,l2);}
    mException(size<=0,EXIT,"invalid input");
    for(int i=0;i<size;i++) str[i]=morn_string[mRand(0,62)];
    str[size]=0;
    return size;
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

struct HandleFileCreate
{
    char filename[256];
};
void endFileCreate(void *info) {NULL;}
#define HASH_FileCreate 0xfdab2bff
MFile *mFileCreate(const char *filename,...)
{
    MFile *file = mObjectCreate(NULL);
    MHandle *hdl = mHandle(file,FileCreate);
    hdl->valid=1;
    struct HandleFileCreate *handle = hdl->handle;
    file->filename = handle->filename;
    
    va_list namepara;
    va_start (namepara,filename);
    vsnprintf(file->filename,256,filename,namepara);
    va_end(namepara);
    return file;
}

void mFileRedefine(MFile *file,char *filename,...)
{
    if(strcmp(filename,file->filename)!=0)
    {
        va_list namepara;
        va_start (namepara,filename);
        vsnprintf(file->filename,256,filename,namepara);
        va_end(namepara);
        mHandleReset(file->handle);
    }
}

int fsize(FILE *f)
{
    mException(f==NULL,EXIT,"invalid file");
    int l=ftell(f);     fseek(f,0,SEEK_END);
    int size=ftell(f)-l;fseek(f,l,SEEK_SET);
    return size;
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
    printf("tttttttttttttttttttt\n");
    if(INVALID_POINTER(handle)) return;
    for(int i=1;i<handle->num;i++)
    {
        MHandle *hdl = (MHandle *)(handle->data[i]);
        hdl->valid = 0;
    }
}

static pthread_mutex_t handle_mutex = PTHREAD_MUTEX_INITIALIZER;
MHandle *GetHandle(MList *handle,int size,unsigned int hash,void (*end)(void *))
{
    mException((handle==NULL)||(size<=0)||(end==NULL),EXIT,"invalid input");
    pthread_mutex_lock(&handle_mutex);
    int num = handle->num;
    for(int i=0;i<num;i++)
    {
        MHandle *handle_data = (MHandle *)(handle->data[i]);
        if(handle_data->flag == hash) {pthread_mutex_unlock(&handle_mutex);return handle_data;}
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
    pthread_mutex_unlock(&handle_mutex);
    return (MHandle *)(handle->data[num]);
}

__thread MObject *morn_object=NULL;
MChain *morn_object_map;
#ifdef _MSC_VER
void morn_end(void)
{
    // printf("endendendendendendendendendendend\n");
    if(morn_object!=NULL) mObjectRelease(morn_object);
    morn_object=NULL;
    
    if(morn_object_map!=NULL)
    {
        if(morn_object_map->chainnode!=NULL)
        {
            MChainNode *node = morn_object_map->chainnode->next;
            while(node!=morn_object_map->chainnode){mObjectRelease(*(MObject **)mMapNodeValue(node));node=node->next;}
        }
        mChainRelease(morn_object_map);
    }
    morn_object_map = NULL;
}
void morn_begin()
{
    morn_object=mObjectCreate(NULL);
    morn_object_map=mChainCreate();
    atexit(morn_end);
}
#pragma section(".CRT$XCU",read)
__declspec(allocate(".CRT$XCU")) void (* mornbegin)() = morn_begin;

#else
__attribute__((constructor)) void morn_begin() {
    morn_object=mObjectCreate(NULL);
    morn_object_map=mChainCreate();
    // printf("before main\n");
} 

__attribute__((destructor)) void morn_end() {
    if(morn_object!=NULL) mObjectRelease(morn_object);
    morn_object=NULL;
    
    if(morn_object_map!=NULL)
    {
        if(morn_object_map->chainnode!=NULL)
        {
            MChainNode *node = morn_object_map->chainnode->next;
            while(node!=morn_object_map->chainnode)
            {
                MObject *obj = *(MObject **)mMapNodeValue(node);
                mObjectRelease(obj);
                node=node->next;
            }
        }
        mChainRelease(morn_object_map);
    }
    // printf("endendendendendendendendendendend\n");
    morn_object_map = NULL;
}
#endif

static int morn_object_count = 0;
MObject *mMornObject(void *p,int size)
{
    if(p==NULL) 
    {
        if(morn_object==NULL) morn_object=mObjectCreate(NULL);
        return morn_object;
    }
    
    if(size<0) size=strlen((char *)p);
    MObject **pobj = mMapRead(morn_object_map,&p,sizeof(void *),NULL,DFLT);
    
    int *psize = (int *)(pobj+1);
    void *pdata = (void *)(psize+1);
    if(pobj != NULL)
    {
        if(size==*psize) if(memcmp(pdata,p,size)==0) return (*pobj);
        mObjectRelease(pobj[0]);
        mMapNodeDelete(morn_object_map,&p,sizeof(void *));
    }
    
    morn_object_count+=1;
    if(morn_object_count>=512)
    {
        MChainNode *node = morn_object_map->chainnode->next;
        while(node!=morn_object_map->chainnode)
        {
            pobj = mMapNodeValue(node);psize=(int *)(pobj+1);pdata=(void *)(psize+1);
            node = node->next;
            if(memcmp(pobj[0]->object,pdata,*psize)!=0)
            {
                mObjectRelease(pobj[0]);
                mMapNodeDelete(morn_object_map,mMapNodeKey(node->last),sizeof(void *));
            }
        }
        morn_object_count = 0;
    }

    MObject *obj = mObjectCreate(p);
    pobj=mMapWrite(morn_object_map,&p,sizeof(void *),NULL,(sizeof(MObject *)+sizeof(int)+size));
    psize=(int *)(pobj+1);pdata=(void *)(psize+1);
    *pobj=obj;*psize=size;memcpy(pdata,p,size);
    
    return obj;
}


void _CNum(double data,char *out,int *flag)
{
    #if defined(__linux__)
    int s=3;
    int dic[17]={0x00b69be9,0x0080b8e4,0x00a4b8e4,0x0089b8e4,0x009b9be5,0x0094bae4,0x00ad85e5,0x0083b8e4,0x00ab85e5,0x009db9e4,0x00818de5,0x00be99e7,0x00838de5,0x0087b8e4,0x00bfbae4,0x009fb4e8,0x00b982e7};
    #elif defined(_WIN64)||defined(_WIN32)
    int s=2;
    int dic[17]={0x0000e3c1,0x0000bbd2,0x0000feb6,0x0000fdc8,0x0000c4cb,0x0000e5ce,0x0000f9c1,0x0000dfc6,0x0000cbb0,0x0000c5be,0x0000aeca,0x0000d9b0,0x0000a7c7,0x0000f2cd,0x0000dad2,0x0000bab8,0x0000e3b5};
    #else
    mException(1,EXIT,"invalid operate system");
    #endif

    if(data<0) {memcpy(out,dic+15,s);out+=s;data=0.0-data;}
    int64_t value = (int64_t)data;mException(ABS(value-data)>1,EXIT,"input data too large");
    double value2 = data-(double)value;
    if(value<=10) {memcpy(out,dic+value,s);out+=s;goto cnum_next;}
    if((value<20)&&(*flag==0)) {memcpy(out,dic+10,s);memcpy(out+s,dic+(value-10),s);out+=s+s;goto cnum_next;}

    if(value>100000000)
    {
        _CNum((double)(value/100000000),out,flag);
        out+=strlen(out);
        memcpy(out,dic+14,s);
        out=out+s;
        *flag = 1;
        value = value%100000000;
    }

    if(value>10000)
    {
        _CNum((double)(value/10000),out,flag);
        out+=strlen(out);
        memcpy(out,dic+13,s);
        out=out+s;
        *flag=1;
        value = value%10000;
    }
    
    #if defined(__linux__)
    dic[2]=0x008cbae4;
    #elif defined(_WIN64)||defined(_WIN32)
    dic[2]=0x0000bdc1;
    #endif
    
    if(value>=1000) {memcpy(out,dic+(value/1000),s);memcpy(out+s,dic+12,s);out=out+s+s;*flag=1;value=value%1000;}
    else if(*flag)  {memcpy(out,dic,s);out=out+s;*flag=0;}

    if(value>=100) {memcpy(out,dic+(value/100),s);memcpy(out+s,dic+11,s);out=out+s+s;*flag=1;value=value%100;}
    else if(*flag)  {memcpy(out,dic,s);out=out+s;*flag=0;}

    #if defined(__linux__)
    dic[2]=0x00a4b8e4;
    #elif defined(_WIN64)||defined(_WIN32)
    dic[2]=0x0000feb6;
    #endif

    if(value>=10) {memcpy(out,dic+(value/10),s);memcpy(out+s,dic+10,s);out=out+s+s;*flag=1;value=value%10;}
    else if(*flag)  {memcpy(out,dic,s);out=out+s;*flag=0;}

    if(value!=0) {memcpy(out,dic+value,s);out=out+s;}

    cnum_next:
    if(value2>0.0000001)
    {
        memcpy(out,dic+16,s);out=out+s;*flag=1;
        for(int i=0;i<9;i++)
        {
            if(value2<0.0000001) {*flag=0;break;}
            value2 = value2*10;
            value = (int)value2;
            value2=value2-(double)value;if(value2>0.999999) {value+=1;value2=0;}
            memcpy(out,dic+value,s);out=out+s;
        }
        if(*flag)
        {
            value2 = value2*10;
            value = (int)(value2+0.5);
            memcpy(out,dic+value,s);out=out+s;
        }
    }
    *out=0;
}

__thread char morn_shu[256];
const char *mShu(double data)
{
    int flag=0;
    _CNum(data,morn_shu,&flag);
    return morn_shu;
}

__thread int morn_layer_order = -1;
__thread int morn_exception = 0;
__thread jmp_buf *morn_jump[32];
void m_Exception(int err,int ID,const char *file,int line,const char *function,const char *message,...)
{
    if(err==0) return;
    char msg[256];
    va_list msgpara;
    va_start (msgpara,message);
    vsnprintf(msg,256,message,msgpara);
    va_end(msgpara);
    mLog(MORN_ERROR,"[%s,line %d,function %s]Error: %s\n",file,line,function,msg);
    if(err >0)
    {
        morn_exception = ID;
        if(morn_layer_order<0) exit(0);
        longjmp(*(morn_jump[morn_layer_order]),morn_exception);
    }
}

// struct FuncPara
// {
//     int parasize[16];
//     int64_t para[16];
// };
// #define MORN_FUNC(func,...)
// {
//     mFunc(func,N,
//           sizeof(
//     struct FuncPara para##func;
//     int N = VA_ARG_NUM(__VA_ARG__);
//     if(N>0)
//     {
//         int parasize = sizeof(VA_ARG0(__VA_ARG__);mException(parasize>8,EXIT,"invalid func para");
//         para##func->parasize = parasize;