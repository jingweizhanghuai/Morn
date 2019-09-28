/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

struct HandleListCreate
{
    MList *list;
    int num;
    void **data;
    MMemory *memory;
};
void endListCreate(void *info)
{
    struct HandleListCreate *handle = (struct HandleListCreate *)info;
    mException((handle->list == NULL),EXIT,"invalid list");
    
    if(handle->memory !=NULL) mMemoryRelease(handle->memory);
    if(handle->data != NULL) mFree(handle->data);
    
    mFree(handle->list);
}
#define HASH_ListCreate 0xfa6c59f

MList *mListCreate(int num,void **data)
{
    MList *list = (MList *)mMalloc(sizeof(MList));
    memset(list,0,sizeof(MList));
    
    MHandle *hdl; ObjectHandle(list,ListCreate,hdl);
    struct HandleListCreate *handle = hdl->handle;
    handle->list = list;
    
    if(num<0) num = 0;
    handle->num = num;
    list->num = num;
    
    if(num>0)
    {
        handle->data = (void **)mMalloc(num*sizeof(void *));
        if(data!=NULL) memcpy(handle->data,data,num*sizeof(void *));
        else           memset(handle->data,   0,num*sizeof(void *));
    }
    else
        mException((data != NULL),EXIT,"invalid input");
    
    list->data = handle->data;
    
    return list;
}

void mListRelease(MList *list)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    
    if(!INVALID_POINTER(list->handle))
        mHandleRelease(list->handle);
}

void mListAppend(MList *list,int n)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    if(n<0) n=list->num+1;
    
    mHandleReset(list->handle);
    
    struct HandleListCreate *handle= ((MHandle *)(list->handle->data[0]))->handle;
    if(n<=handle->num) 
    {
        if(list->data!= handle->data)
        {
            if(list->num>0)
                memcpy(handle->data,list->data,list->num*sizeof(void *));
            list->data = handle->data;
        }
        list->num = n;
        return;
    }
    
    int num = list->num + MAX(MAX(128,n-list->num),(list->num)>>2);
    void **list_data = (void **)mMalloc(num*sizeof(void *));
    if(list->num>0)
        memcpy(list_data,list->data,(list->num)*sizeof(void *));
    memset(list_data+list->num,0,(num-list->num)*sizeof(void *));
    
    if(handle->data != NULL) mFree(handle->data);
    handle->data = list_data;
    handle->num = num;
    
    list->data = handle->data;
    list->num = n;
}

void mListPlace(MList *list,int num,int size)
{
    if(num<=0) return;
    mException((size<=0),EXIT,"invalid input list element size");
    
    list->num = 0;
    mListAppend(list,num);
    
    struct HandleListCreate *handle = ((MHandle *)(list->handle->data[0]))->handle;
    if(handle->memory == NULL) handle->memory = mMemoryCreate(num,size);
    mMemoryIndex(handle->memory,num,size,list->data);
}

struct HandleListWrite
{
    int write_size;
};
void endListWrite(void *info) {NULL;}
#define HASH_ListWrite 0x40aea976
void *mListWrite(MList *list,int n,void *data,int size)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    mException((n>list->num)||(INVALID_POINTER(data)),EXIT,"invalid write location");
    if(size<=0) size = strlen(data)+1;
    
    struct HandleListCreate *handle0 = ((MHandle *)(list->handle->data[0]))->handle;
    if(n<0) n = list->num;
    
    if(handle0->memory == NULL)
        handle0->memory = mMemoryCreate(DFLT,DFLT);
    
    if(n==list->num)
    {
        if(n<handle0->num) list->num = n+1;
        else mListAppend(list,DFLT);
        list->data[n] = mMemoryWrite(handle0->memory,data,size);
    }
    else
    {
        list->data[n] = mMemoryWrite(handle0->memory,data,size);
        MHandle *hdl; ObjectHandle(list,ListWrite,hdl);
        struct HandleListWrite *handle = hdl->handle;
        handle->write_size += size;
        if(handle->write_size>16384)
        {
            mMemoryCollect(handle0->memory,list->data,list->num);
            handle->write_size = 0;
        }
    }
    
    return list->data[n];
}

struct HandleListRead
{
    int order;
};
void endListRead(void *info) {NULL;}
#define HASH_ListRead 0x537cc305
void *mListRead(MList *list,int n,void *data,int size)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(list,ListRead,hdl);
    struct HandleListRead *handle = hdl->handle;
    if(hdl->valid == 0) handle->order = -1;
    hdl->valid = 1;
    
    if(n<0) n = handle->order+1;
    handle->order = n;
    
    if(n>=list->num) return NULL;
    
    if(data!=NULL)
    {
        if(size>0) memcpy(data,list->data[n],size);
        else       strcpy(data,list->data[n]);
    }
    return list->data[n];
}

void mListReorder(MList *list)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    
    void **data = list->data;
    int list_num = list->num;
    void *buff;
   
    int i;
    for(i=0;i<list_num;i++)
    {
        int j = mRand(0,list_num);
        buff = data[i]; data[i] = data[j]; data[j] = buff;
    }
}

void mListCopy(MList *src,MList *dst)
{
    mListAppend(dst,src->num);
    
    MHandle *hdl;
    ObjectHandle(src,ListCreate,hdl);
    struct HandleListCreate *src_handle = hdl->handle;
    if(src_handle->memory == NULL)
    {
        memcpy(dst->data,src->data,src->num*sizeof(void *));
        return;
    }
    
    ObjectHandle(dst,ListCreate,hdl);
    struct HandleListCreate *dst_handle = hdl->handle;
    if(dst_handle->memory == NULL)
        dst_handle->memory = mMemoryCreate(DFLT,DFLT);
        
    mMemoryCopy(src_handle->memory,src->data,dst_handle->memory,dst->data,src->num);
}

void mListMerge(MList *list1,MList *list2,MList *dst)
{
    mListAppend(dst,list1->num+list2->num);
    
    MHandle *hdl;
    ObjectHandle(list1,ListCreate,hdl);
    struct HandleListCreate *handle1 = hdl->handle;
    ObjectHandle(list2,ListCreate,hdl);
    struct HandleListCreate *handle2 = hdl->handle;
    
    ObjectHandle(dst,ListCreate,hdl);
    struct HandleListCreate *dst_handle = hdl->handle;
    
    int num1 = list1->num;
    int num2 = list2->num;
    
    if(dst==list1)
    {
        memcpy(dst->data+num1,list2->data,num2*sizeof(void *));
        mFree(list2->data);list2->data = NULL;list2->num = 0;
    }
    else if(dst==list2)
    {
        memcpy(dst->data+num2,list1->data,num1*sizeof(void *));
        mFree(list1->data);list1->data = NULL;list1->num = 0;
    }
    else
    {
        memcpy(dst->data     ,list1->data,num1*sizeof(void *));
        memcpy(dst->data+num1,list2->data,num2*sizeof(void *));
        mFree(list1->data);list1->data = NULL;list1->num = 0;
        mFree(list2->data);list2->data = NULL;list2->num = 0;
    }
    
    mMemoryMerge(handle1->memory,handle2->memory,dst_handle->memory);
    mMemoryRelease(handle1->memory);handle1->memory = NULL;
    mMemoryRelease(handle2->memory);handle2->memory = NULL;
}

void mListElementDelete(MList *list,int n)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input");
    mException((n>=list->num),EXIT,"invalid input");
    memmove(list->data+n,list->data+n+1,(list->num-n-1)*sizeof(void *));
}

void mListElementInsert(MList *list,int n,void *data,int size)
{
    void *buff;
    
    mListWrite(list,DFLT,data,size);
    buff = list->data[list->num-1];
    
    memmove(list->data+n+1,list->data+n,(list->num-n-1)*sizeof(void *));
    list->data[n] = buff;
}

void mListElementOperate(MList *list,void (*func)(void *,void *),void *para)
{
    mException(INVALID_POINTER(list)||(func==NULL),EXIT,"invalid input");
	int i;
    #pragma omp parallel for
    for(i=0;i<list->num;i++)
        func(list->data[i],para);
}

void mListElementScreen(MList *list,int (*func)(void *,void *),void *para)
{
    mException(INVALID_POINTER(list)||(func==NULL),EXIT,"invalid input");
    
    int n=0;
    for(int i=0;i<list->num;i++)
    {
        if(func(list->data[i],para))
        {
            list->data[n] = list->data[i];
            n=n+1;
        }
    }
    list->num = n;
}
void mListElementSelect(MList *list,void (*func)(void *,void *,int *,int *,void *),void *para)
{
    mException(INVALID_POINTER(list)||(func==NULL),EXIT,"invalid input");
    
    int n=0;
    for(int i=0;i<list->num;i++)
    {
        if(list->data[i]==NULL)
            continue;
        
        int flag1=1;
        for(int j=i+1;j<list->num;j++)
        {
            if(list->data[j] == NULL)
                continue;
            
            int flag2=1;
            func(list->data[i],list->data[j],&flag1,&flag2,para);
            if(flag2==0)
                list->data[j]=NULL;
            if(flag1==0)
                break;
        }
        if(flag1==1)
        {
            list->data[n]=list->data[i];
            n=n+1;
        }
    }
    list->num = n;
}

int mListCluster(MList *list,int *group,int (*func)(void *,void *,void *),void *para)
{
    mException((INVALID_POINTER(list))||(group==NULL)||(func==NULL),EXIT,"invalid input");

    char *valid = mMalloc(list->num * sizeof(char));
    memset(valid,0   ,list->num*sizeof(char));
    memset(group,DFLT,list->num*sizeof(int));
    
    int i,j,k;
    int n=0;
    for(i=0;i<list->num;i++)
    {
        for(j=0;j<i;j++)
        {
            if(group[i]==group[j]) continue;
            
            if(func(list->data[i],list->data[j],para)==1)//同类
            {
                if(group[i] == DFLT)
                    group[i] = group[j];
                else
                {
                    valid[group[j]] = 0;
                    int g = group[j];
                    for(k=0;k<i;k++)
                        if(group[k] == g) group[k] = group[i];
                }
            }
        }
        if(group[i] == DFLT)
        {
            group[i] = n;
            valid[n] = 1;
            n = n+1;
        }
    }
    
    int *c = mMalloc(n *sizeof(int));
    int num = 0;
    for(i=0;i<n;i++)
    {
        if(valid[i] != 0)
            {c[i] = num;num +=1;}
    }

    mFree(valid);
    
    for(i=0;i<list->num;i++)
        group[i] = c[group[i]];

    mFree(c);
    
    return num;
}

void ListSort(void **list_data,int n,int func(void *,void *,void *),void *para)
{
    void *buff;
    
    if(func(list_data[n-1],list_data[0],para)<0) {buff=list_data[n-1];list_data[n-1]=list_data[0];list_data[0]=buff;}
    if(n==2) return;
    
         if(func(list_data[  1],list_data[0],para)<0) {buff=list_data[  0];list_data[  0]=list_data[1];}
    else if(func(list_data[n-1],list_data[1],para)<0) {buff=list_data[n-1];list_data[n-1]=list_data[1];}
    else                                               buff=list_data[  1];
    if(n==3) {list_data[1]=buff;return;}
    
    int i=1;int j=n-2;
    while(1)
    {
        while(func(list_data[j],buff,para)>=0) {j=j-1;if(j==i) goto ListSort_next;}
        list_data[i] = list_data[j];            i=i+1;if(i==j) goto ListSort_next;
        while(func(list_data[i],buff,para)<=0) {i=i+1;if(i==j) goto ListSort_next;}
        list_data[j] = list_data[i];            j=j-1;if(i==j) goto ListSort_next;
    }
   
    ListSort_next:
    list_data[i]=buff;
    if(  i  >1)ListSort(list_data    ,  i  ,func,para);
    if(n-i-1>1)ListSort(list_data+i+1,n-i-1,func,para);
}
void mListSort(MList *list,int func(void *,void *,void *),void *para)
{
    mException((INVALID_POINTER(list))||(func==NULL),EXIT,"invalid input");
    if(list->num<=1)return;
    ListSort(list->data,list->num,func,para);
}


struct HandleStack
{
    int order;
};
void endStack(void *info) {NULL;}
#define HASH_Stack 0x8c4d4c73
void *mStackWrite(MList *stack,void *data,int size)
{
    mException(INVALID_POINTER(stack),EXIT,"invalid stack");
    MHandle *hdl;ObjectHandle(stack,Stack,hdl);
    struct HandleStack *handle = hdl->handle;
    if(hdl->valid == 0) handle->order = -1;
    hdl->valid = 1;
    if(handle->order==stack->num-1) return NULL;
    
    handle->order = handle->order+1;
    return mListWrite(stack,handle->order,data,size);
}

void *mStackRead(MList *stack,void *data,int size)
{
    mException(INVALID_POINTER(stack),EXIT,"invalid stack");
    MHandle *hdl;ObjectHandle(stack,Stack,hdl);
    struct HandleStack *handle = hdl->handle;
    if(hdl->valid == 0) return NULL;
    if(handle->order <0) return NULL;
    
    void *p=stack->data[handle->order];
    handle->order -= 1;
    if(data!=NULL)
    {
        if(size<=0) strcpy(data,p);
        else memcpy(data,p,size);
    }
    
    return p;
}

int mStackSize(MList *stack)
{
    mException(INVALID_POINTER(stack),EXIT,"invalid stack");
    MHandle *hdl;ObjectHandle(stack,Stack,hdl);
    struct HandleStack *handle = hdl->handle;
    if(hdl->valid == 0) handle->order = -1;
    hdl->valid = 1;
    
    return (handle->order+1);
}

struct HandleQueue
{
    int read_order;
    int write_order;
    int flag;
};
void endQueue(void *info) {NULL;}
#define HASH_Queue 0xd98b43dc

int mQueueSize(MList *queue)
{
    mException(INVALID_POINTER(queue),EXIT,"invalid queue");
    MHandle *hdl;ObjectHandle(queue,Queue,hdl);
    struct HandleQueue *handle = hdl->handle;
    if(handle->flag>0) return queue->num;
    if(handle->flag<0) return 0;
    
    int n = handle->write_order - handle->read_order;
    return ((n>0)?n:(queue->num+n));
}

void *mQueueWrite(MList *queue,void *data,int size)
{
    mException(INVALID_POINTER(queue),EXIT,"invalid queue");
    mException(queue->num<=0,EXIT,"invalid queue");
    MHandle *hdl;ObjectHandle(queue,Queue,hdl);
    struct HandleQueue *handle = hdl->handle;
    if(hdl->valid == 0) {handle->read_order=0;handle->write_order=0;}
    hdl->valid = 1;
    
    if(handle->flag>0) return NULL;
    void *p = mListWrite(queue,handle->write_order,data,size);
    handle->write_order = handle->write_order+1;
    if(handle->write_order == queue->num) handle->write_order = 0;
    handle->flag =(handle->write_order == handle->read_order)?1:0;
    return p;
}

void *mQueueRead(MList *queue,void *data,int size)
{
    mException(INVALID_POINTER(queue),EXIT,"invalid queue");
    mException(queue->num<=0,EXIT,"invalid queue");
    MHandle *hdl;ObjectHandle(queue,Queue,hdl);
    struct HandleQueue *handle = hdl->handle;
    if(hdl->valid == 0) return NULL;
    
    if(handle->flag<0) return NULL;
    void *p = queue->data[handle->read_order];
    handle->read_order = handle->read_order +1;
    if(handle->read_order == queue->num) handle->read_order = 0;
    handle->flag =(handle->write_order == handle->read_order)?-1:0;
    
    if(data!=NULL)
    {
        if(size<=0) strcpy(data,p);
        else memcpy(data,p,size);
    }
    return p;
}

// struct HashElement
// {
    // int hash;
    // void *data;
// };
// struct HandleHashList
// {
    // int num;
// };
// void mHashList(MList *list,void *data,int size)
// {
    // if(list->size < 


/*
struct HandleBuffer
{
    int proc_num;
    int *order;
    unsigned char *state;
};
void endBuffer(void *info) 
{
    struct HandleBuffer *handle = info;
    if(handle->state != NULL) mFree(handle->state);
}
#define HASH_Buffer 0xcb4df739
int BufferRead(MList *buffer,int ID,struct HandleBuffer *handle)
{
    int proc_num = handle->proc_num;
    int order = handle->order[ID];
    if(((ID >0)&&(handle->order[ID-1]==order))||((ID==0)&&(handle->order[proc_num-1]==order)))
        return DFLT;
    int state = handle->state[order];
    if((state&1 == 1)||(order<0))
    {
        order = order + 1;
        if(order == buffer->num)
        {
            if(handle->order[handle->proc_num-1]<0) return DFLT;
            order = 0;
        }
        handle->state[handle->order[ID]] = 0;
        handle->order[ID] = order;
        return BufferRead(buffer,ID,handle);
    }
    return order;
}


void *mBufferSet(MList *buffer,int volume,int proc_num)
{
    mException(INVALID_POINTER(buffer),EXIT,"invalid buffer");
    if(volume>0)
    {
        if(buffer->num>volume) buff->num = volume;
        else mListAppend(buff,volume);
    }
    mException(buffer->num<=1,EXIT,"invalid buffer");
    
    mException((proc_num<=0),EXIT,"invalid proc_num");
    MHandle *hdl;ObjectHandle(buffer,Buffer,hdl);
    struct HandleBuffer *handle = hdl->handle;
    if(hdl->valid == 0) 
    {
        handle->order = mMalloc(proc_num*sizeof(int));
        memset(handle->order,-1,proc_num*sizeof(int));
        handle->proc_num = proc_num;
        
        handle->state = mMalloc(buffer->num*sizeof(unsigned char));
        memset(handle->state,0,buffer->num*sizeof(unsigned char));
    }
    hdl->valid = 1;
}

void *mBufferWrite(MList *buffer,int ID,void *data,int size)
{
    mException(INVALID_POINTER(buffer),EXIT,"invalid buffer");
    MHandle *hdl;ObjectHandle(buffer,Buffer,hdl);
    struct HandleBuffer *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"invalid buffer");
    
    int proc_num = handle->proc_num;
    mException((ID>=proc_num)||(ID=<0),EXIT,"invalid ID");
    int order = handle->order[ID];
    if((handle->state[order]&2!=0)||(order<0))
    {
        order = order+1;
        if(order==buffer->num) order=0;
        if((ID==0)&&(state[order]!=0)) return NULL;
        if((ID >0)&&(state[order]!=4)) return NULL;
        
        handle->state[handle->order] = 4;
        handle->order[ID] = order;
    }
    void *p = mListWrite(buffer,order,data,size);
    handle->state[order] = (handle->state[order])|2;
    return p;
}

void mBufferRead(MList *buffer,int ID,void *data,int size)
{
    mException(INVALID_POINTER(buffer),EXIT,"invalid buffer");
    MHandle *hdl;ObjectHandle(buffer,Buffer,hdl);
    struct HandleBuffer *handle = hdl->handle;
    mException((hdl->valid == 0),EXIT,"invalid buffer");
    
    int proc_num = handle->proc_num;
    mException((ID>=proc_num)||(ID=<0),EXIT,"invalid ID");
    int order = handle->order[ID];

    if((handle->state[order]&1!=0)||(order<0))
    {
        order = order+1;
        if(order==buffer->num)
        {
            if(handle->order[proc_num-1]< 0) return NULL;
            order=0;
        }
        
        if(ID>0)
            if(handle->order[ID      -1]==order) return NULL;
        else if(proc_num>1)
            if(handle->order[proc_num-1]==order) return NULL;

        handle->state[handle->order] = 0;
        handle->order = order;
    }
    void *p = mListRead(buffer,order,data,size);
*/

