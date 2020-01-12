/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "morn_util.h"

// int *debug=NULL;

int *morn_mem_data[32] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
int morn_mem_size[32];
      
struct MemChain
{
    int size;
    int *ptr;
    struct MemChain *next;
};
struct MemChain morn_mem_chain_list[32][64];
struct MemChain *morn_mem_chain[32] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
void MemCollect(int index)
{
    int *data = morn_mem_data[index];
    struct MemChain *chain_list = &(morn_mem_chain_list[index][0]);
    memset(chain_list,0,64*sizeof(struct MemChain));
    struct MemChain *chain=NULL;
    
    int n=0;
    int *mem = data;
    while(mem<data+2048)
    {
        if(*mem <=0)
        {
            int *next = mem -(*mem)/sizeof(int)+1;
            while(*next <0)
            {
                if(next >= data + 2048) break;
                *mem = *mem + *next-sizeof(int);
                next = next -(*next)/sizeof(int)+1;
            }
            
            if(0-(*mem) >= 128)
            {
                chain_list[n].size = sizeof(int)-(*mem);
                chain_list[n].ptr = (mem);
                if(n==0)
                {
                    chain_list[n].next = NULL;
                    chain = &(chain_list[n]);
                }
                else if(chain_list[n].size>=chain->size)
                {
                    chain_list[n].next = chain;
                    chain = &(chain_list[n]);
                }
                else
                {
                    struct MemChain *p = chain;
                    while((p->next!=NULL)&&(p->next->size>chain_list[n].size)) p=p->next;
                    chain_list[n].next = p->next;
                    p->next = &(chain_list[n]);
                }
                
                n=n+1;
            }
            
            mem = mem -(*mem)/sizeof(int)+1;
        }
        else
            mem = mem +(*mem)/sizeof(int)+1;
    }
    morn_mem_chain[index] = chain;
}

pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

void *mMemAlloc(int size)
{
    pthread_mutex_lock(&mem_mutex);
    
    int *mem;
    if(size >= 1024)
    {
        mem = (int *)malloc(size+sizeof(int)*4);
        mem[3] = size;mem=mem+4;
        goto Malloc_end;
    }
    
    size = (((size+3)>>2)<<2);
    
    for(int i=0;i<32;i++)
    {
        if(morn_mem_chain[i] == NULL)
        {
            if(morn_mem_data[i] != NULL)
                MemCollect(i);
            continue;
        }
        struct MemChain *chain = morn_mem_chain[i];
        if(chain->size<=size) continue;
        
        mem = chain->ptr+1;
        morn_mem_size[i] += size+sizeof(int);
        
        chain->ptr[0] = size;
        chain->ptr = chain->ptr + size/sizeof(int) +1;
        chain->size -= (size+sizeof(int));
        if(chain->size>0)
            chain->ptr[0] = sizeof(int)-chain->size;
        
        if(chain->next == NULL) goto Malloc_end;
        if(chain->size > chain->next->size) goto Malloc_end;
        
        morn_mem_chain[i] = chain->next;
        if(chain->size<128) goto Malloc_end;
        
        struct MemChain *p = chain;
        while((p->next!=NULL)&&(p->next->size>chain->size)) p=p->next;
        chain->next = p->next;p->next = chain;
        goto Malloc_end;
    }
    
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] != NULL) continue;
        
        morn_mem_data[i] = (int *)malloc(2048*sizeof(int));
        morn_mem_size[i] = size+sizeof(int);
        mem = morn_mem_data[i]+1; mem[-1]=size;
        
        struct MemChain *chain = &(morn_mem_chain_list[i][0]);
        chain->size = 2047*sizeof(int)-size;
        chain->ptr = mem + size/sizeof(int);
        chain->ptr[0] = sizeof(int)-chain->size;
        chain->next=NULL;
        morn_mem_chain[i] = chain;
        
        goto Malloc_end;
    }
    
    for(int i=0;i<32;i++)
    {
        if((int)(2048*sizeof(int))-morn_mem_size[i] > size+512)
            MemCollect(i);
    }
    mem = (int *)malloc(size+sizeof(int)*4);
    mem[3] = size;mem=mem+4;
    
    Malloc_end:
    pthread_mutex_unlock(&mem_mutex);
    
    return mem;
}

void mMemFree(void *data)
{
    pthread_mutex_lock(&mem_mutex);
    // printf("data is %p\n",data);
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] == NULL) continue;
        // printf("morn_mem_data[%d] is %p~%p\n",i,morn_mem_data[i],morn_mem_data[i] + 2048);
        if(((int *)data>=morn_mem_data[i])&&((int *)data < morn_mem_data[i] + 2048))
        {
            int *mem = (int *)data -1;
            
            // printf("data is %p,mem_size is %d\n",data,mem[0]);
            morn_mem_size[i] -= ((*mem)+sizeof(int));
            // if(i==23) printf("%d:%p,%p,qqqqqqqqqqqqqq %p qqqqqqq free size is %d,morn_mem_size[i] is %d\n",i,morn_mem_data[i],morn_mem_data[i] + 2048,data,(*mem)+sizeof(float),morn_mem_size[i]);
            
            mException((*mem<0)||(morn_mem_size[i]<0),EXIT,"invalid operate with address %p,size is %d,%d,%d",data,*mem,morn_mem_size[i],i);
            *mem = 0-(*mem);
            // printf("bbmorn_mem_size[%d] is %d\n",i,morn_mem_size[i]);
            if(morn_mem_size[i] == 0)
            {
                // printf("bbbbbbbbbbb %d\n",i);
                free(morn_mem_data[i]);
                morn_mem_data[i] = NULL;
                morn_mem_chain[i]= NULL;
            }
            goto Free_end;
        }
    }
    free(((int *)data)-4);
    
    Free_end:
    pthread_mutex_unlock(&mem_mutex);
}

/*
int morn_mem_count[32] = {0};
void *mMalloc(int size)
{
    pthread_mutex_lock(&mem_mutex);
    
    int *mem;
    
    if(size >= 8192)
    {
        mem = (int *)malloc(size+sizeof(int)*4);
        mem[3] = size;
        pthread_mutex_unlock(&mem_mutex);
        return (mem+4);
    }
    
    size = ((((size-1)/sizeof(int))+1)+1)*sizeof(int);
    
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] != NULL)
        {
            if(16384*sizeof(int)-morn_mem_size[i] < size+128)
                continue;
            
            mem = morn_mem_data[i];
            while(mem < morn_mem_data[i]+16384)
            {
                if((*mem) == (int)(0-size+sizeof(int)))
                {
                    *mem = size-sizeof(int);
                    morn_mem_size[i] += size;
                    pthread_mutex_unlock(&mem_mutex);
                    return mem+1;
                }
                if((*mem) < (int)(0-size+sizeof(int)))
                {
                    int *next = (int *)((char *)mem + size);
                    *next = *mem + size; 
                    
                    *mem = size-sizeof(int);
                    morn_mem_size[i] += size;
                    
                    pthread_mutex_unlock(&mem_mutex);
                    return mem+1;
                }
                mem = (int *)((char *)mem + ABS(*mem)+sizeof(int));
            }
        }
    }
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] == NULL)
        {
            morn_mem_data[i] = (int *)malloc(16384*sizeof(int));
            morn_mem_size[i] = size;
            
            mem = morn_mem_data[i];
            *mem = size-sizeof(int);
            
            int *next = (int *)((char *)mem + size);
            *next = size - 16383*sizeof(int);
            
            pthread_mutex_unlock(&mem_mutex);
            return mem+1;
        }
    }
    
    mem = (int *)malloc(size+3*sizeof(int));
    mem[3] = size;
    pthread_mutex_unlock(&mem_mutex);
    return (mem+4);
}

void mFree(void *data)
{
    pthread_mutex_lock(&mem_mutex);
    
    int *mem;
    
    // printf("data is %p,%d\n",data,data);
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] != NULL)
        {
            // printf("morn_mem_data[%d] is %p~%p\n",i,morn_mem_data[i],morn_mem_data[i] + 16384);
            if(((int *)data>=morn_mem_data[i])&&((int *)data < morn_mem_data[i] + 16384))
            {
                mem = (int *)data -1;
                morn_mem_count[i] += 1;
                morn_mem_size[i] -= (*mem+sizeof(int));

                *mem = 0-(*mem);
                
                int *next = (int *)((char *)mem - (*mem)+sizeof(int));
                if(((*next)<0)&&(next<morn_mem_data[i]+16384))
                    *mem = *mem + *next -sizeof(int);
                
                if(morn_mem_count[i]>=8)
                {
                    mem = morn_mem_data[i];
                    while(mem < (int *)data)
                    {
                        // printf("mem is %d,%d\n",mem,*mem);
                        if(*mem <=0)
                        {
                            next = (int *)((char *)mem - (*mem)+sizeof(int));
                            if(next>(int *)data)
                                break;
                            if(*next <=0)
                                *mem = *mem + *next-sizeof(int);
                            else
                                mem = next;
                        }
                        else
                            mem = (int *)((char *)mem + (*mem)+sizeof(int));
                    }
                    morn_mem_count[i]=0;
                }
                
                
                if(morn_mem_size[i] == 0)
                // if(*(morn_mem_data[i]) == 0-16383*sizeof(int))
                {
                    // printf("aaaaaaaaaaa\n");
                    free(morn_mem_data[i]);
                    morn_mem_data[i] = NULL;
                }
                
                pthread_mutex_unlock(&mem_mutex);
                return;
            }
        }
    }
    // printf("data is %p,%p,%d,size is %x\n",((int *)data)-4,data,data,*((int *)data-1));
    free(((int *)data)-4);
    pthread_mutex_unlock(&mem_mutex);
}
*/

/*

void **morn_malloc_ptr[256];
int morn_malloc_num[256];

int *morn_mem_data[32];
int morn_mem_size[32];
int morn_mem_count[32] = {0};

pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MEM_LAYER(Flag) ((Flag)>>16)
#define MEM_SIZE(Flag)  ((Flag)&0x0FFFFFFFF)

void AddPtr(void *ptr)
{
    if(morn_layer_order<0) return;
   
    int num = morn_malloc_num[morn_layer_order];
    if(num % 256 == 0)
    {
        int n = num/256 + 1;
        void **buff = malloc(n*256*sizeof(void *));
        if(n>1)
        {
            memcpy(buff,morn_malloc_ptr[morn_layer_order],(n-1)*256*sizeof(void *));
            free(morn_malloc_ptr[morn_layer_order]);
        }
        morn_malloc_ptr[morn_layer_order] = buff;
    }
    morn_malloc_ptr[morn_layer_order][num] = ptr;
    morn_malloc_num[morn_layer_order] = num+1;
}

void DeletePtr(void *ptr)
{
    int i,j;
    for(j=morn_layer_order;j>=0;j--)
    {
        int num = morn_malloc_num[j];
        for(i=0;i<num;i++)
        {
            if(morn_malloc_ptr[j][i] == ptr)
                break;
        }
        if(i<num)
        {
            morn_malloc_ptr[j][i] = morn_malloc_ptr[j][num-1];
            morn_malloc_num[j] = num-1;
            return;
        }
    }
}

void *mMalloc(int size)
{
    pthread_mutex_lock(&mem_mutex);
    
    int *mem;
    
    if(size > 8192)
    {
        mem = (int *)malloc(size+sizeof(int)*4);
        mem[3] = size;
        AddPtr(mem+4);
        pthread_mutex_unlock(&mem_mutex);
        return (mem+4);
    }
    
    size = ((((size-1)/sizeof(int))+1)+1)*sizeof(int);
    
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] != NULL)
        {
            if(16384*sizeof(int)-morn_mem_size[i] < size+128)
                continue;
            
            mem = morn_mem_data[i];
            while(mem < morn_mem_data[i]+16384)
            {
                if((*mem <0)&&(MEM_SIZE(*mem) == size))
                {
                    *mem = (size-sizeof(int)) + (morn_layer_order<<16);
                    
                    morn_mem_size[i] += size;
                    pthread_mutex_unlock(&mem_mutex);
                    return mem+1;
                }
                if((*mem <0)&&(MEM_SIZE(*mem) > size))
                {
                    int *next = mem + size/sizeof(int);
                    *next = *mem - size; 
                    *mem = (size-sizeof(int)) + (morn_layer_order<<16);
                    
                    morn_mem_size[i] += size;
                    pthread_mutex_unlock(&mem_mutex);
                    return mem+1;
                }
                mem = mem + MEM_SIZE(*mem)/sizeof(int)+1;
            }
        }
    }
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] == NULL)
        {
            morn_mem_data[i] = (int *)malloc(16384*sizeof(int));
            morn_mem_size[i] = size;
            
            mem = morn_mem_data[i];
            *mem = (size-sizeof(int)) + (morn_layer_order<<16);
            
            int *next = mem + size/sizeof(int);
            *next = (16384*sizeof(int)-size-sizeof(int)) + ((-1)<<16);
            
            pthread_mutex_unlock(&mem_mutex);
            return mem+1;
        }
    }
    
    mem = (int *)malloc(size+3*sizeof(int));
    mem[3] = size;
    AddPtr(mem+4);
    pthread_mutex_unlock(&mem_mutex);
    return (mem+4);
}

void mFree(void *data)
{
    pthread_mutex_lock(&mem_mutex);
    
    int *mem;
    
    // printf("data is %p,%d\n",data,data);
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] != NULL)
        {
            // printf("morn_mem_data[%d] is %d~%d\n",i,morn_mem_data[i],morn_mem_data[i] + 16384);
            if(((int *)data>=morn_mem_data[i])&&((int *)data < morn_mem_data[i] + 16384))
            {
                mem = (int *)data -1;
                morn_mem_count[i] += 1;
                morn_mem_size[i] -= (*mem+sizeof(int));
                if(morn_mem_size[i] == 0)
                {
                    free(morn_mem_data[i]);
                    morn_mem_data[i] = NULL;
                    pthread_mutex_unlock(&mem_mutex);
                    return;
                }
                
                int *next = mem + MEM_SIZE(*mem)/sizeof(int) +1;
                if(((*next)<0)&&(next<morn_mem_data[i]+16384))
                    *mem = MEM_SIZE(*mem) + MEM_SIZE(*next) + sizeof(int)+((-1)<<16);
                else
                    *mem = MEM_SIZE(*mem)                                +((-1)<<16);
                
                if(morn_mem_count[i]>=8)
                {
                    mem = morn_mem_data[i];
                    while(mem < (int *)data)
                    {
                        // printf("mem is %d,%d\n",mem,*mem);
                        if(*mem <0)
                        {
                            next = mem + MEM_SIZE(*mem)/sizeof(int) + 1;
                            if(next>(int *)data)
                                break;
                            if(*next < 0)
                                *mem = MEM_SIZE(*mem) + MEM_SIZE(*next) + sizeof(int) + ((-1)<<16);
                            else
                                mem = next;
                        }
                        else
                            mem = mem + MEM_SIZE(*mem)/sizeof(int)+1;
                    }
                    morn_mem_count[i]=0;
                }
                
                pthread_mutex_unlock(&mem_mutex);
                return;
            }
        }
    }
    // printf("data is %p,%p,%d,size is %x\n",((int *)data)-4,data,data,*((int *)data-1));
    DeletePtr(((int *)data)-4);
    free(((int *)data)-4);
    pthread_mutex_unlock(&mem_mutex);
}

int mInvalidPoter(void *ptr,int size)
{
    if(ptr==NULL) return 1;
    if((intptr_t)ptr&0x0F != 0) return 1;
    if(size<8192)
    {
        if(MEM_LAYER(((int *)ptr)[-1]) < 0) return 1;
        if(MEM_SIZE(((int *)ptr)[-1]) != size) return 1;
    }
    else
    {
        if(((int *)ptr)[-1] != size) return 1;
    }
    
    return 0;
}
    

void AllFree(int order)
{
    for(int j=order;j<256;j++)
    {
        int num =morn_malloc_num[j];
        
        for(int i=0;i<num;i++)
            free(morn_malloc_ptr[j][i]);
        
        if(morn_malloc_ptr[j] != NULL)
        {
            free(morn_malloc_ptr[j]);
            morn_malloc_ptr[j] = NULL;
        }
    }
    
    pthread_mutex_lock(&mem_mutex);
    
    for(int i=0;i<32;i++)
    {
        if(morn_mem_data[i] == NULL) continue;
        
        int *mem = morn_mem_data[i];
        while(1)
        {
            int *next = mem + MEM_SIZE(*mem)/sizeof(int) + 1;
            if(next>=morn_mem_data[i] + 16384) break;
           
            if(MEM_LAYER(*mem)>order||(*mem<0))
            {
                if(MEM_LAYER(*next)||(*next<0))
                {
                    *mem = MEM_SIZE(*mem) + MEM_SIZE(*next) + sizeof(int) + ((-1)<<16);
                    
                    morn_mem_size[i] -= (*mem+sizeof(int));
                    if(morn_mem_size[i] == 0)
                    {
                        free(morn_mem_data[i]);
                        morn_mem_data[i] = NULL;
                        break;
                    }
                    next = next + MEM_SIZE(*next)/sizeof(int) + 1;
                }
                else
                    mem = next;
            }
            else
                mem = next;
        }
        morn_mem_count[i]=0;
    }
    pthread_mutex_unlock(&mem_mutex);
}
*/

int morn_layer_order[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int morn_exception = 0;
jmp_buf *morn_jump[16][8];
pthread_t morn_pthread_ID[16];

/*
#define mExceptionBegin()\
{\
    morn_layer_order+=1;\
    morn_malloc_num[morn_layer_order] = 0;\
    jmp_buf buf_jump;\
    morn_jump[morn_layer_order] = &buf_jump;\
    if(setjmp(buf_jump))\
        goto MORN_EXCEPTION_END;\
}

#define mExceptionEnd()\
{\
    MORN_EXCEPTION_END:\
    morn_layer_order -= 1;\
}

#define mException(ERROR,ID,...) {\
    if(ERROR < 0)\
    {\
        printf("[%s,line %d]Warning: in function %s:",__FILE__,__LINE__,__FUNCTION__);\
        printf(__VA_ARGS__);\
    }\
    else if(ERROR > 0)\
    {\
        printf("[%s,line %d]Warning: in function %s:",__FILE__,__LINE__,__FUNCTION__);\
        printf(__VA_ARGS__);\
        morn_exception = ID;\
        if(morn_layer_order >= 0)\
            longjmp(*(morn_jump[morn_layer_order]),morn_exception);\
        else\
            exit(0);\
    }\
}
*/

pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef DEBUG
struct MemoryElement
{
    char info[128];
    void *ptr;
    int size;
    int state;
};

void *morn_pointer;
struct MemoryElement **morn_memory_list = NULL;
int morn_memory_num = 0;
int morn_memory_valid_num = 0;

void *MemoryListSet(int size,const char *file,int line,const char *func)
{
    pthread_mutex_lock(&debug_mutex);
    
    void *ptr = mMemAlloc(size+4);
    // printf("ptr is %p\t",ptr);
    struct MemoryElement *element;
    int row,col;
    if((morn_memory_num == 0)&&(morn_memory_list == NULL))
    {
        morn_memory_list = (struct MemoryElement **)malloc(1024*sizeof(struct MemoryElement *));
        memset(morn_memory_list,0,1024*sizeof(struct MemoryElement *));
    }
    row = morn_memory_num/1024;
    col = morn_memory_num%1024;
    if((col == 0)&&(morn_memory_list[row] == NULL))
        morn_memory_list[row] = (struct MemoryElement *)malloc(1024*sizeof(struct MemoryElement));

    element = morn_memory_list[row]+col;
    sprintf(element->info,"%s:%d %s",file,line,func);
    element->size =  *((int *)ptr -1);
    element->state = 1;
    element->ptr = ptr;
    *((char *)ptr + element->size-4) = -1;
    morn_memory_valid_num++;
    morn_memory_num++;
    
    pthread_mutex_unlock(&debug_mutex);
    // printf("ptr is %p,morn_memory_num is %d\n",ptr,morn_memory_num);
    return ptr;
}

void MemoryListUnset(void *ptr,const char *file,int line,const char *func)
{
    pthread_mutex_lock(&debug_mutex);
    // printf("free morn_memory_num is %d\n",morn_memory_num);
    int i;
    int size = *((int *)ptr -1);
    for(i=0;i<morn_memory_num;i++)
    {
        int row = i/1024;
        int col = i%1024;
        struct MemoryElement *element = morn_memory_list[row]+col;
        if(element->ptr == ptr)
        {
            // printf("size is %d,element->size is %d\n",size,element->size);
            if(element->state == 1)
            {
                if((*((char *)ptr + element->size-4) != -1)||(element->size != size))
                {
                    printf("[%s,line %d]Error: in function %s: memory over used\n",file,line,func);
                    exit(0);
                }
                element->state = 0;
                morn_memory_valid_num --;
                break;
            }
        }
    }
    if(i==morn_memory_num)
    {
        printf("morn_memory_num is %d\n",morn_memory_num);
        printf("[%s,line %d]Error: in function %s: free invalid memory\n",file,line,func);
        exit(0);
    }
   
    if(morn_memory_valid_num == 0)
    {
        for(i=0;i<1024;i++) 
        {
            if(morn_memory_list[i] != NULL) 
                free(morn_memory_list[i]);
        }
        free(morn_memory_list);
        morn_memory_list = NULL;
        morn_memory_num = 0;
    }
    mMemFree(ptr);
    pthread_mutex_unlock(&debug_mutex);
}
#endif
void MemoryListPrint(int state)
{
    #ifdef DEBUG
    // printf("print morn_memory_num is %d\n",morn_memory_num);
    for(int i=0;i<morn_memory_num;i++)
    {
        int row = i/1024;
        int col = i%1024;
        struct MemoryElement *element = morn_memory_list[row]+col;
        if((state<0)||(state==element->state))
            printf("%d:[%s] ptr is 0x%p,size is %d,state is %d\n",i,element->info,element->ptr,element->size-4,element->state);
    }
    
    for(int i=0;i<32;i++)
    {
        if(morn_mem_chain[i] != NULL)
            printf("morn_mem_chain[i] is %p\n",morn_mem_chain[i]);
    }
    
    printf("memory not free %d\n",morn_memory_valid_num);
    printf("memory list over\n");
    #endif
}



/*
void main()
{
    int *data[5];
    
    data[0] = (int *)mMalloc(50);
    printf("data[0] is %d\n",data[0]);
    
    data[1] = (int *)mMalloc(100);
    printf("data[1] is %d\n",data[1]);
    
    data[2] = (int *)mMalloc(500);
    printf("data[2] is %d\n",data[2]);
    
    mFree(data[1]);
    
    data[3] = (int *)mMalloc(70);
    printf("data[3] is %d\n",data[3]);
    
    data[4] = (int *)mMalloc(10);
    printf("data[4] is %d\n",data[4]);
    
    mFree(data[0]);
    mFree(data[2]);
    mFree(data[3]);
    mFree(data[4]);
}
*/



