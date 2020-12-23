/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "morn_util.h"

struct HandleChainCreate
{
    MList *memory;
    int collec_num;
};
void endChainCreate(struct HandleChainCreate *handle)
{
    if(handle->memory != NULL) mMemoryRelease(handle->memory);
}
#define HASH_ChainCreate 0xa95525b8
MChain *mChainCreate()
{
    MObject *chain = mObjectCreate(NULL);
    
    MHandle *hdl=mHandle(chain,ChainCreate);
    struct HandleChainCreate *handle = (struct HandleChainCreate *)(hdl->handle);
    
    handle->memory = NULL;
    
    return chain;
}

void mChainRelease(MChain *chain)
{
    mProcRelease(chain);
}

MChainNode *mChainNode(MChain *chain,void *data,int size)
{
    MHandle *hdl = (MHandle *)(chain->handle->data[1]);
    mException(hdl->flag!= HASH_ChainCreate,EXIT,"invalid chain");
    struct HandleChainCreate *handle = (struct HandleChainCreate *)(hdl->handle);
    if(size<0) size=strlen(data);
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
    MChainNode *node = (MChainNode *)mMemoryWrite(handle->memory,NULL,sizeof(MChainNode)+size);
    node->next=node;node->last=node;
    if(size!=0) 
    {
        node->data = (void *)(node+1);
        if(data!=NULL) memcpy(node->data,data,size);
    }
    else node->data=NULL;
    
    return node;
}

void mChainNodeInsert(MChainNode *last,MChainNode *node,MChainNode *next)
{
    mException(INVALID_POINTER(node),EXIT,"invalid input");
    mException((INVALID_POINTER(last))&&(INVALID_POINTER(next)),EXIT,"invalid input");
    
    if(INVALID_POINTER(next))
        next = last->next;
    else if(INVALID_POINTER(last))
        last = next->last;
    else
        mException(((last->next!=next)||(next->last!=last)),EXIT,"invalid input");
    
    last->next = node;
    next->last = node;
    node->last = last;
    node->next = next;
}

void mChainNodeDelete(MChain *chain,MChainNode *node)
{
    node->last->next = node->next;
    node->next->last = node->last;
    if(node==chain->chainnode) chain->chainnode=node->next;
}

void mChainNodeExchange(MChain *chain,MChainNode *node1,MChainNode *node2)
{
    MChainNode *last1 = node1->last;if(last1==node1) last1=NULL;
    MChainNode *next1 = node1->next;if(next1==node1) next1=NULL;
    MChainNode *last2 = node2->last;if(last2==node2) last2=NULL;
    MChainNode *next2 = node2->next;if(next2==node2) next2=NULL;
    node1->last=(last2==NULL)?node1:last2;node1->next=(next2==NULL)?node1:next2;
    node2->last=(last1==NULL)?node2:last1;node2->next=(next1==NULL)?node2:next1;
    if(last1!=NULL) last1->next=node2;if(next1!=NULL) next1->last=node2;
    if(last2!=NULL) last2->next=node1;if(next2!=NULL) next2->last=node1;
}

// void mChainNodeExchange(MChainNode *node1,MChainNode *node2)
// {
//     MChainNode *p = node1;
//     node1 = node2;
//     node2 = p;
// }

struct HandleChainReorder
{
    MList *list;
};
void endChainReorder(void *info)
{
    struct HandleChainReorder *handle = (struct HandleChainReorder *)info;
    if(handle->list!=NULL) mListRelease(handle->list);
}
#define HASH_ChainReorder 0x6d9002b9
void mChainReorder(MChain *chain)
{
    mException(INVALID_POINTER(chain),EXIT,"invalid input source chain");
    MHandle *hdl=mHandle(chain,ChainReorder);
    struct HandleChainReorder *handle = (struct HandleChainReorder *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->list==NULL) handle->list = mListCreate(32,NULL);
        hdl->valid = 1;
    }
    MList *list = handle->list;
    MChainNode *node = chain->chainnode;
    for(int i=0;;i++)
    {
        if(i>=list->num) mListAppend(list,MIN(list->num*2,list->num+1024));
        list->data[i]=node->data;
        node = node->next; if(node==chain->chainnode) {list->num = i;break;}
    }
    mListReorder(list);
    for(int i=0;i<list->num;i++)
    {
        node->data = list->data[i];
        node = node->next;
    }
}
/*
void mChainCopy(MChain *src,MChain *dst)
{
    mException(INVALID_POINTER(src)||INVALID_POINTER(dst),EXIT,"invalid input chain");
    MHandle *hdl;
    ObjectHandle(src,ChainNodeList,hdl);
    struct HandleChainNodeList *src_handle = (struct HandleChainNodeList *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(src_handle->list==NULL) src_handle->list = mListCreate(32,NULL);
        hdl->valid = 1;
    }
    
    
    
    ObjectHandle(dst,ChainNodeList,hdl);
    struct HandleChainNodeList *dst_handle = (struct HandleChainNodeList *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(dst_handle->list==NULL) dst_handle->list = mListCreate(32,NULL);
        hdl->valid = 1;
    }
    

    
    
    MList *list = handle->list;
    MChainNode *node = chain->chainnode;
    for(int i=0;;i++)
    {
        if(i>=list->num) mListAppend(list,MIN(list->num*2,list->num+1024));
        list->data[i]=node->data;
        node = node->next; if(node==chain->chainnode) {list->num = i;break;}
    }
    
    struct HandleListCreate *src_handle = (struct HandleListCreate *)(((MHandle *)(src->handle->data[0]))->handle);
    if(src_handle->memory == NULL)
    {
        memcpy(dst->data,src->data,src->num*sizeof(void *));
        return;
    }
    
    struct HandleListCreate *dst_handle = (struct HandleListCreate *)(((MHandle *)(dst->handle->data[0]))->handle);
    if(dst_handle->memory == NULL)
        dst_handle->memory = mMemoryCreate(DFLT,DFLT);
        
    mMemoryCopy(src_handle->memory,src->data,dst_handle->memory,dst->data,src->num);
}
*/
void mChainMerge(MChain *src1,MChain *src2,MChain *dst)
{
    struct HandleChainCreate *handle1 = (struct HandleChainCreate *)(((MHandle *)(src1->handle->data[1]))->handle);
    struct HandleChainCreate *handle2 = (struct HandleChainCreate *)(((MHandle *)(src2->handle->data[1]))->handle);
    struct HandleChainCreate *dst_handle=(struct HandleChainCreate *)(((MHandle *)(dst->handle->data[1]))->handle);

    MChainNode *node11 = src1->chainnode; MChainNode *node12 = node11->last;
    MChainNode *node21 = src2->chainnode; MChainNode *node22 = node21->last;
    node11->last = node22; node22->next = node11;
    node21->last = node12; node12->next = node21;
    src1->chainnode = NULL;src2->chainnode = NULL;
    dst->chainnode = node11;

    mMemoryMerge(handle1->memory,handle2->memory,dst_handle->memory);
    mMemoryRelease(handle1->memory);handle1->memory = NULL;
    mMemoryRelease(handle2->memory);handle2->memory = NULL;
}

/*
MChain *mChainMerge(int chain_num,MChain *chain,...)
{
    mException((chain_num <= 0),EXIT,"invalid input");
    
    int arg_num = 1;
    va_list arg;
    va_start(arg,chain);
    
    while(1)
    {
        if(!INVALID_POINTER(chain))
            break;
        chain = va_arg(arg,MChain *);
        arg_num = arg_num +1;
        
        if(arg_num >= chain_num)
        {
            va_end(arg);
            mException(1,EXIT,"invalid input");
        }
    }
    
    if(chain_num == 1) return chain;
    
    MChainNode *node_last = ((MChainNode *)(chain->object))->last;
    while(1)
    {
        MChain *merge = va_arg(arg,MChain *);
        arg_num = arg_num +1;
        
        if(!INVALID_POINTER(merge))
        {
            MChainNode *merge_node = (MChainNode *)(merge->object);
            MChainNode *buff = merge_node->last;
            
            node_last->next = merge_node;
            merge_node->last = node_last;
            
            node_last = buff;
        }
        
        if(arg_num >= chain_num)
        {
            va_end(arg);
            break;
        }
    }
    
    node_last->next = (MChainNode *)(chain->object);
    ((MChainNode *)(chain->object))->last = node_last;
    
    return chain;
}
*/

void mChainNodeOperate(MChain *chain,void *function,void *para)
{
    void (*func)(void *,void *) = function;
    mException(INVALID_POINTER(chain)||(func==NULL),EXIT,"invalid input");
    MChainNode *node = chain->chainnode;if(node==NULL) return;
    do{func(node->data,para);node=node->next;}while(node!=chain->chainnode);
}
