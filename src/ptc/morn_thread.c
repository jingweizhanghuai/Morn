/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

void mNULL(void *p) {NULL;}

static __thread int morn_thread_ID = -1;
static int morn_thread_count = 0;
static MThreadSignal morn_thread_signal = MORN_THREAD_SIGNAL;
int mThreadID()
{
    if(morn_thread_ID==-1)
    {
        mThreadLockBegin(morn_thread_signal);
        morn_thread_count +=1;
        morn_thread_ID = morn_thread_count;
        mThreadLockEnd(morn_thread_signal);
    }
    return morn_thread_ID;
}


/*
struct ThreadFuncInfo
{
    void *func;
    void *para;
    MThread thread;
    int flag;
};
void myfunc(struct ThreadFuncInfo *info)
{
    info->flag=0;
    (void *(*func)(void *)) = info->func;
    func(info->para);
    info->flag=1;
}

struct HandleThreadFunc
{
    MList *list;
    MChain *map;
};
struct HandleThreadFunc *morn_thread_func_handle=NULL;
void endThreadFunc(struct HandleThreadFunc *handle)
{
    if(handle->list!=NULL) mListRelease(handle->list);
    morn_thread_func_handle=NULL;
}
#define HASH_ThreadFunc 0x3fac1631
void mThreadFunc(char *thread_name,void *func,void *para)
{
    struct ThreadFuncInfo info={.func=func,.para=para};
    struct HandleThreadFunc *handle=morn_thread_func_handle;
    if(handle==NULL)
    {
        MHandle *hdl = mHandle("Morn",ThreadFunc);
        handle=hdl->handle;
        if(!mHandleValid(hdl))
        {
            if(handle->list==NULL) handle->list=mListCreate();
            morn_thread_func_handle = handle;
            hdl->valid=1;
        }
    }
    mListWrite(handle->list,DFLT,&info,sizeof(struct ThreadFuncInfo));
}

void mThreadFuncRun()
{
    struct HandleThreadFunc *handle=morn_thread_func_handle;
    if(handle==NULL) return;
    MList *list = handle->list;
    
    MThread *pthread=mMalloc(
    for(int i=1;i<handle->list->num;i++)
    {
*/




/*
MThreadSignal *morn_thread_signal0=NULL;


#if defined __GNUC__
void _ThreadSignalInit0(void)
{
    pthread_mutex_init(&(morn_thread_signal0->mutex    ),PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init( &(morn_thread_signal0->condition),PTHREAD_PROCESS_PRIVATE);
}
// pthread_once_t morn_thread_signal_once=PTHREAD_ONCE_INIT;
void mThreadSignalInit(MThreadSignal sgn)
{
    intptr_t p = (intptr_t)(&sgn);
    while(mAtomicCompare((intptr_t)morn_thread_signal0,0,p)==0);
    pthread_once(&(morn_thread_signal0->once),_ThreadSignalInit);
    morn_thread_signal0=NULL;
}

MThreadSignal *mThreadSignalCreate()
{
    MThreadSignal *sgn = mMalloc(sizeof(MThreadSignal));
    pthread_mutex_init(&(sgn->mutex    ),PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init( &(sgn->condition),PTHREAD_PROCESS_PRIVATE);
    // sgn->mutex    =(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    // sgn->condition=(pthread_cond_t )PTHREAD_COND_INITIALIZER;
    return sgn;
}
void mThreadSignalRelease(MThreadSignal *sgn)
{
    pthread_cond_destroy( &(sgn->condition));
    pthread_mutex_destroy(&(sgn->mutex));
    mFree(sgn);
}
#elif defined _MSC_VER
void beginThreadSignal()
{
    InitializeCriticalSection(  &(morn_thread_signal0.mutex    ));
    InitializeConditionVariable(&(morn_thread_signal0.condition));
}

MThreadSignal *mThreadSignalCreate()
{
    MThreadSignal *sgn = mMalloc(sizeof(MThreadSignal));
    InitializeCriticalSection(  &(sgn->mutex    ));
    InitializeConditionVariable(&(sgn->condition));
    return sgn;
}
void mThreadSignalRelease(MThreadSignal *sgn)
{
    DeleteConditionVariable(&(sgn->condition));
    DeleteCriticalSection(  &(sgn->mutex));
    mFree(sgn);
}
#else

void beginThreadSignal()
{
    mtx_init(&(morn_thread_signal0.mutex    ),mtx_plain);
    cnd_init(&(morn_thread_signal0.condition));
}

MThreadSignal *mThreadSignalCreate()
{
    MThreadSignal *sgn = mMalloc(sizeof(MThreadSignal));
    mtx_init(&(sgn->mutex    ),mtx_plain);
    cnd_init(&(sgn->condition));
    return sgn;
}
void mThreadSignalRelease(MThreadSignal *sgn)
{
    cnd_destroy(&(sgn->condition));
    mtx_destroy(&(sgn->mutex    ));
    mFree(sgn);
}
#endif
*/
