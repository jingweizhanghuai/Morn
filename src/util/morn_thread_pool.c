/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    int *flag;
    int state;

    MList *pool;
};

struct ThreadBuffData
{
    void (*func)(void *);
    void *para;
    int *flag;
    int priority;
};

struct HandleThreadPool
{
    MList *pool;
    int thread_num;
    
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

    MHandle *hdl; ObjectHandle(data->pool,ThreadPool,hdl);
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

void mThreadPool(MList *pool,void (*func)(void *),void *para,int *flag,int priority)
{
    mException((pool==NULL)||(func==NULL),EXIT,"invalid input");
    if(priority<0) priority=0x7FFFFFFF;
    if(flag!=NULL) *flag=0;
    
    int i;
    MHandle *hdl; ObjectHandle(pool,ThreadPool,hdl);
    struct HandleThreadPool *handle = (struct HandleThreadPool *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->pool = pool;
        if(handle->thread_num!=pool->num)
        {
            mException(pool->num<=0,EXIT,"invalid input thread pool");
            mListPlace(pool,NULL,pool->num,sizeof(struct ThreadPoolData));
            handle->thread_num = pool->num;
        }
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
            if(priority >= p->priority)
            {
                mListElementInsert(buff,i,&buff_data,sizeof(struct ThreadBuffData));
                break;
            }
        }
        if(i==buff->num) mListWrite(buff,i,&buff_data,sizeof(struct ThreadBuffData));
    }
    pthread_mutex_unlock(&(handle->mutex0));
}
