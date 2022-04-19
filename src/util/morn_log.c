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
#define m_Open(Filename) CreateFile(Filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,0)
#define m_Close(File) CloseHandle(File)
#define m_Write(File,Locate,Pointer,Size) do{int A;\
    do{A=SetFilePointer(File,Locate,NULL,FILE_BEGIN);}while(A!=Locate);\
    do{A=WriteFile(File,Pointer,Size,NULL,NULL);}while(A==0);\
}while(0)
#define m_Lock(File)   LockFile(File,0,0,2*sizeof(int),0)
#define m_Unlock(File) UnlockFile(File,0,0,2*sizeof(int),0)
#define m_Mmap(File,Locate,Pointer,Size) do{\
    HANDLE Map = CreateFileMapping(File,NULL,PAGE_READWRITE,0,Locate+Size,NULL);\
    Pointer = MapViewOfFile(Map,FILE_MAP_WRITE,0,0,0);\
    Pointer+=Locate;\
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
#define m_Open(Filename) open(Filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
#define m_Close(File) close(File)
#define m_Write(File,Locate,Pointer,Size) do{\
    mException((lseek(File,Locate,SEEK_SET)!=Locate),EXIT,"error with file lseek");\
    mException((write(File,Pointer,Size)!=Size),EXIT,"error with file write");\
}while(0)
#define m_Lock(File)   flock(File,LOCK_EX)
#define m_Unlock(File) flock(File,LOCK_UN)
#define m_Mmap(File,Locate,Pointer,Size) do{\
    Pointer=mmap(NULL,(Size),PROT_WRITE,MAP_SHARED,File,Locate);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Munmap(Pointer,Size) munmap(Pointer,Size);
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
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file;
    #else
    int file;
    #endif
    int filesize;
    int mapsize;
    int writesize;
    char *filepointer;
    int filecount;

    int console_valid0;
    int console_valid;

    int func_valid;
    char funcbuff[1024];
    void (*func)(void *,int,void *);
    void *func_para;
};
void endLog(struct HandleLog *handle)
{
    // int *proc_num = mProcVariate("log_proc",sizeof(int));
    // *proc_num--;

    if(handle->file_valid)
    {
        m_Munmap(handle->filepointer,handle->mapsize);
        m_Close(handle->file);
    }
}
#define HASH_Log 0x3f37e6f1
struct HandleLog *LogInit();
void LogFile(char *filename)
{
    if(filename==NULL) return;
    struct HandleLog *handle= LogInit();

    if(handle->file_valid) m_Munmap(handle->filepointer,handle->mapsize);
    if(strcmp(handle->filename,filename)!=0)
    {
        if(handle->file_valid) m_Close(handle->file);
        if(strcmp(filename,"exit")==0) {handle->filename[0]=0;handle->file_valid=0;handle->console_valid = handle->console_valid0||(!(handle->func_valid));return;}
        
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
            handle->file=m_Open(name);
        }
        else
        {
            strcpy(handle->filename,filename);
            // printf("handle->filename=%s\n",handle->filename);
            handle->file=m_Open(handle->filename);
        }
        handle->writesize= 0;
        handle->filesize = 0;
    }
    
    // printf("handle->writesize=%d\n",handle->writesize);
    int locate=handle->filesize-(handle->mapsize-handle->writesize);
    int byte=handle->filebyte;if(byte==0) byte=1024*1024;
    handle->filesize = handle->filesize+byte;
    handle->mapsize  = handle->filesize-locate;
    // printf("handle->filesize=%d,handle->mapsize=%d\n",handle->filesize,handle->mapsize);
    char a=0;m_Write(handle->file,handle->filesize,&a,1);
    m_Mmap(handle->file,locate,handle->filepointer,handle->mapsize);
    handle->writesize=0;
    
    handle->file_valid = 1;
    handle->console_valid = handle->console_valid0;
}

void LogFunction(void **function)
{
    struct HandleLog *handle = LogInit();
    if(function==NULL) {handle->func_valid=0;handle->console_valid = handle->console_valid0||(!(handle->file_valid));return;}
    void *func = *function;
    if(func==NULL) {handle->func_valid=0;handle->console_valid = handle->console_valid0||(!(handle->file_valid));return;}

    handle->func = func;
    handle->func_valid = 1;
    handle->console_valid = handle->console_valid0;
}

struct HandleLog *morn_log_handle = NULL;
struct HandleLog *LogInit()
{
    if(morn_log_handle !=NULL) return morn_log_handle;
    MHandle *hdl = mHandle("Log",Log);
    struct HandleLog *handle = hdl->handle;
    morn_log_handle = handle;
    if(hdl->valid==0)
    {
        mPropertyFunction("Log","exit"         ,mornObjectRemove,"Log");
        mPropertyVariate( "Log","log_level"    ,&morn_log_levelset);
        
        mPropertyVariate( "Log","log_filesize" ,&handle->filebyte);
        mPropertyFunction("Log","log_file"     ,LogFile);

        mPropertyFunction("Log","log_function" ,LogFunction);
        mPropertyVariate( "Log","log_func_para",&handle->func_para);
        
        mPropertyVariate( "Log","log_console"  ,&handle->console_valid0);
        handle->console_valid = handle->console_valid0||(!((handle->file_valid)||(handle->func_valid)));
        hdl->valid=1;
    }
    return handle;
}

void m_Log(int level,const char *format,...)
{
    struct HandleLog *handle = LogInit();
    
    va_list args; va_start(args,format);
    if(handle->console_valid){vprintf(format,args);}
    
    if(handle->file_valid)
    {
        int n=vsnprintf(handle->filepointer+handle->writesize,handle->mapsize-handle->writesize,format,args);
        if(n<0)
        {
            LogFile(handle->filename);
            n=vsnprintf(handle->filepointer,handle->mapsize,format,args);
        }
        handle->writesize+=n;
    }
    
    if(handle->func_valid)
    {
        int n = vsprintf(handle->funcbuff,format,args);
        handle->func(handle->funcbuff,n,handle->func_para);
    }
    
    va_end(args);
}








/*


struct HandleLog
{
    FILE *f;
    MThreadSignal sgn;
    int64_t filesize;
    int fileorder;
    char filename[256];
    char filetype[16];
    
    void (*func)(void *,int,void *);

    MList *buff_list;
};
#define HASH_Log 0x3f37e6f1
static struct HandleLog *morn_log_info=NULL;

__thread int morn_log_level=1;
int morn_log_levelset = -1;
static int64_t morn_log_filesize_set = 0x7FFFFFFFFFFFFFFF;
static void *morn_log_func_para = NULL;

static int morn_log_console_valid0= 0;
static int morn_log_console_valid = 1;
static int morn_log_file_valid    = 0;
static int morn_log_function_valid= 0;

__thread char *morn_log_buff=NULL;
__thread int morn_log_buff_size=65536;

void endLogFile()
{
    if(morn_log_info->f!=NULL)
    {
        int size = 65536-morn_log_buff_size;
        if(size>0) fwrite(morn_log_buff-65536,1,size,morn_log_info->f);
        fclose(morn_log_info->f);
        morn_log_buff_size=65536;
    }
}
void endLogFunction()
{
    if(morn_log_info->func!=NULL)
    {
        int size = 65536-morn_log_buff_size;
        if(size>0) (morn_log_info->func)(morn_log_buff-65536,size,morn_log_func_para);
        morn_log_buff_size=65536;
    }
}
void endLog(struct HandleLog *handle)
{
    mException(morn_log_info!=handle,EXIT,"invalid log set");
    endLogFile();
    endLogFunction();
    if(morn_log_info->buff_list==NULL) return;
    for(int i=0;i<morn_log_info->buff_list->num;i++)
    {
        char **buff = (char **)morn_log_info->buff_list->data[i];
        free((*buff)-65536);
    }
    mListRelease(morn_log_info->buff_list);
}

void LogInfoInit()
{
    MHandle *hdl = mHandle(mMornObject("Log",DFLT),Log);
    morn_log_info = (struct HandleLog *)(hdl->handle);
    if(morn_log_info->buff_list==NULL) morn_log_info->buff_list = mListCreate();
    hdl->valid=1;
}

void LogFile(char *filename)
{
    if(filename==NULL) return;
    
    if(morn_log_info==NULL) LogInfoInit();
    
    if(strcmp(morn_log_info->filename,filename)==0) return;
    strcpy(morn_log_info->filename,filename);
    
    endLogFile();
    if(strcmp(filename,"exit")==0)
    {
        morn_log_file_valid    = 0;
        morn_log_console_valid = morn_log_console_valid0||(!morn_log_function_valid);
        return;
    }
    
    morn_log_info->f = fopen(morn_log_info->filename,"wb");
    
    morn_log_info->filetype[0]=0;
    int len = strlen(morn_log_info->filename);
    for(int j=len;j>0;j--)
    {
        if(morn_log_info->filename[j]=='.')
        {
            strcpy(morn_log_info->filetype,morn_log_info->filename+j);
            morn_log_info->filename[j]=0;
            break;
        }
    }
    morn_log_info->filesize = 0;
    morn_log_info->fileorder= 0;
    
    morn_log_file_valid    = 1;
    morn_log_console_valid = morn_log_console_valid0;
}

void LogFunction(void **function)
{
    if(function==NULL) return;
    void *func = *function;
    if(func==NULL) return;
    
    if(morn_log_info==NULL) LogInfoInit();

    endLogFunction();

    morn_log_info->func = func;
    
    if(func==NULL)
    {
        morn_log_function_valid = 0;
        morn_log_console_valid  = morn_log_console_valid0||(!morn_log_file_valid);
    }
    else
    {
        morn_log_function_valid = 1;
        morn_log_console_valid  = morn_log_console_valid0;
    }
}

void LogConsole(int *valid)
{
    morn_log_console_valid0= *valid;
    morn_log_console_valid = *valid;
}



static int morn_log_init = 0;
void _mLog(int level,const char *format,...)
{
    if(morn_log_init==0)
    {
        mPropertyVariate( "Log","log_level"    ,&morn_log_levelset);
        mPropertyVariate( "Log","log_filesize" ,&morn_log_filesize_set);
        mPropertyVariate( "Log","log_func_para",&morn_log_func_para);
        mPropertyFunction("Log","log_console"  ,LogConsole);
        mPropertyFunction("Log","log_file"     ,LogFile);
        mPropertyFunction("Log","log_function" ,LogFunction);
        if(morn_log_level<morn_log_levelset) return;
        morn_log_init=1;
    }
    // printf("morn_log_levelset=%d\n\n",morn_log_levelset);
    
    va_list args; 
    if(morn_log_console_valid){va_start(args,format);vprintf(format,args);va_end(args);}
    if(morn_log_file_valid||morn_log_function_valid)
    {
        if(morn_log_buff == NULL)
        {
            morn_log_buff =  ((char *)malloc(65536))+65536;
            morn_log_buff_size=65536;
            mListWrite(morn_log_info->buff_list,DFLT,&morn_log_buff,sizeof(char *));
        }
        
        va_start(args,format);unsigned int n=vsnprintf(morn_log_buff-morn_log_buff_size,morn_log_buff_size,format,args);va_end(args);
        if(n>morn_log_buff_size)
        {
            mThreadLockBegin(morn_log_info->sgn);
            if(morn_log_file_valid)
            {
                fwrite(morn_log_buff-65536,1,65536-morn_log_buff_size,morn_log_info->f);
                morn_log_info->filesize += (65536-morn_log_buff_size);
                if(morn_log_info->filesize>=morn_log_filesize_set)
                {
                    fclose(morn_log_info->f);
                    morn_log_info->fileorder+=1;
                    snprintf(morn_filename,256,"%s_%d%s",morn_log_info->filename,morn_log_info->fileorder,morn_log_info->filetype);
                    morn_log_info->f=fopen(morn_filename,"wb");
                    morn_log_info->filesize=0;
                }
            }
            if(morn_log_function_valid)
            {
                (morn_log_info->func)(morn_log_buff-65536,65536-morn_log_buff_size,morn_log_func_para);
            }
            mThreadLockEnd(morn_log_info->sgn);
            
            morn_log_buff_size = 65536;
            va_start(args, format);n = vsnprintf(morn_log_buff-65536,65536,format,args);va_end(args);
        }
        morn_log_buff_size-=n;
    }
}

*/

static const char *morn_log_levelname[5]={"Debug","Info","Warning","Error","\0"};
const char *mLogLevel()
{
    if(morn_log_level%16!=0) return morn_log_levelname[4];
    int n=morn_log_level/16;
    if((n<1)||(n>4)) return morn_log_levelname[4];
    return morn_log_levelname[n-1];
}
