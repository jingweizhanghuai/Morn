#include "morn_util.h"

#if defined(_WIN64)||defined(_WIN32)
#include <Windows.h>
#define m_Exist(ID) (GetProcessVersion(ID/1000)!=0)
#define m_Open(Filename) CreateFile(Filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,0)
#define m_Close(File) CloseHandle(File)
#define m_Fsize(File) GetFileSize(File,NULL)
#define m_Read(File,Locate,Pointer,Size) do{\
    mException((SetFilePointer(File,Locate,NULL,FILE_BEGIN)!=Locate),EXIT,"error with file SetFilePointer");\
    mException((ReadFile(File,Pointer,Size,NULL,NULL)==0),EXIT,"error with ReadFile");\
}while(0)
#define m_Write(File,Locate,Pointer,Size) do{\
    mException((SetFilePointer(File,Locate,NULL,FILE_BEGIN)!=Locate),EXIT,"error with file SetFilePointer");\
    mException((WriteFile(File,Pointer,Size,NULL,NULL)==0),EXIT,"error with WriteFile");\
}while(0)
#define m_Lock(File)   LockFile(File,0,0,2*sizeof(int),0)
#define m_Unlock(File) UnlockFile(File,0,0,2*sizeof(int),0)
#else
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <signal.h>
#define m_Exist(ID) (kill((ID/1000),0)==0)
#define m_Open(Filename) open(Filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
#define m_Close(File) close(File)
#define m_Fsize(File) lseek(File,1,SEEK_END)
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
#endif

char morn_proc_mutex_file[256]={0};

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
void mProcLockBegin(const char *mutexname)
{
    if(mutexname==NULL) mutexname="morn_procmutex";
    MHandle *hdl = mHandle(mMornObject((void *)mutexname,DFLT),ProcLock);
    struct HandleProcLock *handle = (struct HandleProcLock *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->ID=getpid()*1000+mThreadID();
        if(morn_proc_mutex_file[0]==0)
        {
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(morn_proc_mutex_file,"%s/morn_process",getenv("TEMP"));
            if(access(morn_proc_mutex_file,F_OK)<0) mkdir(morn_proc_mutex_file);
            #else
            sprintf(morn_proc_mutex_file,"/tmp/morn_process");
            if(access(morn_proc_mutex_file,F_OK)<0) mkdir(morn_proc_mutex_file,0777);
            #endif
        }
        sprintf(handle->filename,"%s/.%s.bin",morn_proc_mutex_file,mutexname);
        
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

void mProcLockEnd(const char *mutexname)
{
    if(mutexname==NULL) mutexname="morn_process";
    MHandle *hdl = mHandle(mMornObject((void *)mutexname,DFLT),ProcLock);
    struct HandleProcLock *handle = (struct HandleProcLock *)(hdl->handle);
    mException((hdl->valid==0),EXIT,"no process mutex named %s",mutexname);
    mException(handle->state!=1,EXIT,"unlock error");
    int ID=0;
    // m_Lock(handle->file);
    m_Write(handle->file,0,&ID,sizeof(int));
    // m_Unlock(handle->file);
    handle->state=0;
}

/*
struct HandleMonitor
{
    int order;
    // int ID_num;
    // int ID[1024];
    // int state[1024];
    // char discribe[1024][16];
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file;
    #else
    int file;
    #endif
    char filename[256];
};



void mProcMonitorBegin(char *discribeinfo)
{
    MHandle *hdl = mHandle(mMornObject((void *)discribeinfo,DFLT),ProcMonitor);
    struct HandleProcMonitor *handle = (struct HandleProcMonitor *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->ID=getpid()*1000+mThreadID();
        if(morn_proc_mutex_file[0]==0)
        {
            #if defined(_WIN64)||defined(_WIN32)
            sprintf(morn_proc_mutex_file,"%s/morn_process",getenv("TEMP"));
            if(access(morn_proc_mutex_file,F_OK)<0) mkdir(morn_proc_mutex_file);
            #else
            sprintf(morn_proc_mutex_file,"/tmp/morn_process");
            if(access(morn_proc_mutex_file,F_OK)<0) mkdir(morn_proc_mutex_file,0777);
            #endif
        }
        sprintf(handle->filename,"%s/.%s.bin",morn_proc_mutex_file,mutexname);
        
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
        m_Unlock(handle->file);

        handle->state=0;
        
        hdl->valid = 1;
    }
}

*/