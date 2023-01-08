/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"
#define HASH_Coroutine 0x90997e8b

#ifdef WINDOWS
struct CoroutineInfo
{
    char name[64];
    void *fiber;
    void *func;
    void *para;
    int flag;
    struct CoroutineInfo *prev;
    struct CoroutineInfo *next;
};

struct HandleCoroutine
{
    MChain *map;
};
int CoroutineRelease(void *p,int a,struct CoroutineInfo *info,int b)
{
    if(info->flag<0) DeleteFiber(info->fiber);
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
    info->flag=-1;
    while(1) SwitchToFiber(info->prev->fiber);
}

__thread struct HandleCoroutine *morn_coroutine_handle = NULL;
void *_CoroutineInfo(const char *name)
{
    struct CoroutineInfo *info=NULL;
    if(morn_coroutine_handle==NULL)
    {
        MHandle *hdl=mHandle("Coroutine",Coroutine);
        struct HandleCoroutine *handle = hdl->handle;
        if(!mHandleValid(hdl))
        {
            if(handle->map==NULL) handle->map=mChainCreate();
            info=mornMapWrite(handle->map,name,DFLT,NULL,sizeof(struct CoroutineInfo));
            strncpy(info->name,name,63);
            info->flag=1;info->func=NULL;info->prev=NULL;info->next=NULL;
            info->fiber=ConvertThreadToFiber(NULL);

            morn_coroutine_handle = handle;
            hdl->valid=1;
        }
    }
    if(info==NULL) info=mornMapRead(morn_coroutine_handle->map,name,DFLT,NULL,NULL);
    return info;
}

void m_Coroutine(void *info,const char *name1,void *func,void *para)
{
    struct CoroutineInfo *info0=info;
    struct CoroutineInfo *info1=info0->next;
    int info1_valid= (info1!=NULL);
    if(info1_valid) info1_valid=(strcmp(info1->name,name1)==0);
    if(!info1_valid) info1=mornMapRead(morn_coroutine_handle->map,name1,DFLT,NULL,NULL);
    if(info1==NULL)
    {
        info1=mornMapWrite(morn_coroutine_handle->map,name1,DFLT,NULL,sizeof(struct CoroutineInfo));
        info0->next=info1;
        strncpy(info1->name,name1,63);
        info1->func=func;info1->para=para;info1->flag=0;
        info1->prev=info0;
        info1->fiber = CreateFiber(0,(LPFIBER_START_ROUTINE)mornCoroutineFunction,info1);
    }
    if(info1->flag>=0) SwitchToFiber(info1->fiber);
}

#else
struct CoroutineInfo
{
    char name[64];
    ucontext_t ctx;
    int flag;
    struct CoroutineInfo *prev;
    struct CoroutineInfo *next;
};

struct HandleCoroutine
{
    MChain *map;
    ucontext_t *ctx0;
    char *stack[256];
    int stack_num;
};
void endCoroutine(struct HandleCoroutine *handle)
{
    if(handle->map!=NULL) mChainRelease(handle->map);
    for(int i=0;i<handle->stack_num;i++) {if(handle->stack[i]) free(handle->stack[i]);}
}


__thread struct CoroutineInfo *morn_coroutine_info=NULL;
__thread struct HandleCoroutine *morn_coroutine_handle = NULL;
void endCoroutineFunc()
{
    swapcontext(morn_coroutine_handle->ctx0,&(morn_coroutine_info->ctx));
    while(1)
    {
        if(morn_coroutine_info!=NULL)
        {
            morn_coroutine_info->flag=-1;
            morn_coroutine_info = morn_coroutine_info->prev;
            swapcontext(morn_coroutine_handle->ctx0,&(morn_coroutine_info->ctx));
        }
    }
}

void *_CoroutineInfo(const char *name)
{
    struct CoroutineInfo *info;
    int ctx0_valid=1;
    if(morn_coroutine_handle==NULL)
    {
        MHandle *hdl=mHandle("Coroutine",Coroutine);
        struct HandleCoroutine *handle = hdl->handle;
        if(!mHandleValid(hdl))
        {
            if(handle->map==NULL) handle->map=mChainCreate();
            info = mornMapWrite(handle->map,"endCoroutineFunc",DFLT,NULL,sizeof(struct CoroutineInfo));
            strcpy(info->name,"endCoroutineFunc");
            getcontext(&(info->ctx));
            info->flag=1;
            handle->stack[handle->stack_num]=malloc(1024);
            info->ctx.uc_stack.ss_sp = handle->stack[handle->stack_num];
            info->ctx.uc_stack.ss_size = 1024;
            info->ctx.uc_link = NULL;
            makecontext(&(info->ctx),(void*)endCoroutineFunc,0);
            handle->ctx0=&(info->ctx);
            handle->stack_num++;
            ctx0_valid=0;
        
            morn_coroutine_handle = handle;
            hdl->valid=1;
        }
    }
    info=mornMapRead(morn_coroutine_handle->map,name,DFLT,NULL,NULL);
    if(info!=NULL) return info;
    
    info = mornMapWrite(morn_coroutine_handle->map,name,DFLT,NULL,sizeof(struct CoroutineInfo));
    strncpy(info->name,name,63);
    getcontext(&(info->ctx));
    
    if(ctx0_valid==0)
    {
        info->prev=NULL;
        info->flag=1;
        morn_coroutine_info = info;
        swapcontext(&(info->ctx),morn_coroutine_handle->ctx0);
    }
    else
    {
        info->flag=0;
        char *stack=malloc(256*1024);
        int stack_num=morn_coroutine_handle->stack_num;
        mException(stack_num>=256,EXIT,"coroutine too much");
        info->ctx.uc_stack.ss_sp = stack;
        info->ctx.uc_stack.ss_size = 256*1024;
        info->ctx.uc_link = morn_coroutine_handle->ctx0;
        morn_coroutine_handle->stack[stack_num]=stack;
        morn_coroutine_handle->stack_num =stack_num+1;
    }
    return info;
}

void m_Coroutine(void *info,const char *name1,void *func,void *para) 
{
    struct CoroutineInfo *info0 = info;
    struct CoroutineInfo *info1 = info0->next;
    int info1_valid= (info1!=NULL);
    if(info1_valid) info1_valid=(strcmp(info1->name,name1)==0);
    
    if(!info1_valid) {info1=_CoroutineInfo(name1);info0->next=info1;}
    
    if(info1->flag==-1) morn_coroutine_info=info0;
    else {
        morn_coroutine_info=info1;
        if(info1->flag==0){info1->prev=info0;makecontext(&(info1->ctx),(void*)func,1,para);info1->flag=1;}
        mException(swapcontext(&(info0->ctx),&(info1->ctx)),EXIT,"swapcontext error");
    }
}
#endif
