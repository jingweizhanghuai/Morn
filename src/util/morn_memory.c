/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_util.h"

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
        mException((mem==NULL),EXIT,"malloc error");
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
        mException((morn_mem_data[i]==NULL),EXIT,"malloc error");
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
    mException((mem==NULL),EXIT,"malloc error");
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
    *((int8_t *)ptr + element->size-4) = -1;
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
                if((*((int8_t *)ptr + element->size-4) != -1)||(element->size != size))
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

struct MAFBlock
{
    void *info;
    unsigned short size;
    unsigned short flag;
    int idlnum;
    int info_idx;
    int order;
    char data[0];
};
struct MAFInfo
{
    struct MAFBlock **block_list;
    struct MAFBlock *block;
    struct MAFBlock *spare;
    int block_num;
    int block_vol;
    int block_order;
};
__thread struct MAFInfo *morn_alloc_free_info=NULL;
short morn_alloc_free_size[32]={48,80,112,144,176,208,240,272,336,400,464,560,688,816,1008,1168,1328,1552,1840,2352,2864,3376,3888,4400,5424,6448,7472,9008,11056,13104,14736,16400};
char morn_alloc_free_size_idx[512]={0,1,2,3,4,5,6,7,8,8,9,9,10,10,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,14,14,15,15,15,15,15,16,16,16,16,16,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31};
unsigned short morn_alloc_free_ref_flag1[16]={0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};
unsigned short morn_alloc_free_ref_flag2[16]={0xFFFE,0xFFFD,0xFFFB,0xFFF7,0xFFEF,0xFFDF,0xFFBF,0xFF7F,0xFEFF,0xFDFF,0xFBFF,0xF7FF,0xEFFF,0xDFFF,0xBFFF,0x7FFF};

void *m_Malloc(int size)
{
    intptr_t *ptr;
    size=size+16;
    if(size>16392) {ptr=malloc(size); ptr[0]=0; return (((char *)ptr)+16);}
    
    int idx=morn_alloc_free_size_idx[(size-17)/32];
    size = morn_alloc_free_size[idx];
  
    if(morn_alloc_free_info==NULL)
    {
        morn_alloc_free_info=malloc(32*sizeof(struct MAFInfo));
        memset(morn_alloc_free_info,0,32*sizeof(struct MAFInfo));
    }
    struct MAFInfo *info  = morn_alloc_free_info+idx;
    
    struct MAFBlock *block= info->block;
    int invalid_block=(block==NULL);
    if(!invalid_block)invalid_block=(block->idlnum==0);

    if(invalid_block)
    {
        if(info->spare!=NULL)
        {
            info->block = info->spare;
            block = info->block;
            info->spare = NULL;
            invalid_block=0;
        }
        else
        {
            for(int i=0;i<info->block_vol;i++)
            {
            	if(info->block_list[i]==NULL) continue;
	            if(info->block_list[i]->idlnum==0) continue;
                info->block = info->block_list[i];
                block = info->block;
                invalid_block=0;
                break;
            }
        }
    }
    
    if(invalid_block)
    {
    	
        block = m_Malloc(sizeof(struct MAFBlock)+16*size);
        block->size = size;
        block->info = info;info->block =block;

        int n=info->block_num;
        if(n>=info->block_vol)
        {
            struct MAFBlock **list = malloc((n+64)*sizeof(struct MAFBlock *));
            if(info->block_list!=NULL)
            {
                memcpy(list,info->block_list,n*sizeof(struct MAFBlock *));
                free(info->block_list);
            }
            memset(list+n,0,64*sizeof(struct MAFBlock *));
            info->block_list=list;
            info->block_vol=n+64;
            info->block_order=n;
        }

        int m=info->block_order;
        while(1)
        {
        	if(info->block_list[m]==NULL) 
        	{
        		info->block_list[m]=block;
        		block->info_idx=m;
        		m=m+1;if(m>=info->block_vol)m=0;
        		info->block_order=m;
        		info->block_num = n+1;
        		break;
        	}
        	m=m+1;if(m>=info->block_vol) m=0;
        }

        block->flag=0x0001;block->idlnum=15;block->order=1;
        ptr=(intptr_t *)(block->data);
        ptr[0]=(intptr_t)(block);ptr[1]=0;
        return (((char *)ptr)+16);
    }

    block->idlnum--;
    unsigned short flag = block->flag;
    idx=block->order;
    while(1)
    {
        if((flag&morn_alloc_free_ref_flag1[idx])==0) 
        {
            block->flag  = flag | morn_alloc_free_ref_flag1[idx];
            block->order = ((idx+1)&0x0000000F);
            ptr = (intptr_t *)(block->data+size*idx);
            ptr[0]=(intptr_t)(block);ptr[1]=idx;
            return (((char *)ptr)+16);
        }
        idx=((idx+1)&0x0000000F);
    }
}

void m_Free(void *p)
{
    intptr_t *ptr=(intptr_t *)(((char *)p)-16);
    if(ptr[0]==0) {free(ptr);return;}

    struct MAFBlock *block = (struct MAFBlock *)(ptr[0]);
    int idx=ptr[1];
    
    block->order = idx;
    block->flag &= morn_alloc_free_ref_flag2[idx];
    block->idlnum =block->idlnum+1;

    struct MAFInfo *info = block->info;
    if(block!=info->block)
    {
    	if(block->idlnum==16)
    	{
    		info->spare = info->block;
    		info->block = block;
    		if(info->spare->idlnum==16)
    		{
	    		int n = info->spare->info_idx;
	        	info->block_list[n]=NULL;
	        	info->block_order=n;
	        	info->block_num=info->block_num-1;
	        	m_Free(info->spare);
	        	info->spare=NULL;
	        }
    	}
	    else
	    {
	             if(info->spare==NULL) info->spare=block;
	        else if(block->idlnum>info->spare->idlnum) info->spare=block;
	    }
	}
}

void endMAF()
{
	if(morn_alloc_free_info==NULL) return;
	for(int i=0;i<32;i++)
	{
		if(morn_alloc_free_info[i].block     !=NULL) m_Free(morn_alloc_free_info[i].block);
		if(morn_alloc_free_info[i].block_list!=NULL)   free(morn_alloc_free_info[i].block_list);
	}
	free(morn_alloc_free_info);
	morn_alloc_free_info=NULL;
}

