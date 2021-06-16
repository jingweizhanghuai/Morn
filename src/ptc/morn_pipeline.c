/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

MThreadSignal pipeline_sgn=MORN_THREAD_SIGNAL;
struct HandlePipeline
{
    volatile int thread_num;
    int *order;
    int *state;
    volatile int count;

    MThreadSignal sgn0;
    MThreadSignal sgn1;
    // pthread_mutex_t mutex0;
    // pthread_cond_t cond0;
    // pthread_mutex_t mutex1;
    // pthread_cond_t cond1;
    
    volatile int over;
};
void endPipeline(void *info)
{
    struct HandlePipeline *handle = (struct HandlePipeline *)info;
    if(handle->order!= NULL) mFree(handle->order);
    if(handle->state!= NULL) mFree(handle->state);
}
#define HASH_Pipeline 0x479b64f

void *Pipeline0(MList *list,int thread)
{
    MHandle *hdl=mHandle(list,Pipeline);
    struct HandlePipeline *handle = (struct HandlePipeline *)(hdl->handle);
    if(hdl->valid == 0)
    {
        // pthread_mutex_init(&(handle->mutex0),NULL);
        // pthread_mutex_init(&(handle->mutex1),NULL);
        
        // handle->cond0=(pthread_cond_t)PTHREAD_COND_INITIALIZER;
        // handle->cond1=(pthread_cond_t)PTHREAD_COND_INITIALIZER;
        
        handle->thread_num = list->num;
        
        handle->over= 0;
        
        handle->order = (int *)mMalloc(sizeof(int)*list->num);
        handle->state = (int *)mMalloc(sizeof(int)*list->num);
        for(int i=0;i<list->num;i++) {handle->order[i]=0-i;handle->state[i]=-1-i;}
        
        hdl->valid = 1;
        handle->count=1;
    }

    handle->state[thread] = handle->order[thread];
    if(handle->thread_num==1) return NULL;
    
    int thread_num = handle->thread_num;
    mThreadWait(handle->sgn0,(handle->count>=thread_num));
    // pthread_mutex_lock(&(handle->mutex0));
    // while(handle->count<thread_num)
    //     pthread_cond_wait(&(handle->cond0),&(handle->mutex0));
    // pthread_mutex_unlock(&(handle->mutex0));

    if(handle->over!=0)
    {
        handle->thread_num -= handle->over;
        handle->over = 1;
    }
    
    for(int i=0;i<list->num;i++) {handle->order[i]+=1;if(handle->order[i]==list->num) handle->order[i]=0;}
    handle->count = 1;

    mThreadBroadcast(handle->sgn1);
    // pthread_mutex_lock(&(handle->mutex1));
    // pthread_cond_broadcast(&(handle->cond1));
    // pthread_mutex_unlock(&(handle->mutex1));
    
    if(handle->state[thread]<0) return Pipeline0(list,thread);
    
    return list->data[handle->state[thread]];
} 

void *mPipeline(MList *list,int thread)
{
    mException(list==NULL,EXIT,"invalid pipeline");
    mException((thread<0)||(thread>=list->num),EXIT,"invalid process order");
    
    if(thread==list->num-1) return Pipeline0(list,thread);
    
    mThreadLockBegin(pipeline_sgn);
    MHandle *hdl=mHandle(list,Pipeline);
    struct HandlePipeline *handle = (struct HandlePipeline *)(hdl->handle);
    while(handle->count==0) {mSleep(1);}
    
    handle->state[thread] = handle->order[thread];
    handle->count += 1;
    mThreadWake(handle->sgn0,(handle->count==handle->thread_num));
    // if(handle->count == handle->thread_num)
    // {
    //     pthread_mutex_lock(&(handle->mutex0));
    //     pthread_cond_signal(&(handle->cond0));
    //     pthread_mutex_unlock(&(handle->mutex0));
    // }
    mThreadLockEnd(pipeline_sgn);

    mThreadWait(handle->sgn1,(handle->state[thread]!=handle->order[thread]));
    // pthread_mutex_lock(&(handle->mutex1));
    // while(handle->state[thread]==handle->order[thread])
    //     pthread_cond_wait(&(handle->cond1),&(handle->mutex1));
    // pthread_mutex_unlock(&(handle->mutex1));
    
    if(handle->thread_num+thread<list->num) return NULL;
    
    if(handle->state[thread]<0) return mPipeline(list,thread);
    
    return list->data[handle->state[thread]];
}

void mPipelineComplete(MList *list,int thread)
{
    mException(list==NULL,EXIT,"invalid pipeline");
    mException((thread<0)||(thread>=list->num),EXIT,"invalid process order");
    
    if(thread==list->num-1) return;
    
    mThreadLockBegin(pipeline_sgn);
    MHandle *hdl=mHandle(list,Pipeline);
    struct HandlePipeline *handle = (struct HandlePipeline *)(hdl->handle);
    mException((hdl->valid==0),EXIT,"invalid pipeline");

    handle->over = thread+1;
    
    handle->count += 1;
    mThreadWake(handle->sgn0,(handle->count==handle->thread_num));
    // if(handle->count == handle->thread_num)
    // {
    //     pthread_mutex_lock(&(handle->mutex0));
    //     pthread_cond_signal(&(handle->cond0));
    //     pthread_mutex_unlock(&(handle->mutex0));
    // }
    mThreadLockEnd(pipeline_sgn);
}
