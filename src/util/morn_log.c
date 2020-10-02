/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

struct HandleLog
{
    char *buff;
    int size;
    
    FILE *f;
    pthread_mutex_t mutex;
    int64_t filesize_set;
    int64_t filesize;
    int fileorder;
    char filename[256];
    char filetype[16];

    void (*func)(void *,int,void *);
    void *para;
};
static struct HandleLog *morn_log_info=NULL;
__thread int morn_log_level=1;
int morn_log_levelset = -1;
static int morn_log_file_valid    = 0;
static int morn_log_console_valid = 1;
static int morn_log_function_valid= 0;
static const char *morn_log_levelname[5]={"Debug","Info","Warning","Error","\0"};
void endLog(struct HandleLog *handle)
{
    mException(morn_log_info!=handle,EXIT,"invalid log set");
    if(handle->f!=NULL)
    {
        fwrite(handle->buff-65536,1,65536-handle->size,handle->f);
        fclose(handle->f);
        free(handle->buff-65536);
    }
    if(handle->func!=NULL)
    {
        (handle->func)(handle->buff-65536,65536-handle->size,handle->para);
    }
}
#define HASH_Log 0x3f37e6f1
void m_LogSet(int level_set,int output,const char *filename,int64_t file_size,void *function,void *para)
{
    if(INVALID_POINTER(filename)) {filename=NULL;file_size=DFLT;}
    if(INVALID_POINTER(function)) {function=NULL;para     =NULL;}
    
    MHandle *hdl=mHandle(mMornObject(NULL,DFLT),Log);
    struct HandleLog *handle = hdl->handle;
    if(handle->f!=NULL) endLog(handle);
    morn_log_info = handle;
    morn_log_levelset = level_set;
    
    if(output==DFLT)
    {
        morn_log_file_valid    = (filename!=NULL);
        morn_log_function_valid= (function!=NULL);
        morn_log_console_valid = !(morn_log_file_valid||morn_log_function_valid);
    }
    else
    {
        morn_log_file_valid    = ((output|MORN_LOG_FILE   )==MORN_LOG_FILE   );
        morn_log_function_valid= ((output|MORN_LOG_CUSTOM )==MORN_LOG_CUSTOM );
        morn_log_console_valid = ((output|MORN_LOG_CONSOLE)==MORN_LOG_CONSOLE);
    }
    
    mException((morn_log_file_valid+morn_log_console_valid+morn_log_function_valid==0),EXIT,"invalid output set");
    
    if(morn_log_file_valid)
    {
        mException(filename==NULL,EXIT,"invalid log file name,witch is NULL");
        strcpy(handle->filename,filename);
        handle->f = fopen(handle->filename,"wb");

        handle->filetype[0]=0;
        int len = strlen(handle->filename);
        for(int j=len;j>0;j--)
        {
            if(handle->filename[j]=='.')
            {
                strcpy(handle->filetype,handle->filename+j);
                handle->filename[j]=0;
                break;
            }
        }
        if(file_size<0) file_size = 0x7FFFFFFFFFFFFFFF;
        handle->filesize_set = file_size;
        handle->filesize = 0;
        handle->fileorder= 0;
    }
    if(morn_log_function_valid)
    {
        handle->func = function;
        handle->para = para;
    }

    if(morn_log_file_valid||morn_log_function_valid)
    {
        handle->buff = ((char *)malloc(65536))+65536;
        handle->size = 65536;
        
        pthread_mutex_init(&(handle->mutex),NULL);
        // handle->mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    hdl->valid = 1;
}

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

void _mLog(int level,const char *format,...)
{
    va_list args; 
    if(morn_log_console_valid){va_start(args, format);vprintf(format,args);va_end(args);}
    if(morn_log_file_valid||morn_log_function_valid)
    {
        pthread_mutex_lock(&(morn_log_info->mutex));
        va_start(args,format);unsigned int n=vsnprintf(morn_log_info->buff-morn_log_info->size,morn_log_info->size,format,args);va_end(args);
        // printf("n=%d,morn_log_info->size=%d,(n<0)=%d,(n>morn_log_info->size)=%d\n",n,morn_log_info->size,n<0,(n>morn_log_info->size));
        if(n>morn_log_info->size)
        {
            if(morn_log_file_valid)
            {
                fwrite(morn_log_info->buff-65536,1,65536-morn_log_info->size,morn_log_info->f);
                morn_log_info->filesize += (65536-morn_log_info->size);
                if(morn_log_info->filesize>=morn_log_info->filesize_set)
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
                (morn_log_info->func)(morn_log_info->buff-65536,65536-morn_log_info->size,morn_log_info->para);
            }
            morn_log_info->size = 65536;
            va_start(args, format);n = vsnprintf(morn_log_info->buff-65536,65536,format,args);va_end(args);
        }
        morn_log_info->size-=n;
        pthread_mutex_unlock(&(morn_log_info->mutex));
    }
}

const char *mLogLevel()
{
    if(morn_log_level%16!=0) return morn_log_levelname[4];
    int n=morn_log_level/16;
    if((n<0)||(n>3)) return morn_log_levelname[4];
    return morn_log_levelname[n];
}
