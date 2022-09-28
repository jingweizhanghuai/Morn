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
    Pointer = MapViewOfFile(Map,FILE_MAP_READ,0,0,Size);\
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
char morn_message_file[128]={0};

struct ProcTopicState
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
    
    volatile struct ProcTopicState topic[128];
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
        mPropertyVariate(topicname,"topic_size",&(handle->topicsize));
        
        if(morn_message_file[0]==0)
        {
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(morn_message_file,"%s/morn_message",getenv("TEMP"));
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file);
            #else
            sprintf(morn_message_file,"/tmp/morn_message");
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file,0777);
            #endif
        }
        sprintf(handle->filename,"%s/.%s.bin",morn_message_file,topicname);

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
    
    // printf("info->writer_num=%d\n",info->writer_num);
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
            if(morn_message_file[0]==0)
            {
                #if defined(_WIN64)||defined(_WIN32)
                sprintf(morn_message_file,"%s/morn_message",getenv("TEMP"));
                if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file);
                #else
                sprintf(morn_message_file,"/tmp/morn_message");
                if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file,0777);
                #endif
            }
            sprintf(handle->filename,"%s/.%s.bin",morn_message_file,topicname);
            // printf("filename=%s\n",handle->filename);

            int flag = access(handle->filename,F_OK); if(flag<0) return NULL;
            handle->file = m_Open(handle->filename);
     
            int filesize=m_Fsize(handle->file);
            if(filesize<sizeof(struct ProcTopicInfo)) return NULL;

            handle->filesize=filesize;
        }
        int ID;m_Read(handle->file,0,&ID,sizeof(int));
        if(m_Exist(ID)==0) return NULL;

        int64_t write_order;m_Read(handle->file,6*sizeof(int),&write_order,sizeof(int64_t));
        if(write_order==0) return NULL;

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

// void UserSet(int *user,int ID)
// {
//     for(int i=0;i<1024;i++)
//     {
//         if(user[i]==0)   {user[i]=ID; return;}
//         if(!m_Exist(ID)) {user[i]=ID; return;}
//     }
//     mException(1,EXIT,"too many user");
// }
// int UserUnset(int *user,int ID)
// {
//     int count = 0;
//     for(int i=0;i<1024;i++)
//     {
//         if(user[i]==0) continue;
//         if(user[i]==ID) {user[i]=0;continue;}
//         if(!m_Exist(ID)){user[i]=0;continue;}
//         count++;
//     }
//     return count;
// }

struct ProcMessageState
{
    int locate;
    int size;
    char dst[32];
};
struct ProcMessageInfo
{
    volatile int ID;
    volatile int user_num;
    volatile int writer_num;
    volatile int write_locate;
    volatile int64_t write_order;
    volatile int64_t read_order;

    // int user[1024];
    struct ProcMessageState message[1024];
    char ptr[0];
};

/*
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
    int wait_time;
    
    struct ProcMessageInfo *info;
    int64_t order_read;
};
void endProcMessage(struct HandleProcMessage *handle)
{
    if(handle->info==NULL) return;
    struct ProcMessageInfo *info = handle->info;

    m_Lock(handle->file);
    int user_num;m_Read(handle->file,sizeof(int),&user_num,sizeof(int));
    user_num--; m_Write(handle->file,sizeof(int),&user_num,sizeof(int));
    info->user_num =user_num;
    m_Unlock(handle->file);

    m_Munmap(info,handle->filesize+sizeof(struct ProcMessageInfo));
    
    m_Close(handle->file);
    if(user_num==0) {mSleep(10);remove(handle->filename);}
}
#define HASH_ProcMessage 0xbc2aef80

struct HandleProcMessage *morn_proc_message_handle = NULL;
void *m_ProcMessageWrite(const char *dstname,void *data,int write_size)
{
    mException((dstname==NULL)||(data==NULL),EXIT,"invalid input");
    
    if(write_size<0) write_size=strlen(data)+1;
    int size = ((write_size+7)>>3)<<3;

    int ID=0;int user_num=0;int writer_num=0;

    struct HandleProcMessage *handle=morn_proc_message_handle;
    if(handle==NULL)
    {
        MHandle *hdl = mHandle("ProcMessage",ProcMessage);
        handle = (struct HandleProcMessage *)(hdl->handle);
    
        handle->ID= getpid()*1000+mThreadID();
        handle->wait_time = DFLT;
        mPropertyVariate("ProcMessage","wait_time",&(handle->wait_time));
        mPropertyFunction("ProcMessage","exit",mornObjectRemove,"ProcMessage");
        
        if(morn_message_file[0]==0)
        {
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(morn_message_file,"%s/morn_message",getenv("TEMP"));
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file);
            #else
            sprintf(morn_message_file,"/tmp/morn_message");
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file,0777);
            #endif
        }
        sprintf(handle->filename,"%s/.morn_message.bin",morn_message_file);

        int flag = access(handle->filename,F_OK);
        handle->file = m_Open(handle->filename);
        if(flag>=0)
        {
            m_Read(handle->file,sizeof(int),&ID,sizeof(int));
            handle->filesize = m_Fsize(handle->file)-sizeof(struct ProcMessageInfo);
        }
        if(flag<0) 
        {
            handle->filesize = MAX(size,256)*2048;
            m_Write(handle->file,handle->filesize+sizeof(struct ProcMessageInfo)-sizeof(int),&ID,sizeof(int));
        }
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcMessageInfo));
        
        if(flag<0)
        {
            handle->info->write_locate= 0;
            handle->info->write_order = 0;
            handle->info-> read_order = 0;
            int size = handle->filesize/1024;
            for(int i=0;i<1024;i++)
            {
                handle->info->message[i].locate= size*i;
                handle->info->message[i].size  = size;
            }
        }
        
        m_Lock(handle->file);
        if(flag>=0)
        {
            m_Read(handle->file,2*sizeof(int),  &user_num,sizeof(int));
            m_Read(handle->file,3*sizeof(int),&writer_num,sizeof(int));
        }
        if(writer_num==0) ID=handle->ID;else ID=0;
                     m_Write(handle->file,           0 ,         &ID ,sizeof(int));handle->info->ID        = ID;
        user_num++;  m_Write(handle->file,  sizeof(int),    &user_num,sizeof(int));handle->info->user_num  = user_num;
        writer_num++;m_Write(handle->file,2*sizeof(int),  &writer_num,sizeof(int));handle->info->writer_num= writer_num;
        m_Unlock(handle->file);
 
        handle->order_read = MAX(handle->info->write_order,0);
        hdl->valid = 1;
        morn_proc_message_handle=handle;
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
    // printf("write handle->order_read=%d\n",handle->order_read);
    // printf("write info->write_order =%d\n",info->write_order );
    
    if(info->writer_num>1) do
    {
        m_Lock(handle->file);
        m_Read(handle->file,0,&ID,sizeof(int));
        if(ID==0)
        {
            ID=handle->ID;info->ID=ID;
            m_Write(handle->file,0,&ID,sizeof(int));
        }
        m_Unlock(handle->file);
    }while(ID!=handle->ID);

    int order = (info->write_order)%1024;
    int locate= (info->write_locate);
    if(locate+size>handle->filesize)
    {
        int i;for(i=info->read_order;i<info->write_order;i++)
        {
            int  order0 =i%1024;
            int locate0 =info->message[order0].locate;
            int   size0 =info->message[order0].size;
            if(locate0+size0<=locate) break;
        }
        info->read_order=i;
        locate=0;
    }

    if(info->read_order>0)
    {
        int i;for(i=info->read_order;i<info->write_order;i++)
        {
            int  order0 =i%1024;
            int locate0 =info->message[order0].locate;
            int   size0 =info->message[order0].size;
            if((locate0+size0<=locate)||(locate0>=locate+size)) break;
        }
        info->read_order=i;
    }
    
    info->message[order].size =write_size;
    info->message[order].locate = locate;
    strcpy(info->message[order].dst,dstname);
    memcpy(info->ptr+locate,data,write_size);

    info->write_locate=locate+size;
    info->write_order+=1;
    
    if(info->writer_num>1){ID=0;m_Write(handle->file,0,&ID,sizeof(int));info->ID=0;}
    return (info->ptr+locate);
}

void *m_ProcMessageRead(const char *dstname,void *data,int *read_size)
{
    uint64_t t0=0;
    mException((dstname==NULL),EXIT,"invalid input");
    MHandle *hdl = mHandle("ProcMessage",ProcMessage);
    struct HandleProcMessage *handle = (struct HandleProcMessage *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->wait_time = DFLT;
        mPropertyVariate("ProcMessage","wait_time",&(handle->wait_time));
        if(handle->wait_time>=0) t0=mTime();
        
        mPropertyFunction("ProcMessage","exit",mornObjectRemove,"ProcMessage");
        
        if(morn_message_file[0]==0)
        {
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(morn_message_file,"%s/morn_message",getenv("TEMP"));
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file);
            #else
            sprintf(morn_message_file,"/tmp/morn_message");
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file,0777);
            #endif
        }
        sprintf(handle->filename,"%s/.morn_message.bin",morn_message_file);
        
        int flag,ID;
        do{flag = access(handle->filename,F_OK);WAIT}while(flag<0);
        handle->file = m_Open(handle->filename);
        int f_size;do{f_size = m_Fsize(handle->file);WAIT}while(f_size<sizeof(struct ProcTopicInfo));
        do{do{m_Read(handle->file,0,&ID,sizeof(int));WAIT}while(ID==0);flag=m_Exist(ID);}while(!flag);
        handle->filesize=m_Fsize(handle->file)-sizeof(struct ProcMessageInfo);
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcMessageInfo));
        
        m_Lock(handle->file);
        int user_num;m_Read(handle->file,sizeof(int),&user_num,sizeof(int));
        user_num++; m_Write(handle->file,sizeof(int),&user_num,sizeof(int));
        handle->info->user_num =user_num;
        m_Unlock(handle->file);
        
        handle->order_read = (handle->info->write_order-1);
        t0=0;
        hdl->valid = 1;
    }
    struct ProcMessageInfo *info=handle->info;
    
    handle->order_read = MAX(handle->order_read,MAX(info->read_order,info->write_order-1024));
    // printf("read handle->order_read=%I64d\n",handle->order_read);
    // printf("read info->write_order =%I64d\n",info->write_order );
    // printf("read info->read_order =%d\n",info->read_order );
    // printf("handle->wait_time=%d\n",handle->wait_time);
    
    int order=0;
    int i;for(i=handle->order_read;;i++)
    {
        // printf("i=%d,info->write_order=%d\n",i,info->write_order);
        while(i>=info->write_order) 
        {
            if(handle->wait_time>=0)
            {
                if(t0==0) t0=mTime();
                else if(mTime()-t0>=handle->wait_time) 
                    {handle->order_read=info->write_order;return NULL;}
            }
        }
        
        order = i%1024;
        if(strcmp(dstname,info->message[order].dst)==0) break;
    }
    
    handle->order_read=i+1;
    
    int locate= info->message[order].locate;
    int size  = info->message[order].size;

    if(read_size!=NULL) {if(*read_size>0) {size=MIN(*read_size,size);} *read_size=size;}
    if(data!=NULL) memcpy(data,info->ptr+locate,size);
    
    return (info->ptr+locate);
}
*/


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

        if(morn_message_file[0]==0)
        {
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(morn_message_file,"%s/morn_message",getenv("TEMP"));
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file);
            #else
            sprintf(morn_message_file,"/tmp/morn_message");
            if(access(morn_message_file,F_OK)<0) mkdir(morn_message_file,0777);
            #endif
        }
        
        sprintf(handle->idxname,"%s/.morn_variate_idx.bin",morn_message_file);
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
            n=12;m_Write(handle->file_idx,0    ,&n,sizeof(int32_t));
            n= 0;m_Write(handle->file_idx,4    ,&n,sizeof(int32_t));
            n= 1;m_Write(handle->file_idx,8    ,&n,sizeof(int32_t));
        }
        m_Mmap(handle->file_idx,handle->buff_idx,65536);

        sprintf(handle->dataname,"%s/.morn_variate_data.bin",morn_message_file);
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
    size_data += valuesize;
    
    p_idx[0] =handle->size_idx;
    *((int *)(handle->master_data)) = size_data;

    m_Unlock(handle->file_data);
    m_Unlock(handle->file_idx);
    p_idx[1]=0;

    mMapWrite(handle->map,p+4,DFLT,&offset,sizeof(uint16_t));
    return pvalue;
}

