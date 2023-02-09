/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_ptc.h"

#if defined(_WIN64)||defined(_WIN32)
#include <Windows.h>
#include <io.h>
#define access _access
#define FHandle HANDLE
#define F_OK 0
#define m_Open(Filename) CreateFile(Filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,0)
#define m_Close(File) CloseHandle(File)
#define m_Write(File,Locate,Pointer,Size) do{int A;\
    do{A=SetFilePointer(File,Locate,NULL,FILE_BEGIN);}while(A!=Locate);\
    do{A=WriteFile(File,Pointer,Size,NULL,NULL);}while(A==0);\
}while(0)
#define m_Fsize(File) SetFilePointer(File,0,NULL,FILE_END)
#define m_Lock(File)   LockFile(File,0,0,2*sizeof(int),0)
#define m_Unlock(File) UnlockFile(File,0,0,2*sizeof(int),0)
#define m_Mmap(File,Pointer,Size) do{\
    HANDLE Map = CreateFileMapping(File,NULL,PAGE_READWRITE,0,Size,NULL);\
    Pointer = MapViewOfFile(Map,FILE_MAP_ALL_ACCESS,0,0,Size);\
    CloseHandle(Map);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Munmap(Pointer,Size) UnmapViewOfFile((void *)(Pointer));
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <signal.h>
#define FHandle int
#define m_Open(Filename) open(Filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
#define m_Close(File) close(File)
#define m_Write(File,Locate,Pointer,Size) do{\
    mException((lseek(File,Locate,SEEK_SET)!=Locate),EXIT,"error with file lseek");\
    mException((write(File,Pointer,Size)!=Size),EXIT,"error with file write");\
}while(0)
#define m_Fsize(File) lseek(File,0,SEEK_END)
#define m_Lock(File)   flock(File,LOCK_EX)
#define m_Unlock(File) flock(File,LOCK_UN)
#define m_Mmap(File,Pointer,Size) do{\
    Pointer=mmap(NULL,(Size),PROT_READ|PROT_WRITE,MAP_SHARED,File,0);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Munmap(Pointer,Size) munmap((void *)(Pointer),Size);
#endif

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

__thread int morn_log_level=1;
int morn_log_levelset=-1;

struct HandleLog
{
    int file_valid;
    char filename[128];
    char fileprefix[128];
    char filetype[16];
    int fileorder;
    int filerotate;
    int filebyte;
    MThreadSignal sgn;
    
    FHandle file;
    int filesize;
    int writesize;
    char *filepointer;

    int console_valid0;
    int console_valid;

    int func_valid;
    void (*func)(void *,int,void *);
    void *func_para;
};
void endLog(struct HandleLog *handle)
{
    if(handle->file_valid)
    {
        memset((void *)(handle->filepointer+handle->writesize),' ',handle->filesize-handle->writesize-1);
        m_Munmap(handle->filepointer,handle->filesize);
        m_Close(handle->file);
    }
}
#define HASH_Log 0x3f37e6f1

void LogFile(char *filename)
{
    if(filename==NULL) return;
    MHandle *hdl = mHandle("Log",Log);
    struct HandleLog *handle = hdl->handle;
    
    if(strcmp(filename,"exit")==0) 
    {
        if(handle->file_valid)
        {
            memset((void *)(handle->filepointer+handle->writesize),' ',handle->filesize-handle->writesize-1);
            m_Munmap(handle->filepointer,handle->filesize);
            m_Close(handle->file);
        }
    
        handle->fileprefix[0]=0;
        handle->file_valid=0;
        handle->console_valid = handle->console_valid0||(!(handle->func_valid));
        return;
    }
        
    if(handle->filebyte>0)
    {
        char name0[128];char name1[128];
        if(strcmp(handle->filename,filename)!=0)
        {
            endLog(handle);
            strcpy(handle->filename,filename);
            handle->fileorder=0;
            handle->fileprefix[0]=0;
        }
        else if(handle->filesize-handle->writesize>=1024) return;
        else endLog(handle);
        
        if(handle->fileprefix[0]==0)
        {
            int len = strlen(filename);
            memcpy(handle->fileprefix,filename,len+1);
            for(int j=len;j>0;j--)
            {
                if(handle->fileprefix[j]=='.')
                {
                    strcpy(handle->filetype,handle->fileprefix+j+1);
                    handle->fileprefix[j]=0;
                    break;
                }
            }
        }
        
        if(handle->fileorder==0)
        {
            if(access(handle->filename,F_OK)>=0) remove(handle->filename);
            handle->fileorder++;
        }
        else if(handle->fileorder<=handle->filerotate)
        {
            snprintf(name0,128,"%s%d.%s",handle->fileprefix,handle->fileorder,handle->filetype);
            if(access(name0,F_OK)>=0) remove(name0);
            rename(handle->filename,name0);
            handle->fileorder++;
        }
        else
        {
            snprintf(name0,128,"%s1.%s",handle->fileprefix,handle->filetype);
            remove(name0);
            int i=1;for(;i<handle->filerotate-1;i+=2)
            {
                snprintf(name1,128,"%s%d.%s",handle->fileprefix,i+1,handle->filetype);
                rename(name1,name0);
                snprintf(name0,128,"%s%d.%s",handle->fileprefix,i+2,handle->filetype);
                rename(name0,name1);
            }
            if(i<handle->filerotate)
            {
                snprintf(name1,128,"%s%d.%s",handle->fileprefix,i+1,handle->filetype);
                rename(name1,name0);
                strcpy(name0,name1);
            }
            rename(handle->filename,name0);
        }
        handle->file=m_Open(handle->filename);
        handle->writesize= 0;
        handle->filesize = 0;
    }
    else
    {
        if(strcmp(handle->filename,filename)!=0)
        {
            endLog(handle);
            strcpy(handle->filename,filename);
            if(access(handle->filename,F_OK)>=0) remove(handle->filename);
            handle->file=m_Open(handle->filename);
            handle->writesize= 0;
            handle->filesize = 0;
        }
        else if(handle->filesize-handle->writesize>=1024) return;
    }
    
    int byte=handle->filebyte;if(byte==0) byte=64*1024;
    int filesize = handle->filesize+byte;
    char a=' ';m_Write(handle->file,filesize-1,&a,1);

//     char *filepointer0=handle->filepointer;
    m_Mmap(handle->file,handle->filepointer,filesize);
//     if(filepointer0) m_Munmap(filepointer0,handle->filesize);
    
    handle->filesize=filesize;
    
    handle->file_valid = 1;
    handle->console_valid = handle->console_valid0;
}

void LogFunction(void **function)
{
    MHandle *hdl = mHandle("Log",Log);
    struct HandleLog *handle = hdl->handle;
    if(function==NULL) {handle->func_valid=0;handle->console_valid = handle->console_valid0||(!(handle->file_valid));return;}
    void *func = *function;
    if(func==NULL) {handle->func_valid=0;handle->console_valid = handle->console_valid0||(!(handle->file_valid));return;}

    handle->func = func;
    handle->func_valid = 1;
    handle->console_valid = handle->console_valid0;
}

void LogConsole(int *valid)
{
    MHandle *hdl = mHandle("Log",Log);
    struct HandleLog *handle = hdl->handle;
    handle->console_valid0= *valid;
    handle->console_valid = handle->console_valid0||(!((handle->file_valid)||(handle->func_valid)));
    if(handle->console_valid!=handle->console_valid0)
        mLog(MORN_WARNING,mLogFormat1("connot disable log_console"));
}

struct HandleLog *morn_log_handle = NULL;
__thread char morn_log_string[1024];

struct HandleLog *LogInit()
{
    if(morn_log_handle!=NULL) return morn_log_handle;
    MHandle *hdl = mHandle("Log",Log);
    struct HandleLog *handle = hdl->handle;
    if(!mHandleValid(hdl))
    {
        mPropertyFunction("Log","exit"          ,mornObjectRemove,"Log");
        mPropertyVariate( "Log","log_level"     ,&morn_log_levelset,sizeof(int));
                                                
        mPropertyVariate( "Log","log_filesize"  ,&handle->filebyte,sizeof(int));
        mPropertyVariate( "Log","log_filerotate",&handle->filerotate,sizeof(int));
        mPropertyFunction("Log","log_file"      ,LogFile);
        
        mPropertyFunction("Log","log_function"  ,LogFunction);
        mPropertyVariate( "Log","log_func_para" ,&handle->func_para,sizeof(void *));
                                                
        mPropertyFunction("Log","log_console"   ,LogConsole);
        
        handle->console_valid = handle->console_valid0||(!((handle->file_valid)||(handle->func_valid)));
        if(handle->filerotate==0) handle->filerotate=0x7fffffff;
        hdl->valid=1;
    }
    morn_log_handle = handle;
    return handle;
}

volatile int morn_file_writable=1;
void m_Log(int level,const char *format,...)
{
    struct HandleLog *handle = LogInit();
    
    int n=0;
    va_list args;va_start(args,format);
    n=vsnprintf(morn_log_string,1024,format,args);
    va_end(args);
    
    if(handle->console_valid) printf(morn_log_string);
    if(handle->file_valid)
    {
        mThreadLockBegin(handle->sgn);
        if(handle->filesize-handle->writesize<=n) LogFile(handle->filename);
        int l=handle->writesize;handle->writesize+=n;
        memcpy((void *)(handle->filepointer+l),morn_log_string,n);
        mThreadLockEnd(handle->sgn);
    }
    if(handle->func_valid)handle->func(morn_log_string,n,handle->func_para);
}

static const char *morn_log_levelname[6]={"\0","Debug","Info","Warning","Error","\0"};
const char *mLogLevel()
{
    if(morn_log_level%16!=0) return morn_log_levelname[5];
    int n=morn_log_level/16;
    if((n<0)||(n>4)) return morn_log_levelname[5];
    return morn_log_levelname[n];
}

void LogTail(const char *filename)
{
    int locate,size,cnt;
    char *filepointer;
    
    LogTail_begin:
    locate=-1;
    while(1){if(access(filename,F_OK)>=0) break; locate=0;mSleep(10);}
    FHandle file=m_Open(filename);
    size=m_Fsize(file);
    filepointer=NULL;
    m_Mmap(file,filepointer,size);
    if(locate<0) {for(int i=0;i<size;i++) {if(filepointer[i]==0) {locate=i;break;}}}
    if(locate<0) locate=size;
    
    cnt=0;
    while(1)
    {
        mSleep(10);
        if(filepointer!=NULL) if(filepointer[locate]==' ') locate=size;
        if((access(filename,F_OK)<0)||(locate>=size))
        {
            if(filepointer!=NULL) {m_Munmap(filepointer,size);m_Close(file);filepointer=NULL;}
            locate=0;continue;
        }
        
        if(filepointer==NULL)
        {
            file=m_Open(filename);
            size=m_Fsize(file);
            m_Mmap(file,filepointer,size);
        }
        
        char *p=filepointer+locate;
        if(p[0])
        {
            cnt=0;
            int s=strlen(p);
            locate+=s;
            if(s==size) continue;
            printf("%s",p);
        }
        else
        {
            cnt++;if(cnt<100) continue;
            m_Munmap(filepointer,size);m_Close(file);
            cnt=0;goto LogTail_begin;
        }
    }
}




