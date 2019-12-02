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

#define MEMORY_SIZE(Memory,N) (((int *)(Memory->data[N]))[-1])

struct HandleMemory
{
    int mem_num;
    int mem_size;
    int mem_idx;
    char *pdata;
};
void endMemory(void *info) {NULL;}
#define HASH_Memory 0x25793220
MMemory *mMemoryCreate(int num,int size)
{
    MMemory *memory = (MMemory *)mMalloc(sizeof(MMemory));
    memset(memory,0,sizeof(MMemory));
    
    MHandle *hdl; ObjectHandle(memory,Memory,hdl);hdl->valid = 1;
    
    struct HandleMemory *handle = hdl->handle;
     
    if(size<0) {size=0;} size = ((size+15)>>4)<<4;
    if(num<0) num=0;
    size = size*num;
    
    handle->mem_num  = 64;
    memory->data = (void **)mMalloc(handle->mem_num*sizeof(void*));
    
    if(size<=0) 
    {
        memory->num = 0;
        handle->mem_size = 0;
        handle->mem_idx = 0;
        handle->pdata = NULL;
        return memory;
    }
    else
    {
        memory->data[0] = mMalloc(size+16);
        memory->num = 1;
        handle->mem_size = size+16;
        handle->mem_idx = 0;
        handle->pdata=memory->data[0];
        return memory;
    }
}

void mMemoryRelease(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    
    if(!INVALID_POINTER(memory->handle))
        mHandleRelease(memory->handle);
    
    for(int i=0;i<memory->num;i++)
        mFree(memory->data[i]);
    mFree(memory->data);
    mFree(memory);
}

 
void mMemoryDataSet(MMemory *memory,char value)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    
    for(int i=0;i<memory->num;i++)
        memset(memory->data[i],value,MEMORY_SIZE(memory,i));
}

int mMemorySize(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    
    int size = 0;
    for(int i=0;i<memory->num;i++)
        size = size + MEMORY_SIZE(memory,i);
    return size;
}

int mMemoryCheck(MMemory *memory,void *check)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    
    for(int i=0;i<memory->num;i++)
    {
        if(check < memory->data[i])
            continue;
        if((char *)check >= (char *)(memory->data[i])+MEMORY_SIZE(memory,i))
            continue;
        return 1;
    }
    return 0;
}

void mMemoryCollect(MMemory *memory,void **data,int num)
{
    int i,j;
    int n = 0;
    for(i=0;i<memory->num;i++)
    {
        for(j=0;j<num;j++)
        {
            if(data[j] >= memory->data[i])
                if((char *)(data[j]) < (char *)(memory->data[i])+MEMORY_SIZE(memory,i))
                    break;
        }
        if(j==num)
            mFree(memory->data[i]);
        else
            {memory->data[n] = memory->data[i];n=n+1;}
    }
    memory->num = n;
}

void *mMemoryAppend(MMemory *memory,int num,int size)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    mException(((num<=0)||(size<=0)),EXIT,"invalid input");
    
    MHandle *hdl = memory->handle->data[0];
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = hdl->handle;
    
    int memory_num = memory->num + 1;
    if(memory_num > handle->mem_num)
    {
        handle->mem_num += 64;
        void **data = (void **)mMalloc((handle->mem_num)*sizeof(void *));
        memcpy(data,memory->data,memory->num*sizeof(void *));
        mFree(memory->data);
        memory->data = data;
    }

    size = ((size+15)>>4)<<4;
    char *block = mMalloc(size*num+16);
    handle->mem_size = size*num+16;
    handle->pdata=block;
    memory->data[memory->num] = block;
    memory->num = memory_num;
    return block;
}

void mMemoryCopy(MMemory *src,void **isrc,MMemory *dst,void **idst,int num)
{
    mException(INVALID_POINTER(src)||INVALID_POINTER(dst)||(dst==src),EXIT,"invalid input");
    
    int i,j;
    if((isrc==NULL)||(idst==NULL)||(num<=0))
    {
        for(i=0;i<src->num;i++)
        {
            char *p = mMemoryAppend(dst,1,MEMORY_SIZE(src,i));
            memcpy(p,src->data[i],MEMORY_SIZE(src,i));
        }
        return;
    }
    
    int *shift = mMalloc(src->num*sizeof(int));
    memset(shift,DFLT,src->num*sizeof(int));
    
    mHandleReset(dst->handle);
    
    for(j=0;j<num;j++)
    {
        for(i=0;i<src->num;i++)
        {
            if(isrc[j] >= src->data[i])
                if((char *)(isrc[j]) < (char *)(src->data[i])+MEMORY_SIZE(src,i))
                {
                    if(shift[i] == DFLT)
                    {
                        char *p = mMemoryAppend(dst,1,MEMORY_SIZE(src,i));
                        memcpy(p,src->data[i],MEMORY_SIZE(src,i));
                        shift[i] = p-(char *)src->data[i];
                    }
                    idst[j] = (char *)(isrc[j]) + shift[i];
                    break;
                }
        }
        if(i==src->num)
            idst[j] = isrc[j];
    }
    
    mFree(shift);
}

void mMemoryMerge(MMemory *mem1,MMemory *mem2,MMemory *dst)
{
    int num1 = mem1->num;
    int num2 = mem2->num;
    
    void **data = (void **)mMalloc((num1+num2)*sizeof(void *));
    if(num1 > 0)
    {
        memcpy(data,mem1->data,num1*sizeof(void *));
        mFree(mem1->data);
        mem1->data = NULL;
        mem1->num = 0;
        mHandleReset(mem1->handle);
    }
    if(num2 > 0)
    {
        memcpy(data+num1,mem2->data,num2*sizeof(void *));
        mFree(mem2->data);
        mem2->data = NULL;
        mem2->num = 0;
        mHandleReset(mem2->handle);
    }
    
    mHandleReset(dst->handle);
    
    if(dst->data != NULL)
        mFree(dst->data);
    dst->data = data;
    dst->num = num1 + num2;
}

void mMemoryIndex(MMemory *memory,int num,int size,void *index[])
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    mException(((num<=0)||(size<=0)),EXIT,"invalid input");
    mException(INVALID_POINTER(index),EXIT,"invalid input");
    
    size = ((size+15)>>4)<<4;
    
    int i;
    int n = 0;
    for(i=0;i<memory->num;i++)
    {
        int block_size = MEMORY_SIZE(memory,i);
        if(block_size < size)
            continue;
        
        char *block = (char *)(memory->data[i]);
        
        index[n] = (void *)((((intptr_t)block + 15)>>4)<<4);
        block_size = (block + block_size)-(((char *)(index[n]))+size);
        if(block_size > 0) {n = n +1;if(n>=num)return;}
        
        while(block_size >= size)
        {
            index[n] = ((char *)(index[n-1])) + size;
            block_size = block_size - size;
            n = n +1;if(n>=num)return;
        }
    }
    
    mMemoryAppend(memory,num-n,size);
    char *block = (char *)(memory->data[i]);
    
    index[n] = (void *)((((intptr_t)block + 15)>>4)<<4);
    n=n+1;
    
    while(n<num)
    {
        index[n] = ((char *)(index[n-1])) + size;
        n = n +1;
    }
}

void *mMemoryWrite(MMemory *memory,void *data,int size)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    
    MHandle *hdl = memory->handle->data[0];
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = hdl->handle;
    
    MemoryWrite_Check:
    if(handle->mem_size < size)
    {
        handle->mem_idx += 1;
        if(handle->mem_idx<memory->num)
        {
            handle->mem_size = MEMORY_SIZE(memory,handle->mem_idx);
            handle->pdata = memory->data[handle->mem_idx];
            goto MemoryWrite_Check;
        }
        else
        {
            handle->mem_size = MAX(8192,(size<<6));
            handle->pdata =mMemoryAppend(memory,1,handle->mem_size);
        }
    }

    if(data!=NULL) memcpy(handle->pdata,data,size);
    else           memset(handle->pdata,   0,size);
    char *memory_data = handle->pdata;
    handle->pdata = handle->pdata + size;
    handle->mem_size = handle->mem_size - size;
    
    return ((void *)memory_data);
}
