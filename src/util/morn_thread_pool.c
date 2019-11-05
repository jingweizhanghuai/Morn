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
    int state;
    
    MList *pool;
};

struct ThreadBuffData
{
    void (*func)(void *);
    void *para;
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
    struct HandleThreadPool *handle = info;
    MList *pool = handle->pool;
    for(int i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *data = pool->data[i];
        while(data->state == 1) {mSleep(1);}
        data->func = NULL;
        data->state = 1;
        pthread_mutex_lock(&(data->mutex));
        pthread_cond_signal(&(data->cond));
        pthread_mutex_unlock(&(data->mutex));
        
        pthread_join(data->tid,NULL);

        pthread_cond_destroy(&(data->cond));
        pthread_mutex_destroy(&(data->mutex));
    }
    
    if(handle->buff!= NULL) mListRelease(handle->buff);

    pthread_cond_destroy(&(handle->cond0));
    pthread_mutex_destroy(&(handle->mutex0));
}
#define HASH_ThreadPool 0x893d6fdf

void ThreadFunc(void *thread_data)
{
    struct ThreadPoolData *data = thread_data;
    pthread_mutex_init(&(data->mutex),NULL);
    pthread_cond_init( &(data->cond ),NULL);

    MHandle *hdl; ObjectHandle(data->pool,ThreadPool,hdl);
    struct HandleThreadPool *handle = hdl->handle;
    
    while(1)
    {
        pthread_mutex_lock(&(data->mutex));
        while(data->state==0)
            pthread_cond_wait(&(data->cond),&(data->mutex));
        pthread_mutex_unlock(&(data->mutex));
        if(data->func == NULL) return;
        else (data->func)(data->para);

        handle->buff_valid = 1;
        pthread_mutex_lock(&(handle->mutex0));
        if(handle->buff->num==0) data->state = 0;
        else
        {
            struct ThreadBuffData *buff_data = handle->buff->data[handle->buff->num-1];
            data->func = buff_data->func;
            data->para = buff_data->para;
            handle->buff->num = handle->buff->num-1;
            pthread_cond_signal(&(handle->cond0));
        }
        pthread_mutex_unlock(&(handle->mutex0));
    }
}

void mThreadPool(MList *pool,void (*func)(void *),void *para,int priority)
{
    
    mException((pool==NULL)||(func==NULL),EXIT,"invalid input");
    if(priority<=0) priority = 0x7FFFFFFF;
    
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
        
        pthread_mutex_init(&(handle->mutex0),NULL);
        pthread_cond_init( &(handle->cond0 ),NULL);
        
        for(i=0;i<pool->num;i++)
        {
            struct ThreadPoolData *data = pool->data[i];
            data->state=0;
            data->pool = pool;
            mException((pthread_create(&(data->tid),NULL,(void *)ThreadFunc,(void *)data)!=0),EXIT,"error with create thread");
        }
        handle->buff_valid = 1;

        hdl->valid =1;
    }
    mException((pool->num!=handle->thread_num),EXIT,"invalid thread pool");
    
    pthread_mutex_lock(&(handle->mutex0));
    for(i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *data = pool->data[i];
        if(data->state == 1) continue;

        data->func = func;
        data->para = para;
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
        buff_data.priority = priority;

        printf("buff->num is %d\n",buff->num);
        handle->buff_valid = (buff->num < MAX(pool->num,4));
        while(handle->buff_valid==0)
            pthread_cond_wait(&(handle->cond0),&(handle->mutex0));
        printf("buff->num is %d\n",buff->num);
        
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
    pthread_mutex_unlock(&(handle->mutex0));
}
    
int mThreadPoolCheck(MList *pool,void (*func)(void *),void *para)
{
    mException((pool==NULL)||(func==NULL),EXIT,"invalid input");

    MHandle *hdl; ObjectHandle(pool,ThreadPool,hdl);
    mException((hdl->valid == 0),EXIT,"invalid thread pool");
    struct HandleThreadPool *handle = hdl->handle;

    MList *buff = handle->buff;
    for(int i=0;i<buff->num;i++)
    {
        struct ThreadBuffData *buff_data = buff->data[i];
        if((buff_data->func==func)&&(buff_data->para==para)) return 1;
    }

    for(int i=0;i<pool->num;i++)
    {
        struct ThreadPoolData *pool_data = pool->data[i];
        if((pool_data->func==func)&&(pool_data->para==para)) return 1;
    }

    return 0;
}
