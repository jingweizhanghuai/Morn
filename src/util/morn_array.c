/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

struct _MArray{
    int num;
    short element_size;
    unsigned short capacity;
    union
    {
        void *data;
        unsigned char *dataU8;
        char *dataS8;
        unsigned short *dataU16;
        short *dataS16;
        unsigned int *dataU32;
        int *dataS32;
        int64_t *dataS64;
        uint64_t *dataU64;
        float *dataF32;
        double *dataD64;
        void **dataptr;
    };
};

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

// void ArrayDevice(int *device,struct HandleArrayCreate *handle)
// {
//     if(*device==MORN_HOST)
//     {
//         if(handle->devflag==MORN_HOST) return;
//         handle->devflag = MORN_HOST;
//         mMemoryBlockRead(handle->memory);
//     }
//     else if(*device==handle->memory->device)
//     {
//         if(handle->devflag==MORN_DEVICE) return;
//         handle->devflag = MORN_DEVICE;
//         mMemoryBlockWrite(handle->memory);
//     }
//     else
//     {
//         mMemoryBlockCopy(handle->memory,*device);
//         handle->devflag= MORN_DEVICE;
//     }
// }
void endArrayCreate(struct HandleArrayCreate *handle)
{
    mException((handle->array == NULL),EXIT,"invalid array");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->memory!= NULL) mMemoryBlockRelease(handle->memory);
    
    memset(handle->array,0,sizeof(struct _MArray));
    // ObjectFree(handle->array);
}
#define HASH_ArrayCreate 0xb3feafa4
MArray *ArrayCreate(int num,int element_size,void *data)
{
    struct _MArray *array = ObjectAlloc(sizeof(MArray));
    if(num<0) {num = 0;} array->num = num;
    if(element_size<0) {element_size=0;} array->element_size=element_size;
    
    MHandle *hdl=mHandle(array,ArrayCreate);
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(hdl->handle);
    handle->writeable=1;
    handle->array = (MArray *)array;
    array->dataS8 = NULL;
    array->capacity=0;

    handle->memory=NULL;
    handle->num = 0;
    if(!INVALID_POINTER(data))
    {
        mException(num==0,EXIT,"invalid input");
        array->dataS8 = data;
        handle->writeable=0;
    }
    else if(element_size>0)
    {
        num = mBinaryCeil(MAX(num,256));
        handle->memory = mMemoryBlockCreate(num*element_size,MORN_HOST);
        handle->num = num;
        handle->element_size = element_size;
        array->dataS8 = handle->memory->data;
        array->capacity = (num-array->num)&0x0FFFF;
    }
    mPropertyFunction(array,"device",mornMemoryDevice,NULL);
    
    return (MArray *)array;
}

void mArrayRelease(MArray *array)
{
    ObjectFree(array);
}

void ArrayRedefine(MArray *arr,int num,int element_size,void *data)
{
    mException(INVALID_POINTER(arr),EXIT,"invalid input");
    struct _MArray *array = (struct _MArray *)arr;

    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(arr,0)->handle);
    if(num          <=0) num = array->num;
    if(element_size <=0) element_size = array->element_size;
    array->num = num;
    array->element_size = element_size;
    if((element_size ==0)||(num==0)) return;
    // printf("element_size=%d,num=%d\n",element_size,num);
    
    if((num!= handle->num)||(element_size!=handle->element_size)) mHandleReset(array);
    handle->num = handle->num*handle->element_size/element_size;
    handle->element_size = element_size;
    int same_size = (num <= handle->num);
    int reuse = (data!=NULL)&&(data==array->dataS8);
    
    if(same_size&&reuse) return;

    if(same_size&&(INVALID_POINTER(data))&&(handle->num >0)) {array->capacity = (handle->num-array->num)&0x0FFFF;return;}
    mException(reuse&&(array->num>0)&&(handle->num==0),EXIT,"invalid redefine");

    if((num <= 0)||(element_size<=0)) 
    {
        mException((!INVALID_POINTER(data))&&(!reuse),EXIT,"invalid input");
        array->dataS8 = NULL;
        return;
    }
    
    if(reuse) data=NULL;
    if(!INVALID_POINTER(data)) {array->dataS8 = data;return;}
    
    if(num>handle->num)
    {
        handle->num = num;
        array->capacity=0;
        handle->element_size = element_size;
        if(handle->memory!=NULL) handle->memory = MemoryBlockRedefine(handle->memory,num*element_size);
        else                     handle->memory =mMemoryBlockCreate(num*element_size,MORN_HOST);
        array->dataS8 = handle->memory->data;
    }
    else array->capacity = (handle->num-array->num)&0x0FFFF;
}

void mArrayElementDelete(MArray *arr,int n)
{
    struct _MArray *array = (struct _MArray *)arr;
    memmove(array->dataS8+n*array->element_size,array->dataS8+(n+1)*array->element_size,(array->num-n-1)*array->element_size);
    array->num--;
    array->capacity++;
}

// void *_ArrayPushBack32(struct _MArray *array,uint32_t data)
// {
//     if((array->num&array->capacity)==0)
//     {
//         struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
//         if(handle->num==array->num)
//         {
//             int device = handle->memory->device;
//             handle->num = MAX(256,handle->num*2);
//             array->capacity=(handle->num-1)&0x0ffff;
//             MMemoryBlock *memory = mMemoryBlockCreate(handle->num*array->element_size,device);
//             if(array->num>0)
//             {
//                 memcpy(memory->data,array->dataS8,array->num*array->element_size);
//                 mMemoryBlockRelease(handle->memory);
//             }
//             handle->memory=memory;
//             array->dataS8 = memory->data;
//         }
//     }
//     uint32_t *p=array->dataU32+array->num;
//     *p = data;
//     array->num++;
//     return p;
// }

void mArrayAppend(MArray *arr,int n)
{
    struct _MArray *array = (struct _MArray *)arr;
    mException((INVALID_POINTER(array)),EXIT,"invalid input");
    if(n<=0) {n = array->num+256;array->num+=1;}
    else {mException(n<array->num,EXIT,"invalid input");array->num = n;}
    if(n-array->num>array->capacity)
    {
        struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
        if(n>handle->num)
        {
            if(handle->memory!=NULL) handle->memory = MemoryBlockRedefine(handle->memory,n*array->element_size);
            else                     handle->memory =mMemoryBlockCreate(n*array->element_size,MORN_HOST);
            handle->num = n;
            array->dataS8 = handle->memory->data;
            array->capacity=(n-array->num)&0x0FFFF;
        }
    }
}

void *m_ArrayPushBack(MArray *arr,void *data)
{
    struct _MArray *array = (struct _MArray *)arr;
    int es=array->element_size;
    if(array->capacity==0)
    {
        struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
        if(handle->num<=array->num)
        {
            // printf("handle->num=%d,array->num=%d\n",handle->num,array->num);
            mException(handle->memory==NULL,EXIT,"invalid array");
            int device = handle->memory->device;
            handle->num = handle->num*2;
            MMemoryBlock *memory = mMemoryBlockCreate(handle->num*es,device);
            memcpy(memory->data,array->dataS8,es*array->num);
            mMemoryBlockRelease(handle->memory);
            handle->memory=memory;
            array->dataS8 = memory->data;
        }
        array->capacity=(handle->num-array->num)&0x0FFFF;
    }
    S8 *p=(S8 *)(array->dataS8+array->num*es);
         if(es==4) *((S32 *)p) = *((S32 *)data);
    else if(es==8) *((S64 *)p) = *((S64 *)data);
    else if(es==1) *(       p) = *((S8  *)data);
    else if(es==2) *((S16 *)p) = *((S16 *)data);
    else memcpy(p,data,es);
    array->num++;
    array->capacity--;
    return (void *)p;
}

void *m_ArrayWrite(MArray *arr,intptr_t n,void *data)
{
    if(n<0) return m_ArrayPushBack(arr,data);
    int es=arr->element_size;
    S8 *p=(S8 *)(arr->dataS8+n*es);
         if(es==4) *((S32 *)p) = *((S32 *)data);
    else if(es==8) *((S64 *)p) = *((S64 *)data);
    else if(es==1) *(       p) = *((S8  *)data);
    else if(es==2) *((S16 *)p) = *((S16 *)data);
    else memcpy(p,data,es);
    return (void *)p;
}

void *m_ArrayRead(MArray *array,int n,void *data)
{
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    mException(n>=array->num,EXIT,"invalid input");
    
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
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

// void mArrayBitWrite(MArray *array,int n,int data)
// {
    // uint8_t *p=array->dataU8+n/array->element_size;
    // p[n/array
    


