/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#ifndef __linux__
#include "windows.h"
#else
#include "unistd.h"
#include "sys/sysinfo.h"
#endif

#include "morn_util.h"
struct ThreadPoolData
{
    pthread_t tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    void (*func)(void *);
    void *para;
    int *flag;
    int state;

    MList *pool;
};

struct ThreadBuffData
{
    void (*func)(void *);
    void *para;
    int *flag;
    float priority;
};

struct HandleThreadPool
{
    MList *pool;
    int thread_num;
    int cpu_num;
    
    MList *buff;
    int buff_valid;

    pthread_mutex_t mutex0;
    pthread_cond_t cond0;
};
void endThreadPool(void *info)
{
    struct HandleThreadPool *handle = (struct HandleThreadPool *)info;
    MList *pool = handle->pool;
    for(int i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *data = (struct ThreadPoolData *)(pool->data[i]);
        while(data->state == 1) {mSleep(1);}
        data->func = NULL;
        data->state = 1;
        pthread_mutex_lock(&(data->mutex));
        pthread_cond_signal(&(data->cond));
        pthread_mutex_unlock(&(data->mutex));
        pthread_join(data->tid,NULL);
    }
    
    if(handle->buff!= NULL) mListRelease(handle->buff);
}
#define HASH_ThreadPool 0x893d6fdf

void ThreadFunc(void *thread_data)
{
    struct ThreadPoolData *data = (struct ThreadPoolData *)thread_data;
    pthread_mutex_init(&(data->mutex),NULL);
    pthread_cond_init( &(data->cond ),NULL);

    MHandle *hdl=mHandle(data->pool,ThreadPool);
    struct HandleThreadPool *handle = (struct HandleThreadPool *)(hdl->handle);
    
    while(1)
    {
        pthread_mutex_lock(&(data->mutex));
        while(data->state==0)
            pthread_cond_wait(&(data->cond),&(data->mutex));
        pthread_mutex_unlock(&(data->mutex));
        if(data->func == NULL) return;
        else
        {
            (data->func)(data->para);
            if(data->flag!=NULL) *(data->flag)=1;
        }

        handle->buff_valid=1;
        pthread_mutex_lock(&(handle->mutex0));
        if(handle->buff->num==0) data->state = 0;
        else
        {
            struct ThreadBuffData *buff_data = (struct ThreadBuffData *)(handle->buff->data[handle->buff->num-1]);
            data->func = buff_data->func;
            data->para = buff_data->para;
            data->flag = buff_data->flag;
            
            handle->buff->num = handle->buff->num-1;
            pthread_cond_signal(&(handle->cond0));
        }
        pthread_mutex_unlock(&(handle->mutex0));
    }
}

void mThreadPool(MList *pool,void (*func)(void *),void *para,int *flag,float priority)
{
    mException((pool==NULL)||(func==NULL),EXIT,"invalid input");
    if(priority<0) priority=0.0;
    int flag0; if(flag==NULL) flag=&flag0; *flag=0;

    int i;
    MHandle *hdl=mHandle(pool,ThreadPool);
    struct HandleThreadPool *handle = (struct HandleThreadPool *)(hdl->handle);
    if(hdl->valid == 0)
    {
        int n =pool->num;
        if(n<=0)
        {
            #ifndef __linux__
            SYSTEM_INFO sysInfo;GetSystemInfo(&sysInfo);
            handle->cpu_num=sysInfo.dwNumberOfProcessors;
            #else
            handle->cpu_num=sysconf(_SC_NPROCESSORS_ONLN);
            #endif
            n=1;//handle->cpu_num;
        }
        mListPlace(pool,NULL,n,sizeof(struct ThreadPoolData));
        
        handle->pool = pool;
        handle->thread_num = pool->num;
        
        if(handle->buff==NULL) handle->buff=mListCreate(DFLT,NULL);
        mListClear(handle->buff);
        
        pthread_mutex_init(&(handle->mutex0),NULL);
        
        for(i=0;i<pool->num;i++)
        {
            struct ThreadPoolData *data = (struct ThreadPoolData *)(pool->data[i]);
            data->state=0;
            data->pool = pool;
            mException((pthread_create(&(data->tid),NULL,(void* (*)(void*))ThreadFunc,(void *)data)!=0),EXIT,"error with create thread");
        }
        handle->buff_valid = 1;

        hdl->valid =1;
    }
    mException((pool->num!=handle->thread_num),EXIT,"invalid thread pool");

    pthread_mutex_lock(&(handle->mutex0));
    for(i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *data = (struct ThreadPoolData *)(pool->data[i]);
        if(data->state == 1) continue;

        data->func = func;
        data->para = para;
        data->flag = flag;
        data->state = 1;
        pthread_mutex_lock(&(data->mutex));
        pthread_cond_signal(&(data->cond));
        pthread_mutex_unlock(&(data->mutex));
        break;
    }

    if(i==pool->num)
    {
        MList *buff = handle->buff;
        struct ThreadBuffData buff_data;
        buff_data.func = func;
        buff_data.para = para;
        buff_data.flag = flag;
        buff_data.priority = priority;

        handle->buff_valid = (buff->num < MAX(pool->num,4));

        while(!(handle->buff_valid))
            pthread_cond_wait(&(handle->cond0),&(handle->mutex0));
        
        for(i=0;i<buff->num;i++)
        {
            struct ThreadBuffData *p = (struct ThreadBuffData *)(buff->data[i]);
            if(priority <= p->priority)
            {
                mListElementInsert(buff,i,&buff_data,sizeof(struct ThreadBuffData));
                break;
            }
        }
        if(i==buff->num) mListWrite(buff,i,&buff_data,sizeof(struct ThreadBuffData));
    }
    pthread_mutex_unlock(&(handle->mutex0));
}

struct EventInfo
{
    void (*func)(void *);
    void *para;
};
struct HandleEvent
{
    MChain *chain;
    MChainNode *node;
    MList *pool;
    pthread_t tid;
    pthread_mutex_t mutex;
};
void EventFunc(struct HandleEvent *handle)
{
    while(1)
    {
        pthread_mutex_lock(&(handle->mutex));
        MChainNode *node = handle->node;
        struct EventInfo *info = (struct EventInfo *)(node->data);
        handle->node = node->next;
        mChainNodeDelete(handle->chain,node);
        pthread_mutex_unlock(&(handle->mutex));
        
        if(info->func==NULL) return;
        mThreadPool(handle->pool,info->func,info->para,NULL,DFLT);
    }
}
#define HASH_Event 0x154256e5
void endEvent(struct HandleEvent *handle)
{
    struct EventInfo info;info.func=NULL;
    MChainNode *node=mChainNode(handle->chain,&info,sizeof(struct EventInfo));
    pthread_mutex_lock(&(handle->mutex));
    mChainNodeInsert(handle->node,node,NULL);
    pthread_mutex_unlock(&(handle->mutex));
    pthread_join(handle->tid,NULL);
    
    if(handle->chain!=NULL) mChainRelease(handle->chain);
    if(handle->pool !=NULL) mListRelease (handle->pool );
}
void mEvent(void (*func)(void *),void *para)
{
    mException(func==NULL,EXIT,"invalid input");

    MHandle *hdl = mHandle(mMornObject(NULL,DFLT),Event);
    struct HandleEvent *handle=(struct HandleEvent *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->chain==NULL) handle->chain= mChainCreate();
        if(handle->pool ==NULL) handle->pool = mListCreate ();
        pthread_mutex_init(&(handle->mutex),NULL);
    }
    
    struct EventInfo info;info.func=func;info.para=para;
    MChainNode *node=mChainNode(handle->chain,&info,sizeof(struct EventInfo));
    mChainNodeInsert(handle->node,node,NULL);
    
    pthread_mutex_lock(&(handle->mutex));
    if(hdl->valid==0) 
    {
        handle->chain->chainnode=node;
        handle->node=node;
        mException((pthread_create(&(handle->tid),NULL,(void* (*)(void*))EventFunc,(void *)handle)!=0),EXIT,"error with create thread");
        hdl->valid =1;
    }
    else mChainNodeInsert(handle->node,node,NULL);
    pthread_mutex_unlock(&(handle->mutex));
}
