#include "morn_util.h"

#if defined(_WIN64)||defined(_WIN32)
#include <Windows.h>
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
#define m_Munmap(Pointer,Size) UnmapViewOfFile(Pointer);
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <signal.h>
#define m_Exist(ID) (kill((ID/1000),0)==0)
#define m_Open(Filename) open(Filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
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
#define m_Munmap(Pointer,Size) munmap(Pointer,Size);
#endif

char morn_message_file[256]={0};

struct ProcTopicState
{
    int locate;
    int size;
};
struct ProcTopicInfo
{
    volatile int ID;
    volatile int user_num;
    volatile int writer_num;
    volatile int write_locate;
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
    
    struct ProcTopicInfo *info;
    int64_t order_read;
};
void endProcTopic(struct HandleProcTopic *handle)
{
    if(handle->info==NULL) return;
    struct ProcTopicInfo *info = handle->info;

    m_Lock(handle->file);
    int user_num;m_Read(handle->file,sizeof(int),&user_num,sizeof(int));
    user_num--; m_Write(handle->file,sizeof(int),&user_num,sizeof(int));
    info->user_num =user_num;
    m_Unlock(handle->file);

    m_Munmap(info,handle->filesize+sizeof(struct ProcTopicInfo));
    
    m_Close(handle->file);
    if(user_num==0) {mSleep(10);remove(handle->filename);}//printf("bba\n");}
}
#define HASH_ProcTopic 0x72f379ee
void *mProcTopicWrite(const char *msgname,void *data,int write_size)
{
    if(msgname==NULL) msgname="morn_message";
    mException(data==NULL,EXIT,"invalid input");
    
    if(write_size<0) write_size=strlen(data)+1;
    int size = ((write_size+7)>>3)<<3;

    int ID=0;int user_num=0;int writer_num=0;

    MHandle *hdl = mHandle(mMornObject((void *)msgname,DFLT),ProcTopic);
    struct HandleProcTopic *handle = (struct HandleProcTopic *)(hdl->handle);
    if(hdl->valid == 0)
    {
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
        sprintf(handle->filename,"%s/.%s.bin",morn_message_file,msgname);

        int flag = access(handle->filename,F_OK);
        handle->file = m_Open(handle->filename);
        if(flag>=0)
        {
            m_Read(handle->file,sizeof(int),&ID,sizeof(int));
            if(!m_Exist(ID)) {flag=-1;}
            else handle->filesize = m_Fsize(handle->file)-sizeof(struct ProcTopicInfo);
        }
        if(flag<0) 
        {
            handle->filesize = MAX(size,1024)*256;
            m_Write(handle->file,handle->filesize+sizeof(struct ProcTopicInfo),&ID,sizeof(int));
        }
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcTopicInfo));
        
        if(flag<0)
        {
            handle->info->write_locate= 0;
            handle->info->write_order = 0;
            int size = handle->filesize/128;
            for(int i=0;i<128;i++)
            {
                handle->info->topic[i].locate= size*i;
                handle->info->topic[i].size  = size;
            }
        }
        
        m_Lock(handle->file);
        if(flag>=0)
        {
            m_Read(handle->file,2*sizeof(int),  &user_num,sizeof(int));
            m_Read(handle->file,3*sizeof(int),&writer_num,sizeof(int));
        }
        if(writer_num==0) ID=handle->ID;else ID=0;
        // printf("writer_num=%d,ID=%d\n",writer_num,ID);
                     m_Write(handle->file,            0,         &ID ,sizeof(int));handle->info->ID        = ID;
        user_num++;  m_Write(handle->file,  sizeof(int),    &user_num,sizeof(int));handle->info->user_num  = user_num;
        writer_num++;m_Write(handle->file,2*sizeof(int),  &writer_num,sizeof(int));handle->info->writer_num= writer_num;
        m_Unlock(handle->file);
 
        handle->order_read = 0;
        hdl->valid = 1;
    }
    struct ProcTopicInfo *info=handle->info;
    if(size>handle->filesize/8)
    {
        endProcTopic(handle);
        handle->info=NULL;
        mSleep(10);
        remove(handle->filename);
        mException(1,EXIT,"invalid topic size");
    }
    
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

    int order = (info->write_order)%128;
    int locate= (info->write_locate);if(locate+size>handle->filesize)locate=0;
    
    info->topic[order].size =write_size;
    info->topic[order].locate = locate;
    memcpy(info->ptr+locate,data,write_size);

    handle->order_read = info->write_order;
    info->write_locate=locate+size;
    info->write_order+=1;
    
    if(info->writer_num>1){ID=0;m_Write(handle->file,0,&ID,sizeof(int));info->ID=0;}
    return (info->ptr+locate);
}

void *mProcTopicRead(const char *msgname,void *data,int *read_size)
{
    if(msgname==NULL) msgname="morn_message";
    MHandle *hdl = mHandle(mMornObject((void *)msgname,DFLT),ProcTopic);
    struct HandleProcTopic *handle = (struct HandleProcTopic *)(hdl->handle);
    if(hdl->valid == 0)
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
        sprintf(handle->filename,"%s/.%s.bin",morn_message_file,msgname);
        // printf("filename=%s\n",handle->filename);
        
        int flag,ID;
        do{flag = access(handle->filename,F_OK);}while(flag<0);
        // printf("flag=%d\n",flag);
        handle->file = m_Open(handle->filename);
        int fsize;do{fsize = m_Fsize(handle->file);}while(fsize<sizeof(struct ProcTopicInfo));
        // printf("fsize=%d\n",fsize);
        do{do{m_Read(handle->file,0,&ID,sizeof(int));}while(ID==0);flag=m_Exist(ID);}while(!flag);
        int filesize=m_Fsize(handle->file);
        m_Mmap(handle->file,handle->info,filesize);
        
        m_Lock(handle->file);
        int user_num;m_Read(handle->file,sizeof(int),&user_num,sizeof(int));
        user_num++; m_Write(handle->file,sizeof(int),&user_num,sizeof(int));
        handle->info->user_num =user_num;
        m_Unlock(handle->file);
        
        handle->order_read = 0;//handle->info->write_order;
        hdl->valid = 1;
    }
    struct ProcTopicInfo *info=handle->info;

    while(info->write_order<=handle->order_read);
    handle->order_read=info->write_order-1;
    int order = handle->order_read%128;
    handle->order_read+=1;
    
    int locate= info->topic[order].locate;
    int size  = info->topic[order].size;

    if(read_size!=NULL) {if(*read_size>0) {size=MIN(*read_size,size);} *read_size=size;}
    if(data!=NULL) memcpy(data,info->ptr+locate,size);
    return (info->ptr+locate);
}

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
    
    struct ProcMessageState message[1024];
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
    if(user_num==0) {mSleep(10);remove(handle->filename);}//printf("bba\n");}
}
#define HASH_ProcMessage 0xbc2aef80
void *mProcMessageWrite(const char *dstname,void *data,int write_size)
{
    mException((dstname==NULL)||(data==NULL),EXIT,"invalid input");
    
    if(write_size<0) write_size=strlen(data)+1;
    int size = ((write_size+7)>>3)<<3;

    int ID=0;int user_num=0;int writer_num=0;

    MHandle *hdl = mHandle(mMornObject(NULL,DFLT),ProcMessage);
    struct HandleProcMessage *handle = (struct HandleProcMessage *)(hdl->handle);
    if(hdl->valid == 0)
    {
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
        sprintf(handle->filename,"%s/.morn_message.bin",morn_message_file);

        int flag = access(handle->filename,F_OK);
        handle->file = m_Open(handle->filename);
        if(flag>=0)
        {
            m_Read(handle->file,sizeof(int),&ID,sizeof(int));
            if(!m_Exist(ID)) {flag=-1;}
            else handle->filesize = m_Fsize(handle->file)-sizeof(struct ProcMessageInfo);
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
        // printf("writer_num=%d,ID=%d\n",writer_num,ID);
                     m_Write(handle->file,            0,         &ID ,sizeof(int));handle->info->ID        = ID;
        user_num++;  m_Write(handle->file,  sizeof(int),    &user_num,sizeof(int));handle->info->user_num  = user_num;
        writer_num++;m_Write(handle->file,2*sizeof(int),  &writer_num,sizeof(int));handle->info->writer_num= writer_num;
        m_Unlock(handle->file);
 
        handle->order_read = MAX(handle->info->write_order,0);
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

void *mProcMessageRead(const char *dstname,void *data,int *read_size)
{
    mException((dstname==NULL),EXIT,"invalid input");
    MHandle *hdl = mHandle(mMornObject(NULL,DFLT),ProcMessage);
    struct HandleProcMessage *handle = (struct HandleProcMessage *)(hdl->handle);
    if(hdl->valid == 0)
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
        sprintf(handle->filename,"%s/.morn_message.bin",morn_message_file);
        // printf("filename=%s\n",handle->filename);
        
        int flag,ID;
        do{flag = access(handle->filename,F_OK);}while(flag<0);
        // printf("flag=%d\n",flag);
        handle->file = m_Open(handle->filename);
        int fsize;do{fsize = m_Fsize(handle->file);}while(fsize<sizeof(struct ProcTopicInfo));
        // printf("fsize=%d\n",fsize);
        do{do{m_Read(handle->file,0,&ID,sizeof(int));}while(ID==0);flag=m_Exist(ID);}while(!flag);
        handle->filesize=m_Fsize(handle->file)-sizeof(struct ProcMessageInfo);
        m_Mmap(handle->file,handle->info,handle->filesize+sizeof(struct ProcMessageInfo));
        
        m_Lock(handle->file);
        int user_num;m_Read(handle->file,sizeof(int),&user_num,sizeof(int));
        user_num++; m_Write(handle->file,sizeof(int),&user_num,sizeof(int));
        handle->info->user_num =user_num;
        m_Unlock(handle->file);
        
        handle->order_read = (handle->info->write_order-1);
        hdl->valid = 1;
    }
    struct ProcMessageInfo *info=handle->info;

    handle->order_read = MAX(handle->order_read,MAX(info->read_order,info->write_order-1024));
    // printf("read handle->order_read=%d\n",handle->order_read);
    // printf("read info->write_order =%d\n",info->write_order );
    // printf("read info->read_order =%d\n",info->read_order );
    
    int order;
    int i;for(i=handle->order_read;;i++)
    {
        while(i>=info->write_order);
        order = i%1024;
        if(strcmp(dstname,info->message[i].dst)==0) break;
    }
    handle->order_read=i+1;
    
    int locate= info->message[order].locate;
    int size  = info->message[order].size;

    if(read_size!=NULL) {if(*read_size>0) {size=MIN(*read_size,size);} *read_size=size;}
    if(data!=NULL) memcpy(data,info->ptr+locate,size);
    
    return (info->ptr+locate);
}

