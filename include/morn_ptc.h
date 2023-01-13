/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#ifndef _MORN_PTC_H_
#define _MORN_PTC_H_

#include "morn_math.h"
#if defined __GNUC__
#include <pthread.h>
#elif defined _MSC_VER
#include <process.h>
#else
#include <stdatomic.h>
#include <threads.h>
#endif

#ifdef LINUX
#include <ucontext.h>
#elif defined WINDOWS
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

void mNULL(void *p);

int mThreadID();

#if defined __GNUC__
#define mAtomicAdd(pA,B) __sync_add_and_fetch(pA,B)
#define mAtomicSub(pA,B) __sync_sub_and_fetch(pA,B)
#define mAtomicOr( pA,B) __sync_fetch_and_or( pA,B)
#define mAtomicAnd(pA,B) __sync_fetch_and_and(pA,B)
#define mAtomicXor(pA,B) __sync_fetch_and_xor(pA,B)
#define mAtomicSet(pA,B) __sync_lock_test_and_set(pA,B)
#define mAtomicCompare(pA,B,C) __sync_bool_compare_and_swap(pA,B,C)
#elif defined _MSC_VER
#define mAtomicAdd(pA,B)  InterlockedAdd(pA,B)//):((sizeof(*(pA))==8)?_InterlockedAdd64((int64_t *)(pA),(int64_t)(B)):((sizeof(*(pA))==2)?_InterlockedExchangeAdd16((int16_t *)(pA),(int16_t)(B)):_InterlockedExchangeAdd8((int8_t *)(pA),(int8_t)(B)))))
#define mAtomicSub(pA,B)  InterlockedAdd(pA,0-(B))//):((sizeof(*(pA))==8)?_InterlockedAdd64((int64_t *)(pA),0-(int64_t)(B)):((sizeof(*(pA))==2)?_InterlockedExchangeAdd16((int16_t *)(pA),0-(int16_t)(B)):_InterlockedExchangeAdd8((int8_t *)(pA),0-(int8_t)(B)))))
#define mAtomicOr( pA,B) _InterlockedOr( pA,B)//):((sizeof(*(pA))==8)?_InterlockedOr64((uint64_t *)(pA),(uint64_t)(B)):((sizeof(*(pA))==2)?_InterlockedOr16((uint16_t *)(pA),(uint16_t)(B)):_InterlockedOr8((uint8_t *)(pA),(uint8_t)(B)))))
#define mAtomicAnd(pA,B) _InterlockedAnd(pA,B)//):((sizeof(*(pA))==8)?_InterlockedAnd64((uint64_t *)(pA),(uint64_t)(B)):((sizeof(*(pA))==2)?_InterlockedAnd16((uint16_t *)(pA),(uint16_t)(B)):_InterlockedAnd8((uint8_t *)(pA),(uint8_t)(B)))))
#define mAtomicXor(pA,B) _InterlockedXor(pA,B)//):((sizeof(*(pA))==8)?_InterlockedXor64((uint64_t *)(pA),(uint64_t)(B)):((sizeof(*(pA))==2)?_InterlockedXor16((uint16_t *)(pA),(uint16_t)(B)):_InterlockedXor8((uint8_t *)(pA),(uint8_t)(B)))))
#define mAtomicSet(pA,B) _InterlockedExchange(pA,B)//):((sizeof(*(pA))==8)?_InterlockedExchange64((int64_t *)(pA),(int64_t)(B)):((sizeof(*(pA))==2)?_InterlockedExchange16((int16_t *)(pA),(int16_t)(B)):_InterlockedExchange8((int8_t *)(pA),(int8_t)(B)))))
#define mAtomicCompare(pA,B,C) (_InterlockedCompareExchange(pA,C,B)==B)//:((sizeof(*(pA))==8)?(_InterlockedCompareExchange64((int64_t *)(pA),(int64_t)(C),(int64_t)(B))==(int64_t)(B)):((sizeof(*(pA))==2)?(_InterlockedCompareExchange16((int16_t *)(pA),(int16_t)(C),(int16_t)(B))==(int16_t)(B)):(_InterlockedCompareExchange8((int8_t *)(pA),(int8_t)(C),(int8_t)(B))==(int8_t)(B)))))
#else
#define mAtomicAdd(pA,B) atomic_fetch_add((atomic_int *)(pA),(int)(B))
#define mAtomicSub(pA,B) atomic_fetch_sub((atomic_int *)(pA),(int)(B))
#define mAtomicOr( pA,B) atomic_fetch_or( (atomic_uint*)(pA),(int)(B))
#define mAtomicAnd(pA,B) atomic_fetch_and((atomic_uint*)(pA),(int)(B))
#define mAtomicXOr(pA,B) atomic_fetch_xor((atomic_uint*)(pA),(int)(B))
#define mAtomicSet(pA,B) atomic_exchange( (atomic_int *)(pA),(int)(B))
#define mAtomicCompare(pA,B,C) atomic_compare_exchange_strong((atomic_int *)(pA),(int)(B),(int)(C))
#endif


#define HASH_Thread 0xbc4bf36f
#if defined __GNUC__
#define MThread pthread_t
#define mThreadBegin(pThread,Func,Para) mException(pthread_create(pThread,NULL,(void *(*)(void *))(Func),(void *)(Para))!=0,EXIT,"create thread error")
#define mThreadEnd(pThread) pthread_join(*(pThread),NULL);
typedef struct MThreadSignal
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_rwlock_t rwlock;
    volatile int valid;
}MThreadSignal;
#define _ThreadLockInit(Sgn) do{\
    if(mAtomicSet(&((Sgn).valid),HASH_Thread)!=HASH_Thread)\
    {\
        pthread_mutex_init( &((Sgn).mutex    ),PTHREAD_PROCESS_PRIVATE);\
        pthread_cond_init(  &((Sgn).condition),PTHREAD_PROCESS_PRIVATE);\
        pthread_rwlock_init(&((Sgn).rwlock   ),NULL);\
    }\
}while(0)
#define mThreadLockBegin(Sgn) do{_ThreadLockInit(Sgn);pthread_mutex_lock(&((Sgn).mutex));}while(0)
#define mThreadLockEnd(Sgn) pthread_mutex_unlock(&((Sgn).mutex))
#define mThreadReadLockBegin(Sgn) do{_ThreadLockInit(Sgn);pthread_rwlock_rdlock(&((Sgn).rwlock));}while(0)
#define mThreadReadLockEnd(Sgn) pthread_rwlock_unlock(&((Sgn).rwlock))
#define mThreadWriteLockBegin(Sgn) do{_ThreadLockInit(Sgn);pthread_rwlock_wrlock(&((Sgn).rwlock));}while(0)
#define mThreadWriteLockEnd(Sgn) pthread_rwlock_unlock(&((Sgn).rwlock))
#define mThreadWait(Sgn,Cond) do{\
    mThreadLockBegin(Sgn);\
    while(!(Cond))\
        pthread_cond_wait(&((Sgn).condition),&((Sgn).mutex));\
    pthread_mutex_unlock(&((Sgn).mutex));\
}while(0)
#define mThreadWake(Sgn,Cond) do{\
    if(Cond)\
    {\
        mThreadLockBegin(Sgn);\
        pthread_cond_signal(&((Sgn).condition));\
        mThreadLockEnd(Sgn);\
    }\
}while(0)
#define mThreadBroadcast(Sgn) do{\
    mThreadLockBegin(Sgn);\
    pthread_cond_broadcast(&((Sgn).condition));\
    mThreadLockEnd(Sgn);\
}while(0)


#elif defined _MSC_VER
#define MThread HANDLE
#define mThreadBegin(pThread,Func,Para) do{*(pThread)=(HANDLE)_beginthreadex(NULL,0,(_beginthreadex_proc_type)(Func),(void *)(Para),0,NULL);mException(pThread==NULL,EXIT,"create thread error");}while(0)
#define mThreadEnd(pThread) do{WaitForSingleObject((*pThread),INFINITE);CloseHandle(*(pThread));}while(0)
// GetExitCodeThread
typedef struct MThreadSignal
{
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
    SRWLOCK rwlock;
    volatile int valid;
}MThreadSignal;
#define _ThreadLockInit(Sgn) do{\
    if(mAtomicSet(&((Sgn).valid),HASH_Thread)!=HASH_Thread)\
    {\
        InitializeCriticalSection(  &((Sgn).mutex    ));\
        InitializeConditionVariable(&((Sgn).condition));\
        InitializeSRWLock(          &((Sgn).rwlock   ));\
    }\
}while(0)
#define mThreadLockBegin(Sgn) do{_ThreadLockInit(Sgn);EnterCriticalSection(&((Sgn).mutex));}while(0)
#define mThreadLockEnd(Sgn) LeaveCriticalSection(&((Sgn).mutex))
#define mThreadReadLockBegin(Sgn) do{_ThreadLockInit(Sgn);AcquireSRWLockShared(&((Sgn).rwlock));}while(0)
#define mThreadReadLockEnd(Sgn) ReleaseSRWLockShared(&((Sgn).rwlock))
#define mThreadWriteLockBegin(Sgn) do{_ThreadLockInit(Sgn);AcquireSRWLockExclusive(&((Sgn).rwlock));}while(0)
#define mThreadWriteLockEnd(Sgn) ReleaseSRWLockExclusive(&((Sgn).rwlock))
#define mThreadWait(Sgn,Cond) do{\
    mThreadLockBegin(Sgn);\
    while(!(Cond))\
        SleepConditionVariableCS(&((Sgn).condition),&((Sgn).mutex),INFINITE);\
    mThreadLockEnd(Sgn);\
}while(0)
#define mThreadWake(Sgn,Cond) do{\
    if(Cond) WakeConditionVariable(&((Sgn).condition));\
}while(0)
#define mThreadBroadcast(Sgn) do{\
    WakeAllConditionVariable(&((Sgn).condition));\
}while(0)

#else
#define MThread thrd_t
#define ThreadBegin(pThread,Func,Para) thrd_create(pThread,NULL,(thrd_start_t)(Func),(void *)(Para))
#define ThreadEnd(pThread) thrd_join(*(pThread),NULL)
typedef struct MThreadSignal
{
    mtx_t mutex;
    cnd_t condition;
    volatile int rwlock;
    volatile int valid;
}MThreadSignal;
#define _ThreadLockInit(Sgn) do{\
    if(mAtomicSet(&((Sgn).valid),HASH_Thread)!=HASH_Thread)\
    {\
        mtx_init(&((Sgn).mutex),mtx_plain);\
        cnd_init(&((Sgn).condition));\
        (Sgn).rwlock= 0;\
    }\
}while(0)
#define mThreadLockBegin(Sgn) do{_ThreadLockInit(Sgn);mtx_lock(&((Sgn).mutex));}while(0)
#define mThreadLockEnd(Sgn) mtx_unlock(&((Sgn).mutex))
#define mThreadReadLockBegin(Sgn) do{_ThreadLockInit(Sgn);while(mAtomicOr(&(Sgn).rwlock,1)&1==1);}while(0)
#define mThreadReadLockEnd(Sgn) do{while(mAtomicOr(&(Sgn).rwlock,2)&1==1);}while(0)


#define mThreadWait(Sgn,Cond) do{\
    mThreadLockBegin(Sgn);\
    while(!(Cond))\
        cond_wait(&((Sgn).condition),&((Sgn).mutex));\
    mThreadLockEnd(Sgn);\
}while(0)
#define mThreadWake(Sgn,Cond) do{\
    if(Cond)\
    {\
        mThreadLockBegin(Sgn);\
        cond_signal(&((Sgn).condition));\
        mThreadLockEnd(Sgn);\
    }\
}while(0)
#define mThreadBroadcast(Sgn) do{\
    mThreadLockBegin(Sgn);\
    cond_broadcast(&((Sgn).condition));\
    mThreadLockEnd(Sgn);\
}while(0)
#endif

#define MORN_THREAD_SIGNAL {.valid=0}

#define MORN_LOCKED      0x01
#define MORN_READLOCKED  0x02
#define MORN_WRITELOCKED 0x04
#define      mLocked(Sgn) ((Sgn).state&MORN_LOCKED     )
#define  mReadLocked(Sgn) ((Sgn).state&MORN_READLOCKED )
#define mWriteLocked(Sgn) ((Sgn).state&MORN_WRITELOCKED)

#define THREAD_FUNC(F,P) mThreadBegin(pThrd++,F,P)
#define THREAD_FUNC0(F,P) F(P)
#define _mThread(Nl,F0,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14,F15,...) do{\
    int VAN=Nl;\
    mException(VAN<2,EXIT,"invalid Thread number");\
    MThread Thrd[16];\
    MThread *pThrd = &(Thrd[0]);\
    THREAD_FUNC F1;\
    if(VAN> 2) THREAD_FUNC F2;if(VAN> 3) THREAD_FUNC F3 ;if(VAN> 4) THREAD_FUNC F4 ;if(VAN> 5) THREAD_FUNC F5 ;if(VAN> 6) THREAD_FUNC F6 ;if(VAN> 7) THREAD_FUNC F7 ;if(VAN> 8) THREAD_FUNC F8 ;\
    if(VAN> 9) THREAD_FUNC F9;if(VAN>10) THREAD_FUNC F10;if(VAN>11) THREAD_FUNC F11;if(VAN>12) THREAD_FUNC F12;if(VAN>13) THREAD_FUNC F13;if(VAN>14) THREAD_FUNC F14;if(VAN>15) THREAD_FUNC F15;\
    THREAD_FUNC0 F0;\
    for(int I=0;I<VAN-1;I++) {mThreadEnd(&(Thrd[I]));}\
}while(0)
#define mThread(...) _mThread(VANumber(__VA_ARGS__),__VA_ARGS__,(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL))

void *m_ProcTopicWrite(const char *msgname,void *data,int size);
void *m_ProcTopicRead(const char *msgname,void *data,int *size);
#define mProcTopicWrite(Msgname,...) (\
    (VANumber(__VA_ARGS__)==1)?m_ProcTopicWrite(Msgname,(void *)_VA0(__VA_ARGS__),DFLT):\
    (VANumber(__VA_ARGS__)==2)?m_ProcTopicWrite(Msgname,(void *)_VA0(__VA_ARGS__),(int)VA1(__VA_ARGS__)):\
    NULL\
)
#define mProcTopicRead(...) (\
    (VANumber(__VA_ARGS__)==1)?m_ProcTopicRead((const char *)_VA0(__VA_ARGS__),NULL,NULL):\
    (VANumber(__VA_ARGS__)==2)?m_ProcTopicRead((const char *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),NULL):\
    (VANumber(__VA_ARGS__)==3)?m_ProcTopicRead((const char *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(int *)VA2(__VA_ARGS__)):\
    NULL\
)

void *m_ProcMessageWrite(const char *dstname,void *data,int size);
void *m_ProcMessageRead(const char *dstname,void *data,int *size);
#define mProcMessageWrite(Dst,...) (\
    (VANumber(__VA_ARGS__)==1)?m_ProcMessageWrite(Dst,(void *)_VA0(__VA_ARGS__),DFLT):\
    (VANumber(__VA_ARGS__)==2)?m_ProcMessageWrite(Dst,(void *)_VA0(__VA_ARGS__),(int)VA1(__VA_ARGS__)):\
    NULL\
)
#define mProcMessageRead(...) (\
    (VANumber(__VA_ARGS__)==1)?m_ProcMessageRead((const char *)_VA0(__VA_ARGS__),NULL,NULL):\
    (VANumber(__VA_ARGS__)==2)?m_ProcMessageRead((const char *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),NULL):\
    (VANumber(__VA_ARGS__)==3)?m_ProcMessageRead((const char *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(int *)VA2(__VA_ARGS__)):\
    NULL\
)

void *m_ProcVariate(const char *name,int size,int type);
#define mProcMasterVariate(Name,Size) m_ProcVariate(Name,Size,DFLT)
#define mProcSlaveVariate(Name,Size)  m_ProcVariate(Name,Size,1)

void m_ProcLockBegin(const char *mutexname);
void m_ProcLockEnd(const char *mutexname);
#define mProcLockBegin(...) m_ProcLockBegin((VANumber(__VA_ARGS__)==0)?NULL:(const char *)VA0(__VA_ARGS__))
#define mProcLockEnd(...) m_ProcLockEnd((VANumber(__VA_ARGS__)==0)?NULL:(const char *)VA0(__VA_ARGS__))

int mQueueSize(MList *queue);
void *mQueueWrite(MList *queue,void *data,int size);
void *mQueueRead(MList *queue,void *data,int *size);

void m_ThreadPool(MList *pool,void *function,void *func_para,int *flag,int priority);
#define mThreadPool(Ptr,...) do{\
    int VAN=VANumber(__VA_ARGS__);\
    if((Ptr==NULL)||(mObjectType(Ptr)==0xfa6c59f))\
    {\
             if(VAN==1) m_ThreadPool((MList *)Ptr,(void *)_VA0(__VA_ARGS__),NULL,NULL,0.0f);\
        else if(VAN==2) m_ThreadPool((MList *)Ptr,(void *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),NULL,0.0f);\
        else if(VAN==3) m_ThreadPool((MList *)Ptr,(void *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(int *)((intptr_t)VA2(__VA_ARGS__)),0.0f);\
        else if(VAN==4) m_ThreadPool((MList *)Ptr,(void *)_VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(int *)((intptr_t)VA2(__VA_ARGS__)),(int)VA3(__VA_ARGS__));\
        else mException(1,EXIT,"invalid input");\
    }\
    else\
    {\
             if(VAN==0) m_ThreadPool(NULL,Ptr,NULL,NULL,0.0f);\
        else if(VAN==1) m_ThreadPool(NULL,Ptr,(void *)_VA0(__VA_ARGS__),NULL,0.0f);\
        else if(VAN==2) m_ThreadPool(NULL,Ptr,(void *)_VA0(__VA_ARGS__),(int *)VA1(__VA_ARGS__),0.0f);\
        else if(VAN==3) m_ThreadPool(NULL,Ptr,(void *)_VA0(__VA_ARGS__),(int *)VA1(__VA_ARGS__),(intptr_t)VA2(__VA_ARGS__));\
        else mException(1,EXIT,"invalid input");\
    }\
}while(0)

// void m_Coroutine(void *func,void *para);
// #define mCoroutine(...) do{\
//     if(VANumber(__VA_ARGS__)==0) m_Coroutine(NULL,NULL);\
//     else m_Coroutine((void *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__));\
// }while(0)

void *m_CoroutineInfo(void *func,void *para,int *Flag);
void m_Coroutine(void *info);
#define _Coroutine(Func,Para,Flag) do{\
    static void *Info=NULL;\
    void *Key[2]={Func,Para};\
    if((Info==NULL)||(memcmp(Key,Info,2*sizeof(void *))!=0)) Info=m_CoroutineInfo(Func,Para,Flag);\
    m_Coroutine(Info);\
}while(0)
#define mCoroutine(...) do{\
    if(VANumber(__VA_ARGS__)==0) _Coroutine(NULL,NULL,NULL);\
    else _Coroutine((void *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(int *)VA2(__VA_ARGS__));\
}while(0)

#ifdef __cplusplus
}
#endif

#endif
