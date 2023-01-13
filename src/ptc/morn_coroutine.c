/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"
#define HASH_Coroutine 0x90997e8b

#ifdef WINDOWS
struct CoroutineInfo
{
    void *func;
    void *para;
    void *fiber;
    int flag;
    int *p_flag;
    struct CoroutineInfo *prev;
    struct CoroutineInfo *next;
};

struct HandleCoroutine
{
    MChain *map;
    struct CoroutineInfo *info;
};
int CoroutineRelease(void *p,int a,struct CoroutineInfo *info,int b)
{
    if(*(info->p_flag)>=0) DeleteFiber(info->fiber);
    return 0;
}
void endCoroutine(struct HandleCoroutine *handle)
{
    mornMapNodeOperate(handle->map,CoroutineRelease,NULL);
    ConvertFiberToThread();
    if(handle->map!=NULL) mChainRelease(handle->map);
}

void mornCoroutineFunction(struct CoroutineInfo *info)
{
    void (*func)(void *) = info->func;
    func(info->para);
    *(info->p_flag)=1;
    while(1) SwitchToFiber(info->prev->fiber);
}

__thread struct HandleCoroutine *morn_coroutine_handle = NULL;
void *m_CoroutineInfo(void *func,void *para,int *flag)
{
    if((intptr_t)flag==DFLT) flag=NULL;
    if((intptr_t)func==DFLT) func=NULL;
    __thread struct HandleCoroutine *handle=morn_coroutine_handle;
    struct CoroutineInfo *info0=NULL;
    void *key[2];
    if(handle==NULL)
    {
        if(flag!=NULL) *flag=0;
        MHandle *hdl=mHandle("Coroutine",Coroutine);
        handle = hdl->handle;
        if(!mHandleValid(hdl))
        {
            if(handle->map==NULL) handle->map=mChainCreate();
            
            key[0]=NULL;key[1]=NULL;
            info0=mornMapWrite(handle->map,key,2*sizeof(void *),NULL,sizeof(struct CoroutineInfo));
            info0->flag=-1;info0->p_flag=&(info0->flag);
            info0->func=NULL;info0->para=NULL;
            info0->prev=NULL;info0->next=NULL;
            info0->fiber=ConvertThreadToFiber(NULL);
            handle->info=info0;
            
            morn_coroutine_handle = handle;
            hdl->valid=1;
        }
    }
    info0=handle->info;
    
    struct CoroutineInfo *info1=info0->next;
    int valid=(info1!=NULL);
    if( valid) valid=(info1->func==func)&&(info1->para==para);
    if(!valid) {key[0]=func;key[1]=para;info1=mornMapRead(handle->map,key,2*sizeof(void *),NULL,NULL);}
    if(info1==NULL)
    {
        if(flag!=NULL) *flag=0;
        key[0]=func;key[1]=para;
        info1=mornMapWrite(handle->map,key,2*sizeof(void *),NULL,sizeof(struct CoroutineInfo));
        info1->fiber = CreateFiber(0,(LPFIBER_START_ROUTINE)mornCoroutineFunction,info1);
        info1->flag=0;info1->p_flag=(flag!=NULL)?flag:&(info1->flag);
        info1->func=func;info1->para=para;
        info1->next=NULL;info1->prev=info0;
    }
    info0->next=info1;
    return info1;
}

void m_Coroutine(void *info)
{
    struct CoroutineInfo *info1=info;
    if(*(info1->p_flag)==1) return;
    
    morn_coroutine_handle->info=info1;
    SwitchToFiber(info1->fiber);
}

#else
struct CoroutineInfo
{
    void *func;
    void *para;
    ucontext_t ctx;
    int flag;
    int *p_flag;
    struct CoroutineInfo *prev;
    struct CoroutineInfo *next;
};

struct HandleCoroutine
{
    MChain *map;
    ucontext_t *end_ctx;
    struct CoroutineInfo *info;
};
int CoroutineRelease(void *p,int a,struct CoroutineInfo *info,int b)
{
    if(*(info->p_flag)>=0) if(info->ctx.uc_stack.ss_sp) free(info->ctx.uc_stack.ss_sp);
    return 0;
}
void endCoroutine(struct HandleCoroutine *handle)
{
    mornMapNodeOperate(handle->map,CoroutineRelease,NULL);
    if(handle->map!=NULL) mChainRelease(handle->map);
}

__thread struct HandleCoroutine *morn_coroutine_handle = NULL;

void endCoroutineFunc()
{
    struct HandleCoroutine *handle=morn_coroutine_handle;
    swapcontext(handle->end_ctx,&(handle->info->ctx));
    while(1)
    {
        if(handle->info!=NULL)
        {
            *(handle->info->p_flag)=1;
            handle->info = handle->info->prev;
            swapcontext(handle->end_ctx,&(handle->info->ctx));
        }
    }
}

void *m_CoroutineInfo(void *func,void *para,int *flag)
{
    if((intptr_t)flag==DFLT) flag=NULL;
    if((intptr_t)func==DFLT) func=NULL;
    struct HandleCoroutine *handle=morn_coroutine_handle;
    struct CoroutineInfo *info0=NULL;
    void *key[2];
    if(handle==NULL)
    {
        if(flag!=NULL) *flag=0;
        MHandle *hdl=mHandle("Coroutine",Coroutine);
        handle = hdl->handle;
        
        if(!mHandleValid(hdl))
        {
            if(handle->map==NULL) handle->map=mChainCreate();
            
            key[0]=endCoroutineFunc;key[1]=NULL;
            info0 = mornMapWrite(handle->map,key,2*sizeof(void *),NULL,sizeof(struct CoroutineInfo));
            getcontext(&(info0->ctx));
            info0->flag=0;info0->p_flag=&(info0->flag);
            info0->func=endCoroutineFunc;info0->para=NULL;
            info0->ctx.uc_stack.ss_sp = malloc(1024);
            info0->ctx.uc_stack.ss_size = 1024;
            info0->ctx.uc_link = NULL;
            makecontext(&(info0->ctx),(void*)endCoroutineFunc,0);
            handle->end_ctx=&(info0->ctx);
            
            key[0]=NULL;key[1]=NULL;
            info0 = mornMapWrite(handle->map,key,2*sizeof(void *),NULL,sizeof(struct CoroutineInfo));
            getcontext(&(info0->ctx));
            info0->flag=-1;info0->p_flag=&(info0->flag);
            info0->func=NULL;info0->para=NULL;
            info0->ctx.uc_stack.ss_sp = NULL;
            info0->ctx.uc_link = NULL;
            info0->prev=NULL;info0->next=NULL;
            handle->info=info0;
            
            morn_coroutine_handle = handle;
            swapcontext(&(info0->ctx),handle->end_ctx);
            
            hdl->valid=1;
        }
    }
    info0=handle->info;
    
    struct CoroutineInfo *info1=info0->next;
    int valid=(info1!=NULL);
    if( valid) valid=(info1->func==func)&&(info1->para==para);
    if(!valid) {key[0]=func;key[1]=para;info1=mornMapRead(handle->map,key,2*sizeof(void *),NULL,NULL);}
    if(info1==NULL)
    {
        if(flag!=NULL) *flag=0;
        key[0]=func;key[1]=para;
        info1 = mornMapWrite(handle->map,key,2*sizeof(void *),NULL,sizeof(struct CoroutineInfo));
        getcontext(&(info1->ctx));
        info1->flag=0;info1->p_flag=(flag!=NULL)?flag:&(info1->flag);
        info1->func=func;info1->para=para;
        info1->ctx.uc_stack.ss_sp = malloc(256*1024);;
        info1->ctx.uc_stack.ss_size = 256*1024;
        info1->ctx.uc_link = handle->end_ctx;
        info1->next=NULL;info1->prev=info0;
        makecontext(&(info1->ctx),(void*)func,1,para);
    }
    info0->next=info1;
    return info1;
}

void m_Coroutine(void *info)
{
    struct CoroutineInfo *info0,*info1=info;
    if(*(info1->p_flag)==1) return;
    
    info0=morn_coroutine_handle->info;
    morn_coroutine_handle->info=info1;
    mException(swapcontext(&(info0->ctx),&(info1->ctx)),EXIT,"swapcontext error");
}
#endif
