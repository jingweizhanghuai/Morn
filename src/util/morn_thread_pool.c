#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

struct ThreadPoolData
{
    pthread_t tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    void (*func)(void *);
    void *para;
    int state;
    
    MList *buff;
    pthread_mutex_t *mutex0;
};

struct ThreadBuffData
{
    void (*func)(void *);
    void *para;
    int priority;
};

void ThreadFunc(void *thread_data)
{
    struct ThreadPoolData *data = thread_data;
    pthread_mutex_init(&(data->mutex),NULL);
    MList *buff = data->buff;
    while(1)
    {
        pthread_mutex_lock(&(data->mutex));
        while(data->state==0)
            pthread_cond_wait(&(data->cond),&(data->mutex));
        pthread_mutex_unlock(&(data->mutex));
        if(data->func == NULL) return;
        else (data->func)(data->para);

        pthread_mutex_lock(data->mutex0);
        if(buff->num==0) data->state = 0;
        else
        {
            struct ThreadBuffData *buff_data = buff->data[buff->num-1];
            data->func = buff_data->func;
            data->para = buff_data->para;
            buff->num = buff->num-1;
        }
        pthread_mutex_unlock(data->mutex0);
    }
}

struct HandleThreadPool
{
    MList *pool;
    pthread_mutex_t mutex;
    int thread_num;
    MList *buff;
};
void endThreadPool(void *info)
{
    struct HandleThreadPool *handle = info;
    MList *pool = handle->pool;
    for(int i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *data = pool->data[i];
        while(data->state == 1) {mSleep(1);}
        data->func = NULL;
        data->state = 1;
        pthread_join(data->tid,NULL);
    }
    
    if(handle->buff!= NULL) mListRelease(handle->buff);
}
#define HASH_ThreadPool 0x893d6fdf
void mThreadPool(MList *pool,void (*func)(void *),void *para,int priority)
{
    mException((pool==NULL)||(func==NULL),EXIT,"invalid input");
    
    int i;
    MHandle *hdl; ObjectHandle(pool,ThreadPool,hdl);
    struct HandleThreadPool *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->pool = pool;
        if(handle->thread_num!=pool->num)
        {
            mException(pool->num<=0,EXIT,"invalid input thread pool");
            mListPlace(pool,pool->num,sizeof(struct ThreadPoolData));
            handle->thread_num = pool->num;
        }
        if(handle->buff==NULL) handle->buff=mListCreate(DFLT,NULL);
        mListClear(handle->buff);
        
        pthread_mutex_init(&(handle->mutex),NULL);
        
        for(i=0;i<pool->num;i++)
        {
            struct ThreadPoolData *data = pool->data[i];
            data->buff = handle->buff;
            data->mutex0=&(handle->mutex);
            data->state=0;
            mException((pthread_create(&(data->tid),NULL,(void *)ThreadFunc,(void *)data)!=0),EXIT,"error with create thread");
        }

        hdl->valid =1;
    }
    mException((pool->num!=handle->thread_num),EXIT,"invalid thread pool");

    pthread_mutex_lock(&(handle->mutex));
    
    for(i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *data = pool->data[i];
        if(data->state == 1) continue;

        data->func = func;
        data->para = para;
        data->state = 1;
        break;
    }

    if(i==pool->num)
    {
        MList *buff = handle->buff;
        struct ThreadBuffData buff_data;
        buff_data.func = func;
        buff_data.para = para;
        buff_data.priority = priority;
        
        for(i=0;i<buff->num;i++)
        {
            struct ThreadBuffData *p = buff->data[i];
            if(priority >= p->priority)
            {
                mListElementInsert(buff,i,&buff_data,sizeof(struct ThreadBuffData));
                break;
            }
        }
        if(i==buff->num) mListWrite(buff,i,&buff_data,sizeof(struct ThreadBuffData));
    }
    pthread_mutex_unlock(&(handle->mutex));
}
    

    
    
    

    