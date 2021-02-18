/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#ifndef _MORN_PTC_H_
#define _MORN_PTC_H_

#include "morn_util.h"
#if defined __GNUC__
#include <pthread.h>
#elif defined _MSC_VER
#include <windows.h>
// #include <process.h>
#else
#include <stdatomic.h>
#include <threads.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

void mNULL(void *p);

int mThreadID();

#if defined __GNUC__
#define mAtomicAdd(pA,B) __sync_fetch_and_add(pA,B)
#define mAtomicSub(pA,B) __sync_fetch_and_sub(pA,B)
#define mAtomicOr( pA,B) __sync_fetch_and_or( pA,B)
#define mAtomicAnd(pA,B) __sync_fetch_and_and(pA,B)
#define mAtomicXor(pA,B) __sync_fetch_and_xor(pA,B)
#define mAtomicSet(pA,B) __sync_lock_test_and_set(pA,B)
#define mAtomicCompare(pA,B,C) (__sync_val_compare_and_swap(pA,B,C)==B)
#elif defined _MSC_VER
#define mAtomicAdd(pA,B) ((sizeof(*(pA))==4)?_InterlockedAdd((int32_t *)(pA),(int32_t)(B)):((sizeof(*(pA))==8)?_InterlockedAdd64((int64_t *)(pA),(int64_t)(B)):((sizeof(*(pA))==2)?_InterlockedExchangeAdd16((int16_t *)(pA),(int16_t)(B)):_InterlockedExchangeAdd8((int8_t *)(pA),(int8_t)(B)))))
#define mAtomicSub(pA,B) ((sizeof(*(pA))==4)?_InterlockedAdd((int32_t *)(pA),0-(int32_t)(B)):((sizeof(*(pA))==8)?_InterlockedAdd64((int64_t *)(pA),0-(int64_t)(B)):((sizeof(*(pA))==2)?_InterlockedExchangeAdd16((int16_t *)(pA),0-(int16_t)(B)):_InterlockedExchangeAdd8((int8_t *)(pA),0-(int8_t)(B)))))
#define mAtomicOr( pA,B) ((sizeof(*(pA))==4)?_InterlockedOr((uint32_t *)(pA),(uint32_t)(B)):((sizeof(*(pA))==8)?_InterlockedOr64((uint64_t *)(pA),(uint64_t)(B)):((sizeof(*(pA))==2)?_InterlockedOr16((uint16_t *)(pA),(uint16_t)(B)):_InterlockedOr8((uint8_t *)(pA),(uint8_t)(B)))))
#define mAtomicAnd(pA,B) ((sizeof(*(pA))==4)?_InterlockedAnd((uint32_t *)(pA),(uint32_t)(B)):((sizeof(*(pA))==8)?_InterlockedAnd64((uint64_t *)(pA),(uint64_t)(B)):((sizeof(*(pA))==2)?_InterlockedAnd16((uint16_t *)(pA),(uint16_t)(B)):_InterlockedAnd8((uint8_t *)(pA),(uint8_t)(B)))))
#define mAtomicXor(pA,B) ((sizeof(*(pA))==4)?_InterlockedXor((uint32_t *)(pA),(uint32_t)(B)):((sizeof(*(pA))==8)?_InterlockedXor64((uint64_t *)(pA),(uint64_t)(B)):((sizeof(*(pA))==2)?_InterlockedXor16((uint16_t *)(pA),(uint16_t)(B)):_InterlockedXor8((uint8_t *)(pA),(uint8_t)(B)))))
#define mAtomicSet(pA,B) ((sizeof(*(pA))==4)?_InterlockedExchange((int32_t *)(pA),(int32_t)(B)):((sizeof(*(pA))==8)?_InterlockedExchange64((int64_t *)(pA),(int64_t)(B)):((sizeof(*(pA))==2)?_InterlockedExchange16((int16_t *)(pA),(int16_t)(B)):_InterlockedExchange8((int8_t *)(pA),(int8_t)(B)))))
#define mAtomicCompare(pA,B,C) ((sizeof(*(pA))==4)?(_InterlockedCompareExchange((int32_t *)(pA),(int32_t)((intptr_t)(C)),(int32_t)((intptr_t)(B)))==(int32_t)((intptr_t)(B))):((sizeof(*(pA))==8)?(_InterlockedCompareExchange64((int64_t *)(pA),(int64_t)(C),(int64_t)(B))==(int64_t)(B)):((sizeof(*(pA))==2)?(_InterlockedCompareExchange16((int16_t *)(pA),(int16_t)(C),(int16_t)(B))==(int16_t)(B)):(_InterlockedCompareExchange8((int8_t *)(pA),(int8_t)(C),(int8_t)(B))==(int8_t)(B)))))
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
#define mThreadBegin(pThread,Func,Para) pthread_create(pThread,NULL,(void *(*)(void *))(Func),(void *)(Para))
#define mThreadEnd(pThread) pthread_join(*(pThread),NULL);
typedef struct MThreadSignal
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_rwlock_t rwlock;
    volatile int valid;
}MThreadSignal;
#define _ThreadLockInit(Sgn) do{\
    if((Sgn).valid!=HASH_Thread)\
    {\
        if(mAtomicCompare(&((Sgn).valid),0,1))\
        {\
            pthread_mutex_init( &((Sgn).mutex    ),PTHREAD_PROCESS_PRIVATE);\
            pthread_cond_init(  &((Sgn).condition),PTHREAD_PROCESS_PRIVATE);\
            pthread_rwlock_init(&((Sgn).rwlock   ),NULL);\
            (Sgn).valid = HASH_Thread;\
        }\
        else while((Sgn).valid!=HASH_Thread);\
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
#define mThreadBegin(pThread,Func,Para) do{*(pThread)=(HANDLE)_beginthreadex(NULL,0,(_beginthreadex_proc_type)(Func),(void *)(Para),0,NULL);}while(0)
#define mThreadEnd(pThread) do{WaitForSingleObject((*pThread),INFINITE);CloseHandle(*(pThread));}while(0)
typedef struct MThreadSignal
{
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
    SRWLOCK rwlock;
    volatile int valid;
}MThreadSignal;
#define _ThreadLockInit(Sgn) do{\
    if((Sgn).valid!=HASH_Thread)\
    {\
        if(mAtomicCompare(&((Sgn).valid),0,1))\
        {\
            InitializeCriticalSection(  &((Sgn).mutex    ));\
            InitializeConditionVariable(&((Sgn).condition));\
            InitializeSRWLock(          &((Sgn).srw      ));\
            (Sgn).valid = HASH_Thread;\
        }\
        else while((Sgn).valid!=HASH_Thread);\
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
}
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
    if((Sgn).valid!=HASH_Thread)\
    {\
        if(mAtomicCompare(&((Sgn).valid),0,1))\
        {\
            mtx_init(&((Sgn).mutex),mtx_plain);\
            cnd_init(&((Sgn).condition));\
            (Sgn).rwlock= 0;
            (Sgn).valid = HASH_Thread;\
        }\
        else while((Sgn).valid!=HASH_Thread);\
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
    if(Cond)
    {
        mThreadLockBegin(Sgn);
        cond_signal(&((Sgn).condition));
        mThreadLockEnd(Sgn);
    }
}while(0)
#define mThreadBroadcast(Sgn) do{\
    mThreadLockBegin(Sgn);\
    cond_broadcast(&((Sgn).condition)));\
    mThreadLockEnd(Sgn);\
}while(0)
#endif

#define MORN_THREAD_SIGNAL {.valid=0}

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
#define mThread(...) ARG(_mThread(VA_ARG_NUM(__VA_ARGS__),__VA_ARGS__,(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL)))

void *mProcTopicWrite(const char *msgname,void *data,int size);
void *mProcTopicRead(const char *msgname,void *data,int *size);
void *mProcMessageWrite(const char *dstname,void *data,int size);
void *mProcMessageRead(const char *dstname,void *data,int *size);


#ifdef __cplusplus
}
#endif

#endif