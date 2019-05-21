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
    char *memory_data = handle->pdata;
    handle->pdata = handle->pdata + size;
    handle->mem_size = handle->mem_size - size;
    
    return ((void *)memory_data);
}











/*
MMemoryBlock *mMemoryBlockCreate(int size)
{
    MMemoryBlock *block;
    block = (MMemoryBlock *)mMalloc(sizeof(MMemoryBlock));
    
    if(size <0)
    {
        block->size = 0;
        block->memory = NULL;
    }
    else
    {
        block->size = ((size+3)>>2)<<2;
        block->memory = (char *)mMalloc(block->size);
    }

    block->last = block;
    block->next = block;
    
    return block;
}

void mMemoryBlockRelease(MMemoryBlock *block)
{
    (block->last)->next = block->next;
    (block->next)->last = block->last;
    
    if(!INVALID_POINTER(block->memory))
        mFree(block->memory);
    
    mFree(block);
}

void mMemoryBlockRedefine(MMemoryBlock *block,int size)
{
    if(size<block->size)
        return;
    
    block->size = size;
    mFree(block->memory);
    block->memory = mMalloc(size);
}

void mMemoryBlockSet(MMemoryBlock *block,char value)
{
    memset(block->memory,value,block->size);
}

void mMemoryBlockExchange(MMemoryBlock *block1,MMemoryBlock *block2)
{
    int buff;
    char *p;
    
    buff = block1->size;
    block1->size = block2->size;
    block2->size = buff;
    
    p = block1->memory;
    block1->memory = block2->memory;
    block2->memory = p;
}

void mMemoryBlockDelete(MMemoryBlock *mem)
{
    mem->last->next = mem->next;
    mem->next->last = mem->last;
    
    mem->last = mem;
    mem->next = mem;
}

void mMemoryBlockReplm(MMemoryBlock *mem1,MMemoryBlock *mem2)
{
    if(mem1->last != mem1)
    {
        mem1->last->next = mem2;
        mem2->last = mem1->last;
        mem1->last = mem1;
    }
    if(mem1->next != mem1)
    {
        mem1->next->last = mem2;
        mem2->next = mem1->next;
        mem1->next = mem1;
    }
}

// MMemoryBlock *mMemoryBlockSearch(MMemory *mem,int flag)
// {
    // MMemoryBlock *block;
    
    // if(INVALID_POINTER(mem))
        // return NULL;
    
    // block = mem;
    // while(1)
    // {
        // if(block->flag == flag)
            // return block;
        
        // block = (block->next);
        // if(block == mem)
            // return NULL;
    // }
// }

void mMemoryRelease(MMemory *mem)
{
    MMemoryBlock *block;
    MMemoryBlock *p;
   
    if(INVALID_POINTER(mem))
        return;
    
    block = mem;
    while(1)
    {
        p = (block->next);
        
        if(!INVALID_POINTER(block->memory))
            mFree(block->memory);    
        mFree(block);
        
        block = p;
        if(block == mem)
            return;
    }
}

int mMemorySize(MMemory *mem)
{
    MMemoryBlock *block;
    int vol;
    
    vol = 0;
    block = mem;
    while(1)
    {
        vol = vol + mem->size;
        block = block->next;
        if(block == mem)
            return vol;
    }
}

MMemory *mMemoryMerge(int mem_num,MMemory *mem,...)
{
    MMemory *memory;
    MMemoryBlock *merge;
    MMemoryBlock *block;
    int para_num;
    
    va_list para;
    
    mException((mem_num <= 0),"invalid input",EXIT);
    if(mem_num == 1)
        return mem;
        
    para_num = 1;
    va_start(para,mem);
    
    while(1)
    {
        if(!INVALID_POINTER(mem))
            break;
        mem = va_arg(para,MMemoryBlock *);
        para_num = para_num +1;
        
        
        if(para_num >= mem_num)
        {
            va_end(para);
            return NULL;
        }
    }
    
    memory = mem;
    mem = mem->last;
    
    while(1)
    {
        merge = va_arg(para,MMemoryBlock *);
        para_num = para_num +1;
        
        if(!INVALID_POINTER(merge))
        {
            block = merge->last;
            mem->next = merge;
            merge->last = mem;
            mem = block;
        }
        
        if(para_num >= mem_num)
        {
            va_end(para);
            break;
        }
    }
    mem->next = memory;
    memory->last = mem;
    
    return memory;
}

void mMemoryInsert(MMemoryBlock *last,MMemory *mem,MMemoryBlock *next)
{
    MMemoryBlock *mem2,*mem1;
    
    mException(INVALID_POINTER(mem),"invalid input",EXIT);
    mException((INVALID_POINTER(last))&&(INVALID_POINTER(next)),"invalid input",EXIT);
    
    if(INVALID_POINTER(next))
        next = last->next;
    else if(INVALID_POINTER(last))
        last = next->last;
    else
        mException(((last->next!=next)||(next->last!=last)),"invalid input",EXIT);
    
    mem1 = mem;
    mem2 = mem->last;
    
    last->next = mem1;
    next->last = mem2;
    mem1->last = last;
    mem2->next = next;
}

void mMemoryIndex(MMemory *mem,int frame_size,char *index[],int index_num)
{
    MMemoryBlock *p;
    int size;
    int num;

    mException((INVALID_POINTER(mem)||(frame_size<=0)||INVALID_POINTER(index)||(index_num<=0)),"invalid input",EXIT);
    
    frame_size = ((frame_size+15)>>4)<<4;
    
    // size = mem->size;
    
    p = mem;
    num = 0;
    while(1)
    {
        if(p->size >= frame_size)
        {
            index[num] = p->memory;
            size = 16-((intptr_t)(index[num])&0x0F);
            index[num] = index[num]+size;
            
            num = num+1;
            size = p->size - frame_size -size;
            while(num<index_num)
            {
                if(size<frame_size)
                    break;
                
                index[num] = index[num-1] + frame_size;
                num = num+1;
                
                size = size - frame_size;
            }
        }
        if(num >= index_num)
            return;
        
        p = p->next;

        if(p == mem)
            break;
    }
    
    size = (index_num - num)*frame_size+16;
    p = mMemoryBlockCreate(size);
    mMemoryInsert(NULL,p,mem);
    
    index[num] = p->memory;
    size = 16-((intptr_t)(index[num])&0x0F);
    index[num] = index[num]+size;
    num = num +1;
    while(num < index_num)
    {
        index[num] = index[num-1] + frame_size;
        num = num+1;
    }
}

int mMemoryCheck(MMemory *mem,void *point)
{
    MMemory *block;
    
    if((mem==NULL)||(point == NULL))
        return 0;
    
    block = mem;
    do
    {
        if(((char *)point >= block->memory)&&((char *)point < block->memory + block->size))
            return 1;
        
        block = block->next;
    }while(block != mem);
        
    return 0;
}

struct HandleMemory
{
    MMemoryBlock *mem;
    char *pdata;
}
void endMemoryWrite(void *info) {NULL;}
#define HASH_MemoryWrite 0x9de36b79
void *mMemoryWrite(MObject *memory,void *data,int size)
{
    int handle_index;
    mHandle(memory,MemoryWrite,handle_index);
    struct HandleMemory *handle = (struct HandleMemory *)(memory->handle->handle[handle_index]);
    if(memory->handle->valid[handle_index] == 0)
    {
        handle->mem = mMemoryBlockCreate(MAX(256,(size<<4)));
        if(memory->object == NULL)
            memory->object = handle->mem;
        else
            mMemoryBlockInsert(NULL,handle->mem,(MMemoryBlock *)(memory->object));
        
        handle->pdata = handle->mem->memory;
        
        memory->handle->valid[handle_index] = 1;
    }
    
    MMemoryBlock *mem = handle->mem;
    if(handle->pdata+size >= mem->memory + mem->size)
    {
        mem = mMemoryBlockCreate(MAX(256,(size<<4));
        mMemoryBlockInsert(NULL,mem,handle->mem);
        handle->mem = mem;
        handle->pdata = mem->memory;
    }
    
    if(data!=NULL)
        memcpy(handle->pdata,data,size);
    
    char *memory_data = handle->pdata;
    handle->pdata = handle->pdata + size;
    
    return ((void *)memory_data);
}
        
            

void PrintMemory(MMemory *mem)
{
    MMemoryBlock *block;
    block = mem;
    while(1)
    {
        printf("mem is %p,size is %8d,last is %p,next is %p\t",block,block->size,block->last,block->next);
        printf("address is from %p to %p\n",block->memory,block->memory+block->size);
        block = block->next;
        if(block == mem)
            return;
    }
}
*/

/*
void main()
{
    MMemoryBlock *mem;
    int size;
    int *index[15];
    int i;
    
    mem = mCreateMemoryBlock(1024);
    mMemoryAppend(mem,512);
    
    size = mMemorySize(mem);
    printf("size is %d\n",size);
    
    mRedefineMemory(mem,1801);
    size = mMemorySize(mem);
    printf("size is %d\n",size);
    
    mRedefineMemory(mem,1200);
    size = mMemorySize(mem);
    printf("size is %d\n",size);
    
    mMemoryIndex(mem,200,(void **)index,15);
    for(i=0;i<15;i++)
        printf("index[%d] is %d\n",i,index[i]);
    
    mMemoryRelease(mem);
}
*/

/*
void main()
{
    MMemoryBlock *block1,*block2,*block3;
    int size;
    int *index[15];
    int i;
    
    MMemoryBlock *mem;
    
    block1 = mCreateMemoryBlock(1200);
    block2 = mCreateMemoryBlock(100);
    block3 = mCreateMemoryBlock(100);
    
    mMemoryMerge(3,block1,block2,block3);
    
    mem = block1;
    while(mem!=NULL)
    {
        printf("mem is %d,mem size is %d,address is %d,last is %d,next is %d\n",mem,mem->size,mem->memory,mem->last,mem->next);
        mem = mem->next;
    }
    
    mMemoryBlockDelete(block2);
    
    mem = block1;
    while(mem!=NULL)
    {
        printf("mem is %d,mem size is %d,address is %d,last is %d,next is %d\n",mem,mem->size,mem->memory,mem->last,mem->next);
        mem = mem->next;
    }
    
    mem = block1;
    mMemoryIndex(mem,200,(void **)index,15);
    for(i=0;i<15;i++)
        printf("index[%d] is %d\n",i,index[i]);
    
    while(mem!=NULL)
    {
        printf("mem is %d,mem size is %d,address is %d,last is %d,next is %d\n",mem,mem->size,mem->memory,mem->last,mem->next);
        mem = mem->next;
    }
    
    mMemoryRelease(block1);
}
*/

/*
void main()
{
    MMemoryBlock *block1,*block2,*block3,*block4,*block5,*block6;
    int i;
    
    MMemoryBlock *my_mem;
    
    block1 = mMemoryBlockCreate(100,0);
    block2 = mMemoryBlockCreate(200,0);
    block3 = mMemoryBlockCreate(300,0);
    block4 = mMemoryBlockCreate(400,4);
    block5 = mMemoryBlockCreate(500,0);
    block6 = mMemoryBlockCreate(600,0);
    
    my_mem = mMemoryCreate(3,NULL,block1,block2);
    printf("\ninit:\n");
    PrintMemory(my_mem);
    
    mMemoryMerge(5,block3,NULL,block4,block5,block6);
    // PrintMemory(block3);
    mMemoryInsert(block1,block3,block2);
    printf("\nbefore:\n");
    PrintMemory(my_mem);
    
    mMemoryBlockDelete(block1);
    // mMemoryInsert(NULL,block6,my_mem);
    // mMemoryBlockExchange(block1,block5);
    printf("\nafter:\n");
    PrintMemory(my_mem);
    
    mMemoryRelease(my_mem);
}
*/
