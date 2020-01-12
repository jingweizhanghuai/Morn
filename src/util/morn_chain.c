/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "morn_util.h"

struct HandleChainCreate
{
    MList *memory;
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
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT);
    
    MChainNode *node = (MChainNode *)mMemoryWrite(handle->memory,NULL,sizeof(MChainNode));
    memset(node,0,sizeof(MChainNode));
    node->data = mMemoryWrite(handle->memory,data,size);
    
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

void mChainNodeDelete(MChainNode *node)
{
    node->last->next = node->next;
    node->next->last = node->last;
}



void mChainNodeExchange(MChainNode *node1,MChainNode *node2)
{
    // int buff = node1->size;
    // node1->size = node2->size;
    // node2->size = buff;
    
    MChainNode *p = node1;
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


    
    
        