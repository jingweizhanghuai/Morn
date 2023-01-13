/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#ifndef __linux__
#include "windows.h"
#else
#include "unistd.h"
#include "sys/sysinfo.h"
#endif

#include "morn_ptc.h"

struct ThreadPoolData
{
    MThread tid;
    MThreadSignal sgn;
    void *handle;
    void (*func)(void *);
    void **para;
    int *flag;
    volatile int state;
};

struct ThreadBuffData
{
    void (*func)(void *);
    void **para;
    int *flag;
    int priority;
};

struct HandleThreadPool
{
    MList *pool;
    int pool_num;
    
    MList *buff;
    volatile int buff_num;

    int thread_adjust;
    int thread_max;
    int idle_count;

    int tid0;
    MThreadSignal sgn0;
};
struct HandleThreadPool *morn_thread_pool_handle=NULL;
void endThreadPool(struct HandleThreadPool *handle)
{
    for(int i=0;i<handle->pool_num;i++)
    {
        struct ThreadPoolData *data = (struct ThreadPoolData *)(handle->pool->data[i]);
        while(data->state == 1);// {mSleep(1);}
        data->func = NULL;
        data->state = 1;
        mThreadWake(data->sgn,1);
        mThreadEnd(&(data->tid));
    }
    if(handle->pool!=NULL)
    {
        mListRelease(handle->pool);
        morn_thread_pool_handle=NULL;
    }
    if(handle->buff!= NULL) mListRelease(handle->buff);
}

#define HASH_ThreadPool 0x893d6fdf

void ThreadFunc(struct ThreadPoolData *data)
{
    struct HandleThreadPool *handle = data->handle;
    MList *buff = handle->buff;
    
    while(1)
    {
        mThreadWait(data->sgn,data->state==1);
        
        if(data->func == NULL) return;
        else
        {
            (data->func)(*(data->para));
            if(data->flag!=NULL) *(data->flag)=1;
            mFree(data->para);
        }

        struct ThreadBuffData *buff_data;
        
        mThreadLockBegin(handle->sgn0);
        if(buff->num>0)
        {
            buff->num=buff->num-1;
            handle->buff_num = buff->num;
            buff_data = (struct ThreadBuffData *)(buff->data[buff->num]);
            data->func = buff_data->func;
            data->para = buff_data->para;
            data->flag = buff_data->flag;
        }
        else data->state = 0;
        mThreadLockEnd(handle->sgn0);
    }
}

void ThreadPoolNumber(int *number,struct HandleThreadPool *handle)
{
    int n=*number;
    if(handle->tid0==0) handle->tid0=mThreadID();
    else mException(handle->tid0!=mThreadID(),EXIT,"invalid thread");
    if(n==handle->pool_num) return;
    else if(n<handle->pool_num)
    {
        for(int i=n;i<handle->pool_num;i++)
        {
            struct ThreadPoolData *data = (struct ThreadPoolData *)(handle->pool->data[i]);
            while(data->state == 1);
            data->func = NULL;
            data->state = 1;
            mThreadWake(data->sgn,1);
            mThreadEnd(&(data->tid));
        }
    }
    else
    {
        for(int i=handle->pool_num;i<n;i++)
        {
            struct ThreadPoolData *data = mListWrite(handle->pool,DFLT,NULL,sizeof(struct ThreadPoolData));
            data->state=0;
            mThreadBegin(&(data->tid),ThreadFunc,data);
        }
    }
    handle->pool_num = n;
}

void m_ThreadPool(MList *pool,void *function,void *func_para,int *flag,int priority)
{
    if(flag!=NULL) *flag=0;
    
    void (*func)(void *)=function;
    mException((func==NULL),EXIT,"invalid input");

    void **para=mMalloc(sizeof(void *));
    *para=func_para;
    
    if(priority<0) priority=0;
    
    int i;
    struct ThreadPoolData *data;
    struct HandleThreadPool *handle=NULL;
    if(pool==NULL) handle=morn_thread_pool_handle;
    if(handle==NULL)
    {
        MObject *object=(pool==NULL)?mMornObject("ThreadPool",DFLT):(MObject *)pool;
        MHandle *hdl=mHandle(object,ThreadPool);
        handle = (struct HandleThreadPool *)(hdl->handle);
        
        if(pool!=NULL) {handle->pool_num=pool->num;handle->pool=NULL;}
        else
        {
            if(handle->pool==NULL) handle->pool = mListCreate();
            pool=handle->pool;
            mPropertyFunction(object,"exit",mornObjectRemove,"ThreadPool");
            morn_thread_pool_handle=handle;
        }
            
        mPropertyVariate(object,"thread_num",&(handle->pool_num),sizeof(int));
        if(handle->pool_num<=0)
        {
            #ifndef __linux__
            SYSTEM_INFO sysInfo;GetSystemInfo(&sysInfo);
            handle->pool_num=sysInfo.dwNumberOfProcessors;// cpu number
            #else
            handle->pool_num=sysconf(_SC_NPROCESSORS_ONLN);// cpu number
            #endif
            // printf("cpu num=%d\n",handle->pool_num);
        }
        mListPlace(pool,NULL,handle->pool_num,sizeof(struct ThreadPoolData));
        
        if(handle->buff==NULL) handle->buff=mListCreate();

        handle->thread_adjust=0;
        mPropertyVariate(object,"thread_adjust",&(handle->thread_adjust),sizeof(int));
        handle->thread_max = handle->pool_num*2;
        mPropertyVariate(object,"thread_max"   ,&(handle->thread_max   ),sizeof(int));
        
        for(i=0;i<pool->num;i++)
        {
            data = (struct ThreadPoolData *)(pool->data[i]);
            data->state=0;data->handle=handle;
            mThreadBegin(&(data->tid),ThreadFunc,data);
        }
        mPropertyFunction(object,"thread_num",ThreadPoolNumber,handle);
        
        hdl->valid =1;
    }
    if(pool==NULL) pool=handle->pool;

    for(i=0;i<handle->pool_num;i++)
    {
        data = (struct ThreadPoolData *)(pool->data[i]);
        if(data->state == 1) continue;

        data->func = func;
        data->para = para;
        data->flag = flag;
        data->state = 1;
        mThreadWake(data->sgn,1);
        break;
    }

    if(i==handle->pool_num-1) {handle->idle_count=0;return;}
    if(i< handle->pool_num-1)
    {
        data = (struct ThreadPoolData *)(pool->data[handle->pool_num-1]);
        if(data->state == 0) handle->idle_count++;
        if((handle->idle_count > MAX(handle->pool_num,4))&&(handle->thread_adjust)&&(handle->pool_num>2))
        {
            data->func = NULL;
            data->state = 1;
            mThreadWake(data->sgn,1);
            mThreadEnd(&(data->tid));
            
            pool->num = pool->num-1;
            handle->pool_num = pool->num;
        }
        return;
    }
    
    MList *buff = handle->buff;
    if((handle->buff_num == MAX(pool->num,4))&&(handle->thread_adjust)&&(handle->pool_num<handle->thread_max))
    {
        data = (struct ThreadPoolData *)mListWrite(pool,DFLT,NULL,sizeof(struct ThreadPoolData));
        handle->pool_num = pool->num;
        data->handle=handle;
        data->func = func;
        data->para = para;
        data->flag = flag;
        data->state = 1;
        mThreadBegin(&(data->tid),ThreadFunc,data);
        mThreadWake(data->sgn,1);
        return;
    }
    
    while(handle->buff_num > MAX(pool->num,4));
    
    struct ThreadBuffData buff_data;
    buff_data.func = func;
    buff_data.para = para;
    buff_data.flag = flag;
    buff_data.priority = priority;
    
    mThreadLockBegin(handle->sgn0);
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
    handle->buff_num=buff->num;
    mThreadLockEnd(handle->sgn0);
}


/*
struct EventInfo
{
    void (*func)(void *);
    void *para;
    int para_size;
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
        mThreadPool(handle->pool,info->func,info->para,info->para_size,NULL,DFLT);
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
*/
