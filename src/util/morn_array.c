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
        void     *data;
        char     *string;
        uint8_t  *dataU8;
        int8_t   *dataS8;
        uint16_t *dataU16;
        int16_t  *dataS16;
        uint32_t *dataU32;
        int32_t  *dataS32;
        int64_t  *dataS64;
        uint64_t *dataU64;
        float    *dataF32;
        double   *dataD64;
        void    **dataptr;
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
    mObjectType(array)=HASH_ArrayCreate;
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

// int ArrayCapacity(MArray *arr)
// {
//     mException(INVALID_POINTER(arr),EXIT,"invalid input");
//     struct _MArray *array = (struct _MArray *)arr;
//     return array->capacity;
// }

void ArrayRedefine(MArray *arr,int num,int element_size,void *data)
{
    mException(INVALID_POINTER(arr),EXIT,"invalid input");
    struct _MArray *array = (struct _MArray *)arr;
    if(data==array->dataS8) data=NULL;

    if(num          <=0) num = array->num;
    if(element_size <=0) element_size = array->element_size;
    array->num = num;
    array->element_size = element_size;
    array->capacity=0;
    if((element_size ==0)||(num==0)) {array->data=NULL; return;}
    
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(arr,0)->handle);
    if((num!= array->num)||(element_size!=handle->element_size)) mHandleReset(array);
    
    handle->num = handle->num*handle->element_size/element_size;
    handle->element_size = element_size;
    
    if((num <= handle->num)&&(data==NULL)) return;

    void *memory_data=NULL;if(handle->memory) memory_data=handle->memory->data;
    mException((data==NULL)&&(array->data!=memory_data),EXIT,"invalid redefine");
    
    if(data!=NULL) {array->data = data;return;}
    
    if(num>handle->num)
    {
        handle->num = mBinaryCeil(MAX(num,256));
        handle->element_size = element_size;
        if(handle->memory!=NULL) handle->memory =MemoryBlockRedefine(handle->memory,handle->num*element_size);
        else                     handle->memory =mMemoryBlockCreate(handle->num*element_size,MORN_HOST);
        
        array->data = handle->memory->data;
    }
    array->capacity = (handle->num-array->num)&0x0FFFF;
}

void mArrayDataExchange(MArray *arr1,MArray *arr2)
{
    struct HandleArrayCreate *h1 = (struct HandleArrayCreate *)(ObjHandle(arr1,0)->handle);
    struct HandleArrayCreate *h2 = (struct HandleArrayCreate *)(ObjHandle(arr2,0)->handle);
    struct _MArray *a1=(struct _MArray *)arr1;struct _MArray *a2=(struct _MArray *)arr2;
    
    int buff;void *p;
    buff=a1->num;         a1->num         =a2->num;         a2->num         =buff;
    buff=a1->element_size;a1->element_size=a2->element_size;a2->element_size=buff;
    buff=a1->capacity;    a1->capacity    =a2->capacity;    a2->capacity    =buff;
    p   =a1->data;        a1->data        =a2->data;        a2->data        =p;
    
    buff=h1->num;         h1->num         =h2->num;         h2->num         =buff;
    buff=h1->element_size;h1->element_size=h2->element_size;h2->element_size=buff;
    p   =h1->memory;      h1->memory      =h2->memory;      h2->memory      =p;
    
    h1->read_order=0;h2->read_order=0;
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

void m_ArrayAppend(MArray *array,void *data,int n)
{
    int es=array->element_size;
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
    MMemoryBlock *memory0=NULL;
    if(array->num+n>handle->num)
    {
        MMemoryBlock *memory = mMemoryBlockCreate((array->num+n+256)*es,MORN_HOST);
        if(array->num>0) memcpy(memory->data,array->dataS8,array->num*es);
        memory0=handle->memory;
        handle->memory =memory;
        handle->num = array->num+n+256;
        array->dataS8 = memory->data;
    }
    if(data!=NULL)
    {
        mException(n<=0,EXIT,"invalid input");
        memcpy(array->dataU8+array->num*es,data,n*es);
    }
    array->num+=n;
    ((struct _MArray *)array)->capacity = (handle->num-array->num)&0x0FFFF;
    if(memory0!=NULL) mMemoryBlockRelease(memory0);
}

// void ArrayAppend(MArray *arr,int n)
// {
//     struct _MArray *array = (struct _MArray *)arr;
//     mException((INVALID_POINTER(array)),EXIT,"invalid input");
//     if(n<=0) n = array->num+1;
//     
//     if(n-array->num>array->capacity)
//     {
//         struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
//         if(n>handle->num)
//         {
//             MMemoryBlock *memory = mMemoryBlockCreate((n+256)*array->element_size,MORN_HOST);
//             if(array->num>0) memcpy(memory->data,array->dataS8,array->num*array->element_size);
//             if(handle->memory!=NULL) mMemoryBlockRelease(handle->memory);
//             handle->memory =memory;
//             handle->num = n+256;
//             array->dataS8 = memory->data;
//         }
//         array->capacity = (handle->num-n)&0x0FFFF;
//     }
//     else array->capacity -= (n-array->num);
//     array->num = n;
// }

void *m_ArrayPushBack(MArray *arr,void *data)
{
    struct _MArray *array = (struct _MArray *)arr;
    MMemoryBlock *memory0=NULL;
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
            memory0=handle->memory;
            handle->memory=memory;
            array->dataS8 = memory->data;
        }
        array->capacity=(handle->num-array->num)&0x0FFFF;
    }
    int8_t *p=(int8_t *)(array->dataS8+array->num*es);
    if(data==NULL) NULL;
    else if(es==4) *((int32_t *)p) = *((int32_t *)data);
    else if(es==8) *((int64_t *)p) = *((int64_t *)data);
    else if(es==1) *(           p) = *(( int8_t *)data);
    else if(es==2) *((int16_t *)p) = *((int16_t *)data);
    else memcpy(p,data,es);
    array->num++;
    array->capacity--;
    if(memory0!=NULL) mMemoryBlockRelease(memory0);
    return (void *)p;
}

void *m_ArrayWrite(MArray *arr,intptr_t n,void *data,int num)
{
    if(num==0) return NULL;
    if(num<=1)
    {
        if(n<0) return m_ArrayPushBack(arr,data);
        int es=arr->element_size;
        int8_t *p=(int8_t *)(arr->dataS8+n*es);
        if(data==NULL) NULL;
        else if(es==4) *((int32_t *)p) = *((int32_t *)data);
        else if(es==8) *((int64_t *)p) = *((int64_t *)data);
        else if(es==1) *(           p) = *(( int8_t *)data);
        else if(es==2) *((int16_t *)p) = *((int16_t *)data);
        else memcpy(p,data,es);
        return (void *)p;
    }
    else
    {
        struct _MArray *array = (struct _MArray *)arr;
        MMemoryBlock *memory0=NULL;
        int es=array->element_size;
        if(n<0) n=array->num;
        if(n+num-array->num>array->capacity)
        {
            struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
            if(n+num>handle->num)
            {
                MMemoryBlock *memory = mMemoryBlockCreate((n+num+256)*es,MORN_HOST);
                if(array->num>0) memcpy(memory->data,array->dataS8,array->num*es);
//                 if(handle->memory!=NULL) mMemoryBlockRelease(handle->memory);
                memory0=handle->memory;
                handle->memory =memory;
                handle->num = n+num+256;
                array->dataS8 = memory->data;
            }
            array->capacity = (handle->num-n-num)&0x0FFFF;
        }
        else if(n+num>array->num) array->capacity -= (n+num-array->num);
        array->num = MAX(n+num,array->num);
        
        int8_t *p=(int8_t *)(arr->dataS8+n*es);
        if(data!=NULL) memcpy(p,data,num*es);
        if(memory0!=NULL) mMemoryBlockRelease(memory0);
        return (void *)p;
    }
}

void *m_ArrayRead(MArray *array,int n,void *data,int num)
{
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    mException(n>=array->num,EXIT,"invalid input");
    
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)(ObjHandle(array,0)->handle);
    if(n<=0) {n=handle->read_order;if(n>=array->num) n=0;}
    
    int element_size = array->element_size;
    if(num<1) num=1;
    if(n+num>array->num) return NULL;
    handle->read_order=n+num;
    
    int8_t *p=array->dataS8+n*element_size;
    if(data!=NULL) memcpy(data,p,element_size*num);
    return p;
}

struct HandleStream
{
    int8_t *read;
    int8_t *write;
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

struct HandleArrayUnique
{
    uint8_t *pflag;
    uint8_t **flag;
    int es;
};
void endArrayUnique(struct HandleArrayUnique *handle)
{
    if(handle->pflag!=NULL) mFree(handle->pflag);
}
#define HASH_ArrayUnique 0x731ecd63
void mArrayUnique(MArray *array)
{
    int es=array->element_size;
    MHandle *hdl=mHandle(array,ArrayUnique);
    struct HandleArrayUnique *handle=hdl->handle;
    if((hdl->valid==0)||(handle->es!=es))
    {
        if(handle->pflag!=NULL) mFree(handle->pflag);
        handle->pflag = mMalloc(es*256+es*sizeof(uint8_t *));
        handle->es=es;
        handle->flag=(uint8_t **)(handle->pflag+es*256);
        handle->flag[0]=handle->pflag;
        for(int n=1;n<es;n++) handle->flag[n]=handle->flag[n-1]+256;
        hdl->valid=1;
    }
    memset(handle->pflag,0,es*256);
    uint8_t **flag=handle->flag;
    uint8_t *p0=array->dataU8;
    uint8_t *p1=p0+es*array->num;
    int num=0;
    for(uint8_t *p=p0;p<p1;p+=es)
    {
        int n=0;
        for(int i=0;i<es;i++) {int idx=p[i];n+=flag[i][idx];flag[i][idx]=1;}
        if(n==es)
        {
            uint8_t *q;for(q=p0;q<p;q+=es) {if(memcmp(q,p,es)==0) break;}
            if(q==p) n=0;
        }
        if(n<es)
        {
                 if(es==4) ((int32_t *)p0)[num] = *((int32_t *)p);
            else if(es==8) ((int64_t *)p0)[num] = *((int64_t *)p);
            else if(es==1)             p0 [num] = *            p ;
            else if(es==2) ((int16_t *)p0)[num] = *((int16_t *)p);
            else memcpy(p0+num*es,p,es);
            num++;
        }
    }
    array->num=num;
}
