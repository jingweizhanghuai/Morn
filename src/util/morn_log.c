/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
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
    char filetype[16];
    int fileorder;
    int filebyte;
    
    FHandle file;
    int filesize;
    int writesize;
    char *filepointer;
    volatile int file_flag;

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
    
    int v=mAtomicCompare(&(handle->file_flag),0,1);
    if(v==0) while(1) {if(handle->file_flag==0) return;}
    
    if(strcmp(filename,"exit")==0) 
    {
        if(handle->file_valid)
        {
            memset((void *)(handle->filepointer+handle->writesize),' ',handle->filesize-handle->writesize-1);
            m_Munmap(handle->filepointer,handle->filesize);
            m_Close(handle->file);
        }
    
        handle->filename[0]=0;
        handle->file_valid=0;
        handle->console_valid = handle->console_valid0||(!(handle->func_valid));
        handle->file_flag=0;
        return;
    }
    
    if(strcmp(handle->filename,filename)!=0)
    {
        if(handle->file_valid) m_Close(handle->file);
        
        if(handle->filebyte>0)
        {
            if(handle->filename[0]==0)
            {
                int len = strlen(filename);
                for(int j=len;j>0;j--)
                {
                    if(handle->filename[j]=='.')
                    {
                        strcpy(handle->filetype,handle->filename+j);
                        handle->filename[j]=0;
                        break;
                    }
                }
            }
            char name[128];
            snprintf(name,128,"%s%d.%s",handle->filename,handle->fileorder,handle->filetype);
            handle->fileorder++;
            if(access(name,F_OK)>=0) remove(name);
            
            handle->file=m_Open(name);
        }
        else
        {
            strcpy(handle->filename,filename);
            if(access(handle->filename,F_OK)>=0) remove(handle->filename);
            handle->file=m_Open(handle->filename);
        }
        handle->writesize= 0;
        handle->filesize = 0;
    }

    if(handle->filesize-handle->writesize>=1024) {handle->file_flag=0; return;}
    
    int byte=handle->filebyte;if(byte==0) byte=64*1024;
    int filesize = handle->filesize+byte;
    char a=0;m_Write(handle->file,filesize-1,&a,1);

//     char *filepointer0=handle->filepointer;
    m_Mmap(handle->file,handle->filepointer,filesize);
//     if(filepointer0) m_Munmap(filepointer0,handle->filesize);
    
    handle->filesize=filesize;
    
    handle->file_valid = 1;
    handle->console_valid = handle->console_valid0;
    handle->file_flag=0;
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
struct HandleLog *LogInit()
{
    if(morn_log_handle !=NULL) return morn_log_handle;
    MHandle *hdl = mHandle("Log",Log);
    struct HandleLog *handle = hdl->handle;
    if(!mHandleValid(hdl))
    {
        mPropertyFunction("Log","exit"         ,mornObjectRemove,"Log");
        mPropertyVariate( "Log","log_level"    ,&morn_log_levelset,sizeof(int));

        mPropertyVariate( "Log","log_filesize" ,&handle->filebyte,sizeof(int));
        mPropertyFunction("Log","log_file"     ,LogFile);
        
        mPropertyFunction("Log","log_function" ,LogFunction);
        mPropertyVariate( "Log","log_func_para",&handle->func_para,sizeof(void *));
        
        mPropertyFunction("Log","log_console"  ,LogConsole);
        
        handle->console_valid = handle->console_valid0||(!((handle->file_valid)||(handle->func_valid)));
       
        hdl->valid=1;
    }
    morn_log_handle = handle;
    return handle;
}

__thread char morn_log_string[1024];
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
        int l=mAtomicAdd(&(handle->writesize),n);
        int m=handle->filesize-l+n;
        if(m<=n) LogFile(handle->filename);
        memcpy((void *)(handle->filepointer+l-n),morn_log_string,n);
    }
    if(handle->func_valid)handle->func(morn_log_string,n,handle->func_para);
}

static const char *morn_log_levelname[6]={"\0","Debug","Info","Warning","Error","\0"};
const char *mLogLevel()
{
    if(morn_log_level%16!=0) return morn_log_levelname[5];
    int n=morn_log_level/16;
    if((n<0)||(n>3)) return morn_log_levelname[4];
    return morn_log_levelname[n];
}

void LogTail(const char *filename)
{
    int flag=-1; 
    while(flag<0){flag = access(filename,F_OK); mSleep(10);}

    FHandle file=m_Open(filename);
    int size = 0;
    int locate=0;
    char *filepointer=NULL;

    while(1)
    {
        mSleep(10);
        if(locate>=size)
        {
            if(filepointer!=NULL) m_Munmap(filepointer,size);
            size=m_Fsize(file);
            // printf("size=%d\n",size);
            m_Mmap(file,filepointer,size);
            if(locate==0) {for(int i=0;i<size;i++) {if(filepointer[i]==0) {locate=i;break;}}}
        }
        // printf("locate=%d,size=%d\n",locate,size);
        char *p=filepointer+locate;
        if(p[0])
        {
            int s=strlen(p);
            locate+=s;
            printf("%s",p);
        }
    }
}




