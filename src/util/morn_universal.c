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

struct HandleFileCreate
{
    char filename[256];
};
void endFileCreate(void *info) {NULL;}
#define HASH_FileCreate 0xfdab2bff
MFile *FileCreate(const char *filename)
{
    MFile *file = mObjectCreate(NULL);
    MHandle *hdl = mHandle(file,FileCreate);
    hdl->valid=1;
    struct HandleFileCreate *handle = hdl->handle;
    file->filename = handle->filename;
    strcpy(file->filename,filename);
    return file;
}

void FileRedefine(MFile *file,char *filename)
{
    if(strcmp(filename,file->filename)!=0)
    {
        strcpy(file->filename,filename);
        mHandleReset(file->handle);
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

__thread MObject *morn_object=NULL;
MChain *morn_object_map;
#ifdef _MSC_VER
void morn_end()
{
    if(morn_object!=NULL) mObjectRelease(morn_object);
    morn_object=NULL;

    if(morn_object_map!=NULL)
    {
        MChainNode *node = morn_object_map->chainnode->next;
        while(node!=morn_object_map->chainnode){mObjectRelease(*(MObject **)mMapNodeValue(node));node=node->next;}
        mChainRelease(morn_object_map);
    }
    morn_object_map = NULL;
    // printf("after main\n"); 
}
void morn_begin()
{
    morn_object=mObjectCreate(NULL);
    morn_object_map=mChainCreate();
    // printf("before\n");
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
            while(node!=morn_object_map->chainnode){mObjectRelease(*(MObject **)mMapNodeValue(node));node=node->next;}
        }
        mChainRelease(morn_object_map);
    }
    morn_object_map = NULL;
}
#endif

MObject *mMornObject(void *p)
{
    if(p==NULL) 
    {
        if(morn_object==NULL) morn_object=mObjectCreate(NULL);
        return morn_object;
    }
    
    MObject **pobj = mMapRead(morn_object_map,&p,sizeof(void *),NULL,DFLT);
    if(pobj != NULL) return (*pobj);

    MObject *obj = mObjectCreate(p);
    pobj=mMapWrite(morn_object_map,&p,sizeof(void *),&obj,sizeof(MObject *));
    return (*pobj);
}

char morn_time_string[128];
char *mTimeString(time_t time_value,const char *format)
{
    time_t tv=(time_value<0)?time(NULL):time_value;
    struct tm *t=localtime(&tv);
    if(format==NULL) {strcpy(morn_time_string,asctime(t));return morn_time_string;}

    char *wday[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    char *month[12]={"January","February","March","April","May","June","July","August","September","October","November","December"};
    char *awday[7]={"Sun.","Mon.","Tues.","Wed.","Thur.","Fri.y","Sat."};
    char *amonth[12]={"Jan.","Feb.","Mar.","Apr.","May.","Jun.","Jul.","Aug.","Sept.","Oct.","Nov.","Dec."};
    
    char str[256];strcpy(str,format);
    intptr_t d[16];int n=0;
    char *p,*q;
    for(p=str;*p!=0;p++)
    {
        if(n>=16)break;
        if(*p=='/') {p++;continue;}
        if(*p=='%')
        {
            for(q=p+1;*q!=0;q++)
            {
                if((*q>='0')&&(*q<='9')) continue;
                // if(*q=='.') continue;
                     if(*q=='Y') {d[n++]=t->tm_year+1900; *q='d';}
                else if(*q=='M') {d[n++]=t->tm_mon+1;*q='d';}
                else if(*q=='W') {d[n++]=t->tm_wday; *q='d';}
                else if(*q=='D') {d[n++]=t->tm_mday; *q='d';}
                else if(*q=='H') {d[n++]=t->tm_hour; *q='d';}
                else if(*q=='m') {d[n++]=t->tm_min ; *q='d';}
                else if(*q=='S') {d[n++]=t->tm_sec ; *q='d';}
                else if((q[0]=='s')&&(q[1]=='M')) {d[n++]=(intptr_t)( month[t->tm_mon ]);q[0]='h';q[1]='s';q++;}
                else if((q[0]=='a')&&(q[1]=='M')) {d[n++]=(intptr_t)(amonth[t->tm_mon ]);q[0]='h';q[1]='s';q++;}
                else if((q[0]=='s')&&(q[1]=='W')) {d[n++]=(intptr_t)(  wday[t->tm_wday]);q[0]='h';q[1]='s';q++;}
                else if((q[0]=='a')&&(q[1]=='W')) {d[n++]=(intptr_t)( awday[t->tm_wday]);q[0]='h';q[1]='s';q++;}
                else mException(1,EXIT,"invalid format");
                break;
            }
            p=q;
        }
    }
    sprintf(morn_time_string,str,d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14],d[15]);
    return morn_time_string;
}

time_t mStringTime(char *in,const char *format)
{
    if(in == NULL) return time(NULL);
    if(format==NULL) format = "%sW %sM %D %H:%m:%S %Y";

    int day=0,month=0,year=0,week=0,hour=0,minute=0,second=0;
    char s_week[16],s_month[16];
    char *amonth[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    void *ptr[16];
    char str[128];memset(str,0,128);
    
    const char *p,*q;
    int n=0,m=0;
    for(p=format;*p!=0;p++)
    {
        if((n>=16)||(m>=128))break;
        str[m]=*p;m++;
        if(*p=='/') {p++;continue;}
        if(*p=='%')
        {
            for(q=p+1;*q!=0;q++)
            {
                if((*q>='0')&&(*q<='9')) continue;
                // if(*q=='.') continue;
                     if(*q=='Y') {ptr[n++]=&year  ;str[m++]='d';}
                else if(*q=='M') {ptr[n++]=&month ;str[m++]='d';}
                else if(*q=='W') {ptr[n++]=&week  ;str[m++]='d';}
                else if(*q=='D') {ptr[n++]=&day   ;str[m++]='d';}
                else if(*q=='H') {ptr[n++]=&hour  ;str[m++]='d';}
                else if(*q=='m') {ptr[n++]=&minute;str[m++]='d';}
                else if(*q=='S') {ptr[n++]=&second;str[m++]='d';}
                else if(((q[0]=='s')||(q[0]=='a'))&&((q[1]=='M')||(q[1]=='W')))
                {
                    ptr[n++]=(q[1]=='M')?s_month:s_week;
                    if(!q[2]) str[m++]='s';
                    else
                    {
                        str[m++]='[';
                        str[m++]='^';str[m++]='0';str[m++]='-';str[m++]='9';
                        str[m++]='^';str[m++]=q[2];
                        str[m++]=']';
                    }
                    q++;
                }
                else mException(1,EXIT,"invalid format");
                break;
            }
            p=q;
        }
    }
    // printf("%s\n",str);
    sscanf(in,str,ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8],ptr[9],ptr[10],ptr[11],ptr[12],ptr[13],ptr[14],ptr[15]);
    if(month==0) {s_month[3]=0; for(int i=0;i<12;i++) {if(stricmp(s_month,amonth[i])==0) {month=i+1;break;}}}
    if((year==0)||(month ==0)||(day   ==0)) return DFLT;
    if((hour==0)||(minute==0)||(second==0)) return DFLT;
    struct tm t;t.tm_year=year-1900; t.tm_mon=month-1;t.tm_mday=day;t.tm_hour=hour;t.tm_min=minute;t.tm_sec=second;
    return mktime(&t);
}