/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

#if defined(_WIN64)||defined(_WIN32)
#include <Windows.h>
#include <io.h>
#define access _access
#define F_OK 0

#define m_Exist(ID) (GetProcessVersion(ID/1000)!=0)
#define m_Open(Filename) CreateFile(Filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,0)
#define m_Close(File) CloseHandle(File)
#define m_Read(File,Locate,Pointer,Size) do{int A;\
    do{A=SetFilePointer(File,Locate,NULL,FILE_BEGIN);}while(A!=Locate);\
    do{A=ReadFile(File,Pointer,Size,NULL,NULL);}while(A==0);\
}while(0)
#define m_Write(File,Locate,Pointer,Size) do{int A;\
    do{A=SetFilePointer(File,Locate,NULL,FILE_BEGIN);}while(A!=Locate);\
    do{A=WriteFile(File,Pointer,Size,NULL,NULL);}while(A==0);\
}while(0)
#define m_Lock(File)   LockFile(File,0,0,2*sizeof(int),0)
#define m_Unlock(File) UnlockFile(File,0,0,2*sizeof(int),0)
// #define m_Fsize(File) GetFileSize(File,NULL)
#define m_Fsize(File) SetFilePointer(File,0,NULL,FILE_END)
#define m_Mmap(File,Pointer,Size) do{\
    HANDLE Map = CreateFileMapping(File,NULL,PAGE_READWRITE,0,Size,NULL);\
    Pointer = MapViewOfFile(Map,FILE_MAP_ALL_ACCESS,0,0,Size);\
    CloseHandle(Map);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Mrmap(File,Pointer,Size) do{\
    HANDLE Map = CreateFileMapping(File,NULL,PAGE_READONLY,0,Size,NULL);\
    Pointer = MapViewOfFile(Map,FILE_MAP_ALL_ACCESS,0,0,Size);\
    CloseHandle(Map);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)

#define m_Munmap(Pointer,Size) UnmapViewOfFile(Pointer);
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <signal.h>
#define m_Exist(ID) (kill((ID/1000),0)==0)
#define m_Open(Filename) open(Filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#define m_Close(File) close(File)
#define m_Read(File,Locate,Pointer,Size) do{\
    mException((lseek(File,Locate,SEEK_SET)!=Locate),EXIT,"error with file lseek");\
    mException((read(File,Pointer,Size)!=Size),EXIT,"error with file read");\
}while(0)
#define m_Write(File,Locate,Pointer,Size) do{\
    mException((lseek(File,Locate,SEEK_SET)!=Locate),EXIT,"error with file lseek");\
    mException((write(File,Pointer,Size)!=Size),EXIT,"error with file write");\
}while(0)
#define m_Lock(File)   flock(File,LOCK_EX)
#define m_Unlock(File) flock(File,LOCK_UN)
#define m_Fsize(File) lseek(File,1,SEEK_END)
#define m_Mmap(File,Pointer,Size) do{\
    Pointer=mmap(NULL,(Size),PROT_READ|PROT_WRITE,MAP_SHARED,File,0);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Mrmap(File,Pointer,Size) do{\
    Pointer=mmap(NULL,(Size),PROT_READ,MAP_SHARED,File,0);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Munmap(Pointer,Size) munmap(Pointer,Size);
#endif

#define WAIT {if(handle->wait_time>0) {if(mTime()-t0>handle->wait_time) return NULL;}}

struct ProcCommunicateState
{
    int locate;
    int size;
};
struct ProcTopicInfo
{
    volatile int32_t ID;
    volatile int32_t user_num;
    volatile int32_t writer_num;
    volatile int32_t write_locate;
    volatile int32_t write_state;
             int32_t reserve;
    volatile int64_t write_order;
    
    volatile struct ProcCommunicateState topic[128];
    char ptr[0];
};

struct HandleProcTopic
{
    char filename[256];
    
    int ID;
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file;
    #else
    int file;
    #endif
    
    int filesize;
    int topicsize;
    
    struct ProcTopicInfo *info;
    int64_t order_read;
};
void endProcTopic(struct HandleProcTopic *handle)
{
    if(handle->info==NULL) return;
    struct ProcTopicInfo *info = handle->info;

    m_Lock(handle->file);
    int32_t user_num;m_Read(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
    user_num--;     m_Write(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
    info->user_num =user_num;
    m_Unlock(handle->file);

    m_Munmap(info,handle->filesize+sizeof(struct ProcTopicInfo));
    
    m_Close(handle->file);
    if(user_num==0) {mSleep(10);remove(handle->filename);}//printf("bba\n");}
}
#define HASH_ProcTopic 0x72f379ee
void *m_ProcTopicWrite(const char *topicname,void *data,int write_size)
{
    if(topicname==NULL) topicname="morn_topic";
    mException(data==NULL,EXIT,"invalid input");
    
    if(write_size<0) write_size=strlen(data)+1;
    int size = ((write_size+7)>>3)<<3;

    int ID=0;

    MHandle *hdl = mHandle(topicname,ProcTopic);
    struct HandleProcTopic *handle = (struct HandleProcTopic *)(hdl->handle);
    if(!mHandleValid(hdl))
    {
        handle->ID= getpid()*1000+mThreadID();
        // printf("handle->ID=%d\n",handle->ID);
        mPropertyFunction(topicname,"exit",mornObjectRemove,topicname);
        mPropertyVariate(topicname,"topic_size",&(handle->topicsize),sizeof(int));
        
        char dirname[128];
        #if defined(_WIN64)||defined(_WIN32)
        sprintf(dirname,"%s/morn_topic",getenv("TEMP"));
        if(access(dirname,F_OK)<0) mkdir(dirname);
        #else
        umask(0x000);
        sprintf(dirname,"/tmp/morn_topic");
        if(access(dirname,F_OK)<0) mkdir(dirname,0777);
        #endif
        sprintf(handle->filename,"%s/.%s.bin",dirname,topicname);

        int flag = access(handle->filename,F_OK);
        handle->file = m_Open(handle->filename);
        if(flag>=0)
        {
            // m_Read(handle->file,sizeof(int),&ID,sizeof(int));
            handle->filesize = m_Fsize(handle->file)-sizeof(struct ProcTopicInfo);
        }
        if(flag<0)
        {
            if(handle->topicsize==0) handle->topicsize = MAX(size*2,1024);
            handle->filesize = handle->topicsize*16;
            m_Write(handle->file,handle->filesize+sizeof(struct ProcTopicInfo),&ID,sizeof(int));
        }
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcTopicInfo));
        
        if(flag<0)
        {
            handle->info->write_locate= 0;
            handle->info->write_order = 0;
            int size = handle->filesize/8;
            for(int i=0;i<8;i++)
            {
                handle->info->topic[i].locate= size*i;
                handle->info->topic[i].size  = size;
            }
        }
        
        int32_t user_num=0;int32_t writer_num=0;
        m_Lock(handle->file);
        if(flag>=0)
        {
            m_Read(handle->file,  sizeof(int32_t),  &user_num,sizeof(int32_t));
            m_Read(handle->file,2*sizeof(int32_t),&writer_num,sizeof(int32_t));
        }
        user_num++;  m_Write(handle->file,  sizeof(int32_t),&user_num  ,sizeof(int32_t));handle->info->user_num  = user_num;
        writer_num++;m_Write(handle->file,2*sizeof(int32_t),&writer_num,sizeof(int32_t));handle->info->writer_num= writer_num;
        m_Unlock(handle->file);
 
        hdl->valid = 1;
    }
    struct ProcTopicInfo *info=handle->info;
    if(size>handle->filesize/2)
    {
        endProcTopic(handle);
        handle->info=NULL;
        mSleep(10);
        remove(handle->filename);
        mException(1,EXIT,"invalid topic size");
    }
    
    if(info->writer_num>1)
    {
        while(1)
        {
            m_Lock(handle->file);
            if((info->write_state==0)||(info->ID==0)) 
            {
                info->write_state=1;info->ID=handle->ID;
                m_Unlock(handle->file);break;
            }
            if(m_Exist(info->ID)==0){info->writer_num-=1;info->ID=0;}
            m_Unlock(handle->file);
        }
    }

    int order = (info->write_order)%8;
    int locate= (info->write_locate);if(locate+size>handle->filesize)locate=0;
    
    info->topic[order].size =write_size;
    info->topic[order].locate = locate;
    memcpy(info->ptr+locate,data,write_size);

    info->write_locate=locate+size;
    info->write_order+=1;
    
    if(info->writer_num>1) info->write_state=0;
    return (info->ptr+locate);
}

void *m_ProcTopicRead(const char *topicname,void *data,int *read_size)
{
    if(topicname==NULL) topicname="morn_topic";
    MHandle *hdl = mHandle(topicname,ProcTopic);
    struct HandleProcTopic *handle = (struct HandleProcTopic *)(hdl->handle);
    if(!mHandleValid(hdl))
    {
        mPropertyFunction(topicname,"exit",mornObjectRemove,topicname);
        handle->order_read=-1;
        if(read_size!=NULL) *read_size = 0;

        if(handle->filesize==0)
        {
            char dirname[128];
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(dirname,"%s/morn_topic",getenv("TEMP"));
            if(access(dirname,F_OK)<0) mkdir(dirname);
            #else
            sprintf(dirname,"/tmp/morn_topic");
            if(access(dirname,F_OK)<0) mkdir(dirname,0777);
            #endif
            sprintf(handle->filename,"%s/.%s.bin",dirname,topicname);
            // printf("filename=%s\n",handle->filename);

            int flag = access(handle->filename,F_OK); if(flag<0) {hdl->valid=0;return NULL;}
            handle->file = m_Open(handle->filename);
     
            int filesize=m_Fsize(handle->file);
            if(filesize<sizeof(struct ProcTopicInfo)) {hdl->valid=0;return NULL;}

            handle->filesize=filesize;
        }
        int ID;m_Read(handle->file,0,&ID,sizeof(int));
        if(m_Exist(ID)==0) {hdl->valid=0;return NULL;}

        int64_t write_order;m_Read(handle->file,6*sizeof(int),&write_order,sizeof(int64_t));
        if(write_order==0) {hdl->valid=0;return NULL;}

        m_Mmap(handle->file,handle->info,handle->filesize);
        
        m_Lock(handle->file);
        int32_t user_num;m_Read(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
        user_num++;     m_Write(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
        handle->info->user_num =user_num;
        m_Unlock(handle->file);

        hdl->valid = 1;
    }
    struct ProcTopicInfo *info=handle->info;
    
    int64_t order_read=info->write_order-1;
    
    int order = order_read%8;
    int locate= info->topic[order].locate;
    int size  = info->topic[order].size;

    if(read_size!=NULL) {if(*read_size>0) {size=MIN(*read_size,size);} *read_size=size;}
    if(data!=NULL) memcpy(data,info->ptr+locate,size);

    // printf("order_read=%d,handle->order_read=%d\n",order_read,handle->order_read);
    char *p =(order_read>handle->order_read)?(info->ptr+locate):NULL;
    handle->order_read = order_read;
    return p;
}


struct ProcMessageInfo
{
    volatile int32_t wID;
    volatile int32_t user_num;
    volatile int32_t writer_num;
    volatile int32_t write_locate;
    volatile int32_t write_state;
             int32_t reserve;
    volatile int64_t write_order;
    
    volatile int32_t reader_num;
    volatile int32_t read_locate;
    volatile int32_t read_order;
    
    volatile int rID[256];
    volatile uint64_t rorder[256];
    
    volatile struct ProcCommunicateState message[128];
    
    char ptr[0];
    
};

struct HandleProcMessage
{
    char filename[256];
    
    int ID;
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file;
    #else
    int file;
    #endif
    
    int filesize;
    int messagesize;
    int64_t reader_idx;
    
    struct ProcMessageInfo *info;
};
#define HASH_ProcMessage 0xbc2aef80
void endProcMessage(struct HandleProcMessage *handle)
{
    if(handle->info==NULL) return;
    struct ProcMessageInfo *info = handle->info;
    
    int idx= handle->reader_idx;
    if(idx>=0)
    {
        info->rID[idx]=0;
        info->rorder[idx]=-1;
        info->reader_num-=1;
    }

    m_Lock(handle->file);
    int32_t user_num;m_Read(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
    user_num--;     m_Write(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
    info->user_num =user_num;
    m_Unlock(handle->file);
    
    m_Munmap(info,handle->filesize+sizeof(struct ProcMessageInfo));
    
    m_Close(handle->file);
    if(user_num==0) {mSleep(10);remove(handle->filename);}
}

void *m_ProcMessageWrite(const char *messagename,void *data,int write_size)
{
    if(messagename==NULL) messagename="morn_message";
    mException(data==NULL,EXIT,"invalid input");
    
    if(write_size<0) write_size=strlen(data)+1;
    int size = ((write_size+7)>>3)<<3;

    MHandle *hdl = mHandle(messagename,ProcMessage);
    struct HandleProcMessage *handle = (struct HandleProcMessage *)(hdl->handle);
    if(!mHandleValid(hdl))
    {
        handle->ID= getpid()*1000+mThreadID();
        mPropertyFunction(messagename,"exit",mornObjectRemove,messagename);
        mPropertyVariate(messagename,"message_size",&(handle->messagesize),sizeof(int));
        
        char dirname[128];
        #if defined(_WIN64)||defined(_WIN32)
        sprintf(dirname,"%s/morn_message",getenv("TEMP"));
        if(access(dirname,F_OK)<0) mkdir(dirname);
        #else
        umask(0x000);
        sprintf(dirname,"/tmp/morn_message");
        if(access(dirname,F_OK)<0) mkdir(dirname,0777);
        #endif
        sprintf(handle->filename,"%s/.%s.bin",dirname,messagename);

        int flag = access(handle->filename,F_OK);
        handle->file = m_Open(handle->filename);
        m_Lock(handle->file);
        if(flag>=0)
        {
            handle->filesize = m_Fsize(handle->file)-sizeof(struct ProcMessageInfo);
        }
        if(flag<0)
        {
            if(handle->messagesize==0) handle->messagesize = MAX(size*2,1024);
            handle->filesize = handle->messagesize*256;
            m_Write(handle->file,handle->filesize+sizeof(struct ProcMessageInfo),&flag,sizeof(int));
        }
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcMessageInfo));
        
        if(flag<0)
        {
            handle->info->wID=handle->ID;
            handle->info->write_locate= 0;
            handle->info->write_order = 0;
            int size = handle->filesize/128;
            for(int i=0;i<128;i++)
            {
                handle->info->message[i].locate= size*i;
                handle->info->message[i].size  = size;
            }
            handle->info->reader_num = 0;
            handle->info->read_locate= 0;
            handle->info->read_order = 0;
            for(int i=0;i<256;i++){handle->info->rID[i]=0;handle->info->rorder[i]=-1;}
        }
        
        handle->reader_idx=-1;
        int32_t user_num=0;int32_t writer_num=0;
        
        if(flag>=0)
        {
            m_Read(handle->file,  sizeof(int32_t),  &user_num,sizeof(int32_t));
            m_Read(handle->file,2*sizeof(int32_t),&writer_num,sizeof(int32_t));
        }
        user_num++;  m_Write(handle->file,  sizeof(int32_t),&user_num  ,sizeof(int32_t));handle->info->user_num  = user_num;
        writer_num++;m_Write(handle->file,2*sizeof(int32_t),&writer_num,sizeof(int32_t));handle->info->writer_num= writer_num;
        m_Unlock(handle->file);
 
        hdl->valid = 1;
    }
    struct ProcMessageInfo *info=handle->info;
    if(size>handle->filesize/16)
    {
        endProcMessage(handle);
        handle->info=NULL;
        mSleep(10);
        remove(handle->filename);
        mException(1,EXIT,"invalid message size");
    }
    
    while(info->writer_num>1)
    {
        m_Lock(handle->file);
        if((info->write_state==0)||(info->wID==0)) 
        {
            info->write_state=1;
            m_Unlock(handle->file);break;
        }
        if(m_Exist(info->wID)==0) info->writer_num-=1;
        m_Unlock(handle->file);
        mSleep(1);
    }
    info->wID=handle->ID;
    
    if(info->write_locate+size>handle->filesize) info->write_locate = 0;
    while(((info->write_locate<info->read_locate)&&(info->write_locate+size>info->read_locate))||(info->write_order-info->read_order>=128))
    {
        if(info->reader_num==0) break;
        int order_min=-1;
        for(int i=0;i<info->reader_num;i++)
            order_min = MIN(info->rorder[i],order_min);
        
        info->read_order = order_min;
        info->read_locate = info->message[order_min%128].locate;
        mSleep(1);
    }
    
    int order =(info->write_order)%128;
    int locate= info->write_locate;if(locate+size>handle->filesize)locate=0;
    
    info->message[order].size =write_size;
    info->message[order].locate = locate;
    memcpy(info->ptr+locate,data,write_size);

    info->write_locate=locate+size;
    info->write_order+=1;
    
    info->write_state=0;
    return (info->ptr+locate);
}

void *m_ProcMessageRead(const char *messagename,void *data,int *read_size)
{
    if(messagename==NULL) messagename="morn_message";
    MHandle *hdl = mHandle(messagename,ProcMessage);
    struct HandleProcMessage *handle = (struct HandleProcMessage *)(hdl->handle);
    if(!mHandleValid(hdl))
    {
        mPropertyFunction(messagename,"exit",mornObjectRemove,messagename);

        if(read_size!=NULL) *read_size = 0;
        
        handle->ID = getpid()*1000+mThreadID();

        if(handle->filesize==0)
        {
            char dirname[128];
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(dirname,"%s/morn_message",getenv("TEMP"));
            if(access(dirname,F_OK)<0) mkdir(dirname);
            #else
            sprintf(dirname,"/tmp/morn_message");
            if(access(dirname,F_OK)<0) mkdir(dirname,0777);
            #endif
            sprintf(handle->filename,"%s/.%s.bin",dirname,messagename);

            int flag=-1;
            while(flag<0) {flag= access(handle->filename,F_OK);}
            handle->file = m_Open(handle->filename);
     
            int filesize=0;
            while(filesize<sizeof(struct ProcMessageInfo)) {filesize=m_Fsize(handle->file);}

            handle->filesize=filesize;
        }
        int wID;m_Read(handle->file,0,&wID,sizeof(int));
        while(m_Exist(wID)==0) m_Read(handle->file,0,&wID,sizeof(int));

        int64_t write_order=0;
        while(write_order==0) m_Read(handle->file,6*sizeof(int),&write_order,sizeof(int64_t));
        
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcMessageInfo));
        
        m_Lock(handle->file);
        
        for(int i=0;i<=handle->info->reader_num;i++)
        {
            if(handle->info->rorder[i]==-1)
            {
                if(i==handle->info->reader_num)
                {
                    mException(i==256,EXIT,"too many readers");
                    handle->info->reader_num=i+1;
                }
                handle->info->rorder[i]=handle->info->write_order;
                handle->info->rID[i]=handle->ID;
                handle->reader_idx=i;
                break;
            }
        }
        
        int32_t user_num;m_Read(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
        user_num++;     m_Write(handle->file,sizeof(int32_t),&user_num,sizeof(int32_t));
        handle->info->user_num =user_num;
        
        m_Unlock(handle->file);

        hdl->valid = 1;
    }
    struct ProcMessageInfo *info=handle->info;
    int idx=handle->reader_idx;
    
    int order_read = info->rorder[idx];
    while(order_read>=info->write_order) mSleep(1);
    info->rorder[idx]=order_read+1;
    
    int order = order_read%128;
    int locate= info->message[order].locate;
    int size  = info->message[order].size;
    
    if(read_size!=NULL) {if(*read_size>0) {size=MIN(*read_size,size);} *read_size=size;}
    if(data!=NULL) memcpy(data,info->ptr+locate,size);

    return info->ptr+locate;
}

struct HandleProcVariate
{
    char idxname[256];
    char dataname[256];
    int32_t ID;
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file_idx;
    HANDLE file_data;
    #else
    int file_idx;
    int file_data;
    #endif

    int size_idx;

    uint8_t *buff_idx;
    uint8_t *master_data;
    uint8_t *slave_data;
    MMap *map;
};
void endProcVariate(struct HandleProcVariate *handle)
{
    if(handle->map!=NULL) mMapRelease(handle->map);

    int user_num;m_Read(handle->file_idx,8,&user_num,sizeof(int32_t));
    user_num--; m_Write(handle->file_idx,8,&user_num,sizeof(int32_t));

    if(handle->buff_idx   !=NULL) m_Munmap(handle->buff_idx   ,65536);
    if(handle->master_data!=NULL) m_Munmap(handle->master_data,65536);
    if(handle->slave_data !=NULL) m_Munmap(handle->slave_data ,65536);
    m_Close(handle->file_idx);
    m_Close(handle->file_data);
    if(user_num==0) 
    {
        mSleep(10);remove(handle->dataname);
        mSleep(10);remove(handle->idxname);
    }
}
#define HASH_ProcVariate 0x45828c6f
void *m_ProcVariate(const char *name,int size,int type)
{
    mException((name==NULL)||(size<=0)||(size>128),EXIT,"invalid input");

    MHandle *hdl = mHandle("ProcVariate",ProcVariate);
    struct HandleProcVariate *handle = (struct HandleProcVariate *)(hdl->handle);
    if(!mHandleValid(hdl))
    {
        mPropertyFunction("ProcVariate","exit",mornObjectRemove,"ProcVariate");
        handle->ID= getpid()*1000+mThreadID();

        #if defined(_WIN64)||defined(_WIN32)
        char *dirname=getenv("TEMP");
        #else
        umask(0x000);
        char *dirname="/tmp";
        #endif
        
        sprintf(handle->idxname,"%s/.morn_variate_idx.bin",dirname);
        int flag = access(handle->idxname,F_OK);
        handle->file_idx = m_Open(handle->idxname);
        if(flag>=0)
        {
            mException(m_Fsize(handle->file_idx)<65536,EXIT,"invalid map file %s",handle->idxname);
            int user_num;m_Read(handle->file_idx,8,&user_num,sizeof(int32_t));
            user_num++; m_Write(handle->file_idx,8,&user_num,sizeof(int32_t));
        }
        else
        {
            int n;
            n= 0;m_Write(handle->file_idx,65532,&n,sizeof(int32_t));
            n= 8;m_Write(handle->file_idx,0    ,&n,sizeof(int32_t));
            n= 0;m_Write(handle->file_idx,4    ,&n,sizeof(int32_t));
        }
        m_Mmap(handle->file_idx,handle->buff_idx,65536);
        handle->size_idx = 8;

        sprintf(handle->dataname,"%s/.morn_variate_data.bin",dirname);
        flag = access(handle->dataname,F_OK);
        handle->file_data = m_Open(handle->dataname);
        
        if(flag>=0)
            mException(m_Fsize(handle->file_data)<65536,EXIT,"invalid map file %s",handle->dataname);
        else
        {
            int n;
            n=0;m_Write(handle->file_data,65532,&n,sizeof(int32_t));
            n=8;m_Write(handle->file_data,0    ,&n,sizeof(int32_t));
            n=0;m_Write(handle->file_data,4    ,&n,sizeof(int32_t));
        }
        m_Mmap( handle->file_data,handle->master_data,65536);
        m_Mrmap(handle->file_data,handle->slave_data ,65536);
        
        if(handle->map==NULL) handle->map = mMapCreate();
        hdl->valid = 1;
    }

    void *p_data=(type==DFLT)?handle->master_data:handle->slave_data;
    
    uint8_t *pvalue=NULL;
    uint16_t offset=0,keysize,valuesize;
    
    mMapRead(handle->map,name,DFLT,&offset,NULL);
    if(offset) return (p_data+offset);

    m_Lock(handle->file_idx);
    int *p_idx = (int *)(handle->buff_idx);
    while(p_idx[1]!=0) {if(!m_Exist(p_idx[1])) break;} p_idx[1]=getpid()*1000;
    int size_idx = p_idx[0];
    
    uint8_t *p=handle->buff_idx+8+handle->size_idx;
    while(handle->size_idx<size_idx)
    {
        offset     =*((uint16_t *)(p  ));
        keysize    =*((uint16_t *)(p+2));
        int32_t *ID=  ( int32_t *)(p+4) ;
        char   *key=  (    char *)(p+8) ;
        
        pvalue=p_data+offset;
        mMapWrite(handle->map,key,DFLT,&offset,sizeof(uint16_t));

        handle->size_idx += 8+keysize;
        p=handle->buff_idx+8+handle->size_idx;

        if(strcmp(name,key)==0) 
        {
            if(type==DFLT)
            {
                if(*ID==0) *ID=handle->ID;
                else if(*ID!=handle->ID)
                {
                    mException(m_Exist(*ID),EXIT,"mult process master variate");
                    *ID=handle->ID;
                }
            }
            m_Unlock(handle->file_idx);p_idx[1]=0;
            return pvalue;
        }
    }
    
    keysize=(strlen(name)+1+3)/4;
    keysize=keysize*4;
    mException(keysize>128,EXIT,"invalid input");
   
    valuesize = (size+7)/8;
    valuesize = valuesize*8;

    m_Lock(handle->file_data);
    handle->size_idx += 8+keysize;
    mException(handle->size_idx>65536,EXIT,"too much variate\n");
    
    int size_data = *((int *)(handle->master_data));
    mException(size_data+valuesize>65536,EXIT,"too much variate\n");

    *((uint16_t *)(p  ))=size_data;
    *((uint16_t *)(p+2))=keysize;
    *(( int32_t *)(p+4))=(type==DFLT)?handle->ID:0;
    memcpy((char *)(p+8),name,keysize);
    offset = size_data;
    pvalue = p_data+offset;
    memset(pvalue,DFLT,valuesize);
    size_data += valuesize;
    
    p_idx[0] =handle->size_idx;
    *((int *)(handle->master_data)) = size_data;

    m_Unlock(handle->file_data);
    m_Unlock(handle->file_idx);
    p_idx[1]=0;

    mMapWrite(handle->map,p+4,DFLT,&offset,sizeof(uint16_t));
    return pvalue;
}

struct HandleProcLock
{
    int ID;
    int state;
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file;
    #else
    int file;
    #endif
    char filename[256];
};
void endProcLock(struct HandleProcLock *handle)
{
    if(handle->file!=0)
    {
        int num;
        m_Lock(handle->file);
        m_Read(handle->file,sizeof(int),&num,sizeof(int));
        num--;
        m_Write(handle->file,sizeof(int),&num,sizeof(int));
        m_Unlock(handle->file);
        m_Close(handle->file);
        if(num==0) remove(handle->filename);
    }
}
#define HASH_ProcLock 0x426e864
void m_ProcLockBegin(const char *mutexname)
{
    if(mutexname==NULL) mutexname="morn_procmutex";
    MHandle *hdl = mHandle(mMornObject((void *)mutexname,DFLT),ProcLock);
    struct HandleProcLock *handle = (struct HandleProcLock *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->ID=getpid()*1000+mThreadID();
        if(handle->filename[0]==0)
        {
            char dirname[128];
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(dirname,"%s/morn_lock",getenv("TEMP"));
            if(access(dirname,F_OK)<0) mkdir(dirname);
            #else
            sprintf(dirname,"/tmp/morn_lock");
            if(access(dirname,F_OK)<0) mkdir(dirname,0777);
            #endif
            sprintf(handle->filename,"%s/.%s.bin",dirname,mutexname);
            // printf("filename=%s\n",handle->filename);
        }
        
        int num=0;int ID=0;
        int flag = access(handle->filename,F_OK);
        handle->file = m_Open(handle->filename);
        if(flag>=0){volatile int fsize;do {fsize = m_Fsize(handle->file);}while(fsize<2*sizeof(int));}
        m_Lock(handle->file);
        if(flag>=0)
        {
            m_Read(handle->file,0,&ID,sizeof(int));
            if(ID!=0) {if(!m_Exist(ID)) {flag=-1;ID=0;}}
        }
        if(flag>=0) m_Read(handle->file,sizeof(int),&num,sizeof(int));
        else m_Write(handle->file,0,&ID,sizeof(int));
        num++;m_Write(handle->file,sizeof(int),&num,sizeof(int));
//         printf("num=%d,handle->ID=%d\n",num,handle->ID);
        
        m_Unlock(handle->file);

        handle->state=0;
        
        hdl->valid = 1;
    }
    int ID;
    do
    {
        m_Lock(handle->file);
        m_Read(handle->file,0,&ID,sizeof(int));
        if(ID==0)
        {
            ID=handle->ID;
            m_Write(handle->file,0,&ID,sizeof(int));
        }
        m_Unlock(handle->file);
    }while(ID!=handle->ID);
    handle->state=1;
}

void m_ProcLockEnd(const char *mutexname)
{
    if(mutexname==NULL) mutexname="morn_process";
    MHandle *hdl = mHandle(mMornObject((void *)mutexname,DFLT),ProcLock);
    struct HandleProcLock *handle = (struct HandleProcLock *)(hdl->handle);
    mException((hdl->valid==0),EXIT,"no process mutex named %s",mutexname);
    mException(handle->state!=1,EXIT,"unlock error");
    int ID=0;
//     m_Lock(handle->file);
    m_Write(handle->file,0,&ID,sizeof(int));
//     m_Unlock(handle->file);
    handle->state=0;
}

