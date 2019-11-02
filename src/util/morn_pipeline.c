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

pthread_mutex_t pipeline_mutex = PTHREAD_MUTEX_INITIALIZER;
struct HandlePipeline
{
    volatile int thread_num;
    int *order;
    int *state;
    volatile int count;
    
    pthread_mutex_t mutex0;
    pthread_cond_t cond0;
    
    pthread_mutex_t mutex1;
    pthread_cond_t cond1;
    
    volatile int over;
};
void endPipeline(void *info)
{
    struct HandlePipeline *handle = info;
    if(handle->order!= NULL) mFree(handle->order);
    if(handle->state!= NULL) mFree(handle->state);
}
#define HASH_Pipeline 0x479b64f

void *Pipeline0(MList *list,int thread)
{
    MHandle *hdl; ObjectHandle(list,Pipeline,hdl);
    struct HandlePipeline *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        pthread_mutex_init(&(handle->mutex0),NULL);
        pthread_mutex_init(&(handle->mutex1),NULL);
        
        handle->cond0=(pthread_cond_t)PTHREAD_COND_INITIALIZER;
        handle->cond1=(pthread_cond_t)PTHREAD_COND_INITIALIZER;
        
        handle->thread_num = list->num;
        handle->count=1;

        handle->over= 0;
        
        handle->order = mMalloc(sizeof(int)*list->num);
        handle->state = mMalloc(sizeof(int)*list->num);
        for(int i=0;i<list->num;i++) {handle->order[i]=0-i;handle->state[i]=-1-i;}
            
        hdl->valid = 1;
    }
    
    handle->state[thread] = handle->order[thread];
    if(handle->thread_num==1) return NULL;
    
    int thread_num = handle->thread_num;
    pthread_mutex_lock(&(handle->mutex0));
    while(handle->count<thread_num)
        pthread_cond_wait(&(handle->cond0),&(handle->mutex0));
    pthread_mutex_unlock(&(handle->mutex0));

    if(handle->over!=0)
    {
        handle->thread_num -= handle->over;
        handle->over = 1;
    }
    
    for(int i=0;i<list->num;i++) {handle->order[i]+=1;if(handle->order[i]==list->num) handle->order[i]=0;}
    handle->count = 1;
    
    pthread_mutex_lock(&(handle->mutex1));
    pthread_cond_broadcast(&(handle->cond1));
    pthread_mutex_unlock(&(handle->mutex1));
    
    if(handle->state[thread]<0) return Pipeline0(list,thread);
    
    return list->data[handle->state[thread]];
} 

void *mPipeline(MList *list,int thread)
{
    mException(list==NULL,EXIT,"invalid pipeline");
    mException((thread<0)||(thread>=list->num),EXIT,"invalid process order");
    
    if(thread==list->num-1) return Pipeline0(list,thread);
    
    pthread_mutex_lock(&pipeline_mutex);
    MHandle *hdl; ObjectHandle(list,Pipeline,hdl);
    struct HandlePipeline *handle = hdl->handle;
    while(handle->count==0) {mSleep(1);}

    handle->state[thread] = handle->order[thread];
    handle->count += 1;
    if(handle->count == handle->thread_num)
    {
        pthread_mutex_lock(&(handle->mutex0));
        pthread_cond_signal(&(handle->cond0));
        pthread_mutex_unlock(&(handle->mutex0));
    }
    pthread_mutex_unlock(&pipeline_mutex);
    
    pthread_mutex_lock(&(handle->mutex1));
    while(handle->state[thread]==handle->order[thread])
        pthread_cond_wait(&(handle->cond1),&(handle->mutex1));
    pthread_mutex_unlock(&(handle->mutex1));
    
    if(handle->thread_num+thread<list->num) return NULL;
    
    if(handle->state[thread]<0) return mPipeline(list,thread);
    
    return list->data[handle->state[thread]];
}

void mPipelineComplete(MList *list,int thread)
{
    mException(list==NULL,EXIT,"invalid pipeline");
    mException((thread<0)||(thread>=list->num),EXIT,"invalid process order");
    
    if(thread==list->num-1) return;
    
    pthread_mutex_lock(&pipeline_mutex);
    MHandle *hdl; ObjectHandle(list,Pipeline,hdl);
    struct HandlePipeline *handle = hdl->handle;
    mException((hdl->valid==0),EXIT,"invalid pipeline");

    handle->over = thread+1;
    
    handle->count += 1;
    if(handle->count == handle->thread_num)
    {
        pthread_mutex_lock(&(handle->mutex0));
        pthread_cond_signal(&(handle->cond0));
        pthread_mutex_unlock(&(handle->mutex0));
    }
    pthread_mutex_unlock(&pipeline_mutex);
}
