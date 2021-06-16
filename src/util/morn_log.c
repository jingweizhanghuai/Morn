/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

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

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

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

static const char *morn_log_levelname[5]={"Debug","Info","Warning","Error","\0"};
const char *mLogLevel()
{
    if(morn_log_level%16!=0) return morn_log_levelname[4];
    int n=morn_log_level/16;
    if((n<0)||(n>3)) return morn_log_levelname[4];
    return morn_log_levelname[n];
}
