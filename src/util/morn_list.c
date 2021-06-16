/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

struct HandleListCreate
{
    MList *list;
    MChain *property;
    int num;
    void **data;
    MMemory *memory;
    int defrag_size;
    int read_order;
};
void endListCreate(struct HandleListCreate *handle)
{
    mException((handle->list == NULL),EXIT,"invalid list");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->memory !=NULL) mMemoryRelease(handle->memory);
    if(handle->data != NULL) mFree(handle->data);
    
    mFree(handle->list);
}
#define HASH_ListCreate 0xfa6c59f
MList *ListCreate(int num,void **data)
{
    MList *list = (MList *)mMalloc(sizeof(MList));
    memset(list,0,sizeof(MList));

    list->handle=mHandleCreate();
    MHandle *hdl=mHandle(list,ListCreate);
    struct HandleListCreate *handle = (struct HandleListCreate *)(hdl->handle);
    handle->list = list;
    
    if(num<0) num = 0;
    handle->num = num;
    list->num = num;
    
    if(num>0)
    {
        handle->data = (void **)mMalloc(num*sizeof(void *));
        if(!INVALID_POINTER(data)) memcpy(handle->data,data,num*sizeof(void *));
        else                       memset(handle->data,   0,num*sizeof(void *));
    }
    else
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
    
    list->data = handle->data;
    return list;
}

void mListRelease(MList *list)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    
    if(!INVALID_POINTER(list->handle))
        mHandleRelease(list->handle);
}

void m_ListAppend(MList *list,void **data,int n)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    if(n<0) n=list->num+1;
    else mException(n<list->num,EXIT,"invalid list append number");
    
    struct HandleListCreate *handle= (struct HandleListCreate *)(((MHandle *)(list->handle->data[0]))->handle);
    if(n<=handle->num) 
    {
        if((list->data!= handle->data)&&(list->num>0))
            memcpy(handle->data,list->data,list->num*sizeof(void *));
        if(data!=NULL) memcpy(handle->data,data,(n-list->num)*sizeof(void *));
        list->data = handle->data;
        list->num = n;
        return;
    }

    // printf("aaaaaaaaaaaaaa\n");
    int num = list->num + MAX(MAX(128,n-list->num),(list->num)>>1);
    void **list_data = (void **)mMalloc(num*sizeof(void *));
    if(list->num>0)
        memcpy(list_data,list->data,(list->num)*sizeof(void *));
    memset(list_data+list->num,0,(num-list->num)*sizeof(void *));
    if(data!=NULL) memcpy(list_data+list->num,data,(n-list->num)*sizeof(void *));
    
    if(handle->data != NULL) mFree(handle->data);
    handle->data = list_data;
    handle->num = num;
    
    list->data = handle->data;
    list->num = n;
}

void mListPlace(MList *list,void *data,int num,int size)
{
    if(num<=0) return;
    mException((size<=0),EXIT,"invalid input list element size");

    int list_num = list->num;
    mListAppend(list,list_num+num);
    
    struct HandleListCreate *handle = (struct HandleListCreate *)(((MHandle *)(list->handle->data[0]))->handle);

    void **idx = list->data+list_num;
    if(handle->memory == NULL) handle->memory = mMemoryCreate(1,size*num,MORN_HOST);
    else mMemoryAppend(handle->memory,size*num);
    mMemoryIndex(handle->memory,num,size,&idx,1);
    
    // printf("list_num=%d\n",list_num);
    // printf("idx0=%p,list->data[0]=%p\n",idx[0],list->data[0]);
    
    if(data==NULL) return;
    char *p=(char *)data;
    for(int i=0;i<num;i++) {memcpy(list->data[list_num+i],p,size);p+=size;}
}

// void mListOperate(MList *list,void (*func)(void *,void *),void *para)
// {
//     for(int i=0;i<list->num;i++) func(list->data[i],para);
// }

// struct HandleListWrite
// {
//     int defrag_size;
// };
// void endListWrite(void *info) {}
// #define HASH_ListWrite 0x40aea976
void *mListWrite(MList *list,int n,void *data,int size)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input source list");
    mException((n>list->num),EXIT,"invalid write location %d(with list->num is %d)",n,list->num);
    if(size<0)
    {
        mException((INVALID_POINTER(data)),EXIT,"invalid data to write,which is %p",data);
        size = strlen((char *)data)+1;
    }
    
    struct HandleListCreate *handle0 = (struct HandleListCreate *)(((MHandle *)(list->handle->data[0]))->handle);

    if(n<0) n = list->num;

    if(handle0->memory == NULL) handle0->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
    void *ptr = mMemoryWrite(handle0->memory,data,size);
    
    int flag = (n==list->num); if(!flag) flag=(list->data[n]==NULL);
    if(flag)
    {
        if(n<handle0->num) list->num = n+1;
        else mListAppend(list,DFLT);
        list->data[n] = ptr;
    }
    else
    {
        list->data[n] = ptr;
        handle0->defrag_size += size;
        if(handle0->defrag_size>16384)
        {
            mListElementOperate(list,MemoryCollect,handle0->memory);
            MemoryDefrag(handle0->memory);
            handle0->defrag_size=0;
        }
    }
    
    return list->data[n];
}

// struct HandleListRead
// {
//     int read_order;
// };
// void endListRead(void *info) {}
// #define HASH_ListRead 0x537cc305
void *mListRead(MList *list,int n,void *data,int size)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input");
    struct HandleListCreate *handle0 = (struct HandleListCreate *)(((MHandle *)(list->handle->data[0]))->handle);

    // MHandle *hdl=mHandle(list,ListRead);
    // struct HandleListRead *handle = (struct HandleListRead *)(hdl->handle);
    // if(hdl->valid == 0) handle->read_order = -1;
    // hdl->valid = 1;
    
    if(n<0) n = handle0->read_order;
    handle0->read_order = n+1;
    
    if(n>=list->num) return NULL;
    
    if(data!=NULL)
    {
        if(size>0) memcpy(        data,        list->data[n],size);
        else       strcpy((char *)data,(char *)list->data[n]);
    }
    return list->data[n];
}

void mListClear(MList *list)
{
    list->num=0;
    struct HandleListCreate *handle0 = (struct HandleListCreate *)(((MHandle *)(list->handle->data[0]))->handle);
    if(handle0->memory!=NULL) mMemoryClear(handle0->memory);
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
    
    struct HandleListCreate *src_handle = (struct HandleListCreate *)(((MHandle *)(src->handle->data[0]))->handle);
    if(src_handle->memory == NULL)
    {
        memcpy(dst->data,src->data,src->num*sizeof(void *));
        return;
    }
    
    struct HandleListCreate *dst_handle = (struct HandleListCreate *)(((MHandle *)(dst->handle->data[0]))->handle);
    if(dst_handle->memory == NULL)
        dst_handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
        
    mMemoryCopy(src_handle->memory,&(src->data),dst_handle->memory,&(src->data),1,&(src->num));
}

void mListMerge(MList *list1,MList *list2,MList *dst)
{
    if(list1->num+list2->num==0) {mListClear(dst); return;}
    mListAppend(dst,list1->num+list2->num);
    
    struct HandleListCreate *handle1 = (struct HandleListCreate *)(((MHandle *)(list1->handle->data[0]))->handle);
    struct HandleListCreate *handle2 = (struct HandleListCreate *)(((MHandle *)(list2->handle->data[0]))->handle);
    struct HandleListCreate *dst_handle =(struct HandleListCreate *)(((MHandle *)(dst->handle->data[0]))->handle);
    
    int num1 = list1->num;
    int num2 = list2->num;
    
    if(dst==list1)
    {
        if(num2>0)
        {
            memcpy(dst->data+num1,list2->data,num2*sizeof(void *));
            mFree(list2->data);list2->data = NULL;list2->num = 0;
        }
    }
    else if(dst==list2)
    {
        if(num1>0)
        {
            memcpy(dst->data+num2,list1->data,num1*sizeof(void *));
            mFree(list1->data);list1->data = NULL;list1->num = 0;
        }
    }
    else
    {
        if(num1>0)
        {
            memcpy(dst->data     ,list1->data,num1*sizeof(void *));
            mFree(list1->data);list1->data = NULL;list1->num = 0;
        }
        if(num2>0)
        {
            memcpy(dst->data+num1,list2->data,num2*sizeof(void *));
            mFree(list2->data);list2->data = NULL;list2->num = 0;
        }
    }

    if(dst_handle->memory==NULL) dst_handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
    else                         mMemoryRedefine(dst_handle->memory,num1+num2,DFLT,DFLT);
    mMemoryMerge(handle1->memory,handle2->memory,dst_handle->memory);
    mMemoryRelease(handle1->memory);handle1->memory = NULL;
    mMemoryRelease(handle2->memory);handle2->memory = NULL;
}

void mListElementDelete(MList *list,int n)
{
    mException(INVALID_POINTER(list),EXIT,"invalid input");
    mException((n>=list->num),EXIT,"invalid input");
    memmove(list->data+n,list->data+n+1,(list->num-n-1)*sizeof(void *));
    list->num-=1;
}

void *mListElementInsert(MList *list,int n,void *data,int size)
{
    mListWrite(list,DFLT,data,size);
    void *buff = list->data[list->num-1];
    
    memmove(list->data+n+1,list->data+n,(list->num-n-1)*sizeof(void *));
    list->data[n] = buff;
    return buff;
}

void mListElementOperate(MList *list,void *function,void *para)
{
    void (*func)(void *,void *) = function;
    mException(INVALID_POINTER(list)||(func==NULL),EXIT,"invalid input");
	int i;
    // #pragma omp parallel for
    for(i=0;i<list->num;i++)
        func(list->data[i],para);
}

void mListElementScreen(MList *list,void *function,void *para)
{
    int (*func)(void *,void *) = function;
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
void mListElementSelect(MList *list,void *function,void *para)
{
    void (*func)(void *,void *,int *,int *,void *) = function;
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

/*
void mListSelect(MList *list,void (*func)(void *,void *,int *,int *,void *),void *para)
{
    mException(INVALID_POINTER(list)||(func==NULL),EXIT,"invalid input");
    void **data = list->data;
    
    int *flag=mMalloc((list->num+2)*sizeof(int));
    flag=flag+1;
    memset(flag,DFLT,list->num*sizeof(int));
    flag[-1]=list->num; flag[list->num]=-1;
    
    int flag1,flag2;
    while(1)
    {
        int ok=1;
        for(int i=flag[i];i<list->num;i++)
        {
            if(flag[i]<0) continue;
            for(int j=flag[i]+1;j<list->num;j++)
            {
                if(j==i) continue;
                if((flag[j]>=0)&&(flag[j]<list->num)) continue;
                
                func(data[i],data[j],&flag1,&flag2,para);
                if(flag1==0) {flag[i] = j;ok=0;break;}
                if(flag2==0) {flag[j] = i;ok=0;continue;}
            }
            if(flag[i]>=0) continue;
            flag[i]=list->num;
        }
        if(ok) break;
    }

    int n=0;
    for(int i=0;i<list->num;i++) if(flag[i]==list->num) {list->data[n]=data[i];n++;}
    list->num = n;
    
    mFree(flag-1);
}
*/

int mListCluster(MList *list,int *group,void *function,void *para)
{
    int (*func)(void *,void *,void *) = function;
    mException((INVALID_POINTER(list))||(group==NULL)||(func==NULL),EXIT,"invalid input");

    char *valid = (char *)mMalloc(list->num * sizeof(char));
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
    
    int *c = (int *)mMalloc(n *sizeof(int));
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

struct HandleListClassify
{
    int *group;
    char *valid;
    MSheet *sheet;
    int list_num;
};
void endListClassify(struct HandleListClassify *handle)
{
    if(handle->group!=NULL) mFree(handle->group);
    if(handle->valid!=NULL) mFree(handle->valid);
    if(handle->sheet!=NULL) mSheetRelease(handle->sheet);
}
#define HASH_ListClassify 0x24c19acf
MSheet *mListClassify(MList *list,void *function,void *para)
{
    int (*func)(void *,void *,void *) = function;
    mException((INVALID_POINTER(list))||(func==NULL),EXIT,"invalid input");

    MHandle *hdl = mHandle(list,ListClassify);
    struct HandleListClassify *handle = (struct HandleListClassify *)(hdl->handle);
    if((hdl->valid == 0)||(handle->list_num<list->num))
    {
        if(handle->list_num<list->num)
        {
            if(handle->group!=NULL) {mFree(handle->group);handle->group=NULL;}
            if(handle->valid!=NULL) {mFree(handle->valid);handle->valid=NULL;}
        }
        if(handle->group==NULL) handle->group = (int  *)mMalloc(list->num*sizeof(int ));
        if(handle->valid==NULL) handle->valid = (char *)mMalloc(list->num*sizeof(char));
        handle->list_num = list->num;

        if(handle->sheet == NULL) handle->sheet = mSheetCreate();
        hdl->valid = 1;
    }
    char *valid = handle->valid; int *group = handle->group;
    memset(valid,0   ,list->num*sizeof(char));
    memset(group,DFLT,list->num*sizeof(int));
    
    int i,j,k;
    int n=0;
    for(i=0;i<list->num;i++)
    {
        for(j=0;j<i;j++)
        {
            if(group[i]==group[j]) continue;
            
            if(func(list->data[i],list->data[j],para)==1)
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
    
    int *c = (int *)mMalloc(n *sizeof(int));
    int num = 0;
    for(i=0;i<n;i++)
    {
        if(valid[i] != 0)
            {c[i] = num;num +=1;}
    }

    MSheet *sheet = handle->sheet;
    mSheetClear(sheet);
    
    mSheetRowAppend(sheet,num);
    for(i=0;i<list->num;i++)
    {
        int g = c[group[i]];
        int n = sheet->col[g];
        mSheetColAppend(sheet,g,n+1);
        sheet->data[g][n]=list->data[i];
    }
    mFree(c);
    return sheet;
}

void _ListSort(void **list_data,int n,int (*func)(void *,void *,void *),void *para)
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
    if(  i  >1) _ListSort(list_data    ,  i  ,func,para);
    if(n-i-1>1) _ListSort(list_data+i+1,n-i-1,func,para);
}
void mListSort(MList *list,void *function,void *para)
{
    int (*func)(void *,void *,void *) = function;
    mException((INVALID_POINTER(list))||(func==NULL),EXIT,"invalid input");
    if(list->num<=1)return;
    _ListSort(list->data,list->num,func,para);
}

struct HandleListMatch
{
    int list_num;
    int *idx;
};
void endListMatch(struct HandleListMatch *handle)
{
    if(handle->idx!=NULL) mFree(handle->idx);
}
#define HASH_ListMatch 0x39871020
int *m_ListMatch(MList *src,MList *dst,float thresh,void *function,void *para)
{
    float (*func)(void *,void *,void *) = function;
    mException((INVALID_POINTER(src)||INVALID_POINTER(dst)),EXIT,"invalid input");
    MHandle *hdl = mHandle(src,ListMatch);
    struct HandleListMatch *handle = (struct HandleListMatch *)(hdl->handle);
    if((hdl->valid==0)||(src->num>handle->list_num))
    {
        int list_num = MAX(src->num,handle->list_num);
        if(list_num>handle->list_num)
        {
            if(handle->idx     !=NULL) mFree(handle->idx);
            handle->idx = mMalloc(list_num*sizeof(int));
            handle->list_num = list_num;
        }
        hdl->valid = 1;
    }
    if(dst->num==0) {memset(handle->idx,DFLT,src->num*sizeof(int));return handle->idx;}

    for(int i=0;i<src->num;i++)
    {
        float d_min = func(src->data[i],dst->data[0],para);int idx = 0;
        for(int j=1;j<dst->num;j++)
        {
            float d = func(src->data[i],dst->data[j],para);
            if(d<d_min){d_min=d;idx=j;}
        }
        handle->idx[i]=(d_min<thresh)?idx:DFLT;
    }
    return (handle->idx);
}

struct HandleStack
{
    volatile int order;
};
void endStack(void *info) {}
#define HASH_Stack 0x8c4d4c73
void *mStackWrite(MList *stack,void *data,int size)
{
    mException(INVALID_POINTER(stack),EXIT,"invalid stack");
    MHandle *hdl=mHandle(stack,Stack);
    struct HandleStack *handle = (struct HandleStack *)(hdl->handle);
    if(hdl->valid == 0) handle->order = -1;
    hdl->valid = 1;
    if(handle->order==stack->num-1) return NULL;
    
    mAtomicAdd(&(handle->order),1);
    return mListWrite(stack,handle->order,data,size);
}

void *mStackRead(MList *stack,void *data,int size)
{
    mException(INVALID_POINTER(stack),EXIT,"invalid stack");
    MHandle *hdl=mHandle(stack,Stack);
    struct HandleStack *handle = (struct HandleStack *)(hdl->handle);
    if(hdl->valid == 0) return NULL;
    if(handle->order <0) return NULL;

    int order = mAtomicSub(&(handle->order),1);
    void *p=stack->data[order];
    if(data!=NULL)
    {
        if(size<=0) strcpy((char *)data,(char *)p);
        else memcpy(data,p,size);
    }
    
    return p;
}

int mStackSize(MList *stack)
{
    mException(INVALID_POINTER(stack),EXIT,"invalid stack");
    MHandle *hdl=mHandle(stack,Stack);
    struct HandleStack *handle = (struct HandleStack *)(hdl->handle);
    if(hdl->valid == 0) handle->order = -1;
    hdl->valid = 1;
    
    return (handle->order+1);
}

// struct HandleQueue
// {
//     volatile int read_order;
//     volatile int write_order;
//     volatile int flag;
// };
// void endQueue(void *info) {}
// #define HASH_Queue 0xd98b43dc

// int mQueueSize(MList *queue)
// {
//     mException(INVALID_POINTER(queue),EXIT,"invalid queue");
//     MHandle *hdl=mHandle(queue,Queue);
//     struct HandleQueue *handle = (struct HandleQueue *)(hdl->handle);
//     if(handle->flag>0) return queue->num;
//     if(handle->flag<0) return 0;
    
//     int n = handle->write_order - handle->read_order;
//     return ((n>0)?n:(queue->num+n));
// }

// void *mQueueWrite(MList *queue,void *data,int size)
// {
//     mException(INVALID_POINTER(queue),EXIT,"invalid queue");
//     mException(queue->num<=0,EXIT,"invalid queue");
//     MHandle *hdl=mHandle(queue,Queue);
//     struct HandleQueue *handle = (struct HandleQueue *)(hdl->handle);
//     if(hdl->valid == 0) {handle->read_order=0;handle->write_order=0;}
//     hdl->valid = 1;
    
//     if(handle->flag>0) return NULL;

//     int order=mAtomicAdd(&(handle->write_order),1);
//     if(order>=queue->num) order=order-queue->num;
//     void *p = mListWrite(queue,order,data,size);
    
//     mAtomicCompare(&(handle->write_order),queue->num,0);
//     handle->flag =(handle->write_order == handle->read_order)?1:0;
//     return p;
// }

// void *mQueueRead(MList *queue,void *data,int size)
// {
//     mException(INVALID_POINTER(queue),EXIT,"invalid queue");
//     mException(queue->num<=0,EXIT,"invalid queue");
//     MHandle *hdl=mHandle(queue,Queue);
//     struct HandleQueue *handle = (struct HandleQueue *)(hdl->handle);
//     if(hdl->valid == 0) return NULL;
    
//     if(handle->flag<0) return NULL;
//     int order = mAtomicAdd(&(handle->read_order),1);
//     void *p = queue->data[order];
//     mAtomicCompare(&(handle->read_order),queue->num,0);
//     handle->flag =(handle->write_order == handle->read_order)?-1:0;
    
//     if(data!=NULL)
//     {
//         if(size<=0) strcpy((char *)data,(char *)p);
//         else memcpy(data,p,size);
//     }
//     return p;
// }

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

