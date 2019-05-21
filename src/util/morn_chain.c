#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "morn_util.h"

struct HandleChainCreate
{
    MList *memory;
    int count;
};

void endChainCreate(void *info)
{
    struct HandleChainCreate *handle = (struct HandleChainCreate *)info;
    
    if(handle->memory != NULL)
        mMemoryRelease(handle->memory);
}
#define HASH_ChainCreate 0xa95525b8
 
MChain *mChainCreate()
{
    MObject *chain = mProcCreate(NULL);
    
    MHandle *hdl; ObjectHandle(chain,ChainCreate,hdl);
    
    return chain;
}

void mChainRelease(MChain *chain)
{
    mProcRelease(chain);
}

MChainNode *mChainNode(MChain *chain,void *data,int size)
{
    MHandle *hdl = chain->handle->data[1];
    mException(hdl->flag!= HASH_ChainCreate,EXIT,"invalid chain");
    struct HandleChainCreate *handle = hdl->handle;
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT);
    
    if(size<0)size=0;
    MChainNode *node = mMemoryWrite(handle->memory,NULL,sizeof(MChainNode)+size);
    memset(node,0,sizeof(MChainNode));
    
    if(size>0)
    {
        node->data = node+1;
        if(data!=NULL) memcpy(node->data,data,size);
    }
    
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
    
    MHandle *hdl = chain->handle->data[1];
    mException(hdl->flag!= HASH_ChainCreate,EXIT,"invalid chain");
    struct HandleChainCreate *handle = hdl->handle;
    handle->count += 1;
    if(handle->count>=256)
    {
        #define MEMORY_SIZE(Memory,N) (((int *)(Memory->data[N]))[-1])
        int i;
        int n = 0;
        MMemory *memory = handle->memory;
        for(i=0;i<memory->num;i++)
        {
            node = chain->chainnode; do
            {
                if(node->data >= memory->data[i])
                    if((char *)(node->data) < (char *)(memory->data[i])+MEMORY_SIZE(memory,i))
                        break;
            }while(node!=chain->chainnode);
            if(node==chain->chainnode)
                mFree(memory->data[i]);
            else
                {memory->data[n] = memory->data[i];n=n+1;}
        }
        memory->num = n;
        handle->count = 0;
    }
}



void mChainNodeExchange(MChainNode *node1,MChainNode *node2)
{
    // int buff = node1->size;
    // node1->size = node2->size;
    // node2->size = buff;
    
    void *p = node1;
    node1 = node2;
    node2 = p;
}

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
    
    if(chain_num == 1)
        return chain;
    
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


    
    
        