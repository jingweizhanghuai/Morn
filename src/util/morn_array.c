/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

struct HandleArrayCreate
{
    MArray *array;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    int num;
    int element_size;
    MMemoryBlock *memory;
    int read_order;
};
void endArrayCreate(struct HandleArrayCreate *handle)
{
    mException((handle->array == NULL),EXIT,"invalid array");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->memory!= NULL) mMemoryBlockRelease(handle->memory);
    
    mFree(handle->array);
}
#define HASH_ArrayCreate 0xb3feafa4
MArray *ArrayCreate(int num,int element_size,void *data)
{
    MArray *array = (MArray *)mMalloc(sizeof(MArray));
    memset(array,0,sizeof(MArray));   
    
    if(num<0) {num = 0;} array->num = num;
    if(element_size<0) {element_size=0;} array->element_size=element_size;
    
    array->handle = mHandleCreate();
    MHandle *hdl=mHandle(array,ArrayCreate);
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(hdl->handle);
    handle->writeable=1;
    handle->array = array;
    array->data = NULL;
    
    if(num==0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
    }
    else if(!INVALID_POINTER(data))
        array->data = data;
    else if(element_size>0)
    {
        handle->memory = mMemoryBlockCreate(num*element_size,MORN_HOST);
        handle->num = num;
        handle->element_size = element_size;
        array->data = handle->memory->data;
    }
    
    return array;
}

void mArrayRelease(MArray *array)
{   
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    
    if(!INVALID_POINTER(array->handle))
        mHandleRelease(array->handle);
}

void ArrayRedefine(MArray *array,int num,int element_size,void *data)
{
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(((MHandle *)(array->handle->data[0]))->handle);
    if(num          <=0) num = array->num;
    if(element_size <=0) element_size = array->element_size;
    
    if((num!= handle->num)||(element_size!=handle->element_size)) mHandleReset(array->handle);
    int same_size = (num*element_size <= handle->num*handle->element_size);
    int reuse = (data==array->data);
    int flag = (array->num>0);
    
    array->num = num;
    array->element_size = element_size;
    
    handle->element_size = element_size;
    if(same_size&&reuse) return;
    if(same_size&&(INVALID_POINTER(data))&&(handle->num >0)) return;
    
    mException(reuse&&flag&&(handle->num==0),EXIT,"invalid redefine");
    
    handle->num=0;
    if((num <= 0)||(element_size<=0)) 
    {
        mException((!INVALID_POINTER(data))&&(!reuse),EXIT,"invalid input");
        array->data = NULL;
        return;
    }
    
    if(reuse) data=NULL;
    
    if(!INVALID_POINTER(data)) {handle->element_size = element_size;array->data = data;return;}
    
    if(num>handle->num)
    {
        handle->num = num;
        handle->element_size = element_size;
        if(handle->memory!=NULL) mMemoryBlockRelease(handle->memory);
        handle->memory = mMemoryBlockCreate(num*element_size,MORN_HOST);
        array->data = handle->memory->data;
    }
}

void mArrayElementDelete(MArray *array,int n)
{
    memmove(array->dataS8+n*array->element_size,array->dataS8+(n+1)*array->element_size,(array->num-n-1)*array->element_size);
    array->num--;
}

void mArrayAppend(MArray *array,int n)
{
    mException((INVALID_POINTER(array)),EXIT,"invalid input");
    if(n<=0) n = array->num+1;else mException(n<array->num,EXIT,"invalid input");
    array->num = n;
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(((MHandle *)(array->handle->data[0]))->handle);
    if(n<handle->num) return;
    handle->memory = MemoryBlockRedefine(handle->memory,handle->num*array->element_size);
    array->dataS8 = handle->memory->data;
}

void ArrayExpand(MArray *array,int n)
{
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(((MHandle *)(array->handle->data[0]))->handle);\
    if(n+1024>handle->num)
    {
        handle->num = MAX(1024,handle->num*2);
        handle->memory = MemoryBlockRedefine(handle->memory,handle->num*array->element_size);
        // printf("handle->memory=%p\n",handle->memory);
        array->dataS8 = handle->memory->data;
    }
}

// void m_ArrayPushBack(MArray *array,void *data)
// {
//     int n=array->num;
//     int element_size = array->element_size;
//     if((n&0x3FF)==0) ArrayExpand(array,n);
//     array->num=n+1;
//     char *p=array->dataS8+n*element_size;
//     memcpy(p,data,element_size);
// }

#define _ArrayWrite(Sz,Array,Order,Data) {\
    int Array_num=Array->num;\
    struct HandleArrayCreate *Handle = (struct HandleArrayCreate *)(((MHandle *)(Array->handle->data[0]))->handle);\
    int N=Order;if(N<0) {N=Array_num;}\
    Array->num=MAX(N+1,Array->num);\
    \
    if(N>=Handle->num)\
    {\
        Handle->num = MAX(1024,N*2);\
        MMemoryBlock *Memory = mMemoryBlockCreate(Handle->num*(Sz/8),MORN_HOST);\
        if(Array_num)\
        {\
            memcpy(Memory->data,Array->dataS8,(Sz/8)*Array_num);\
            mMemoryBlockRelease(Handle->memory);\
        }\
        Handle->memory=Memory;\
        \
        Array->dataS8 = Memory->data;\
    }\
    Array->dataS##Sz[N] = *((S##Sz *)(Data));\
}
void _ArrayWrite_1(MArray *Array,int Order,void *Data) {_ArrayWrite( 8,Array,Order,Data);}
void _ArrayWrite_2(MArray *Array,int Order,void *Data) {_ArrayWrite(16,Array,Order,Data);}
void _ArrayWrite_4(MArray *Array,int Order,void *Data) {_ArrayWrite(32,Array,Order,Data);}
void _ArrayWrite_8(MArray *Array,int Order,void *Data) {_ArrayWrite(64,Array,Order,Data);}

void m_ArrayWrite(MArray *array,int n,void *data)
{
    int array_num=array->num;
    int element_size = array->element_size;
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(((MHandle *)(array->handle->data[0]))->handle);
    if(n<0) {n=array_num;}
    array->num=MAX(n+1,array->num);
    
    if(n>=handle->num)
    {
        handle->num = MAX(256,n*1.5);
        MMemoryBlock *memory = mMemoryBlockCreate(handle->num*element_size,MORN_HOST);
        if(array_num)
        {
            memcpy(memory->data,array->dataS8,element_size*array_num);
            mMemoryBlockRelease(handle->memory);
        }
        handle->memory=memory;
        
        array->dataS8 = memory->data;
    }
    char *p=array->dataS8+n*element_size;
    if(data) memcpy(p,data,element_size);
}

void *m_ArrayRead(MArray *array,int n,void *data)
{
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    mException(n>=array->num,EXIT,"invalid input");
    
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(((MHandle *)(array->handle->data[0]))->handle);
    if(n<=0) {n=handle->read_order;if(n>=array->num) n=0;}
    handle->read_order=n+1;
    
    int element_size = array->element_size;
    char *p=array->dataS8+n*element_size;
    if(data!=NULL) memcpy(data,p,element_size);
    return p;
}

struct HandleStream
{
    char *read;
    char *write;
    MThreadSignal sgn;
    // pthread_mutex_t stream_mutex;
};
void endStream(void *info) {}
#define HASH_Stream 0xcab28d39

int mStreamRead(MArray *buff,void *data,int num)
{
    mException((buff==NULL)||(data==NULL)||(num<=0),EXIT,"invalid input");
    
    // struct HandleArrayCreate *handle0 = (struct HandleArrayCreate *)(((MHandle *)(buff->handle->data[0]))->handle);
    num = num*buff->element_size;
    
    MHandle *hdl=mHandle(buff,Stream);
    struct HandleStream *handle = (struct HandleStream *)(hdl->handle);
    if(hdl->valid == 0) return ((0-num)/buff->element_size);

    mThreadLockBegin(handle->sgn);
    // pthread_mutex_lock(&(handle->stream_mutex));
    
    int size=(handle->write>=handle->read)?(handle->write-handle->read):(buff->num*buff->element_size-(handle->read-handle->write));
    // printf("read num=%d,size=%d\n",num,size);
    if(num>size) goto StreamRead_end;

    int size0 = (buff->dataS8+buff->num*buff->element_size)-handle->read;
    if(size0 <= num)
    {
        memcpy(data,handle->read,size0);
        data = ((char *)data) + size0;
        handle->read = buff->dataS8;
        num = num - size0;
    }
    memcpy(data,handle->read,num);
    handle->read = handle->read + num;

    StreamRead_end:
    mThreadLockEnd(handle->sgn);
    // pthread_mutex_unlock(&(handle->stream_mutex));
    return ((size-num)/buff->element_size);
}

int mStreamWrite(MArray *buff,void *data,int num)
{
    mException((buff==NULL)||(data==NULL)||(num<=0),EXIT,"invalid input");
    
    // struct HandleArrayCreate *handle0 = (struct HandleArrayCreate *)(((MHandle *)(buff->handle->data[0]))->handle);
    num = num*buff->element_size;
    
    MHandle *hdl=mHandle(buff,Stream);
    struct HandleStream *handle = (struct HandleStream *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->read = buff->dataS8;
        handle->write= buff->dataS8;
        // pthread_mutex_init(&(handle->stream_mutex),NULL);
        // handle->stream_mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    mThreadLockBegin(handle->sgn);
    // pthread_mutex_lock(&(handle->stream_mutex));
    
    int size=(handle->read>handle->write)?(handle->read-handle->write):(buff->num*buff->element_size-(handle->write-handle->read));
    // printf("write num=%d,size=%d\n",num,size);
    if(num>size) goto StreamWrite_end;

    int size0 = (buff->dataS8+buff->num*buff->element_size)-handle->write;
    if(size0 <= num)
    {
        memcpy(handle->write,data,size0);
        data = ((char *)data) + size0;
        handle->write = buff->dataS8;
        num = num - size0;
    }
    memcpy(handle->write,data,num);
    handle->write = handle->write + num;
  
    StreamWrite_end:
    hdl->valid = 1;
    mThreadLockEnd(handle->sgn);
    // pthread_mutex_unlock(&(handle->stream_mutex));
    return ((size-num)/buff->element_size);
}


