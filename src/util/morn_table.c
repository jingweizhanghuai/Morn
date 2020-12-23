/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

// int ElementSize(const char *str,int size)
// {
//     if(size!=sizeof(DFLT)) return size;
//     if(strcmp(str,"DFLT")==0) return -1;
//     return size;
// }

struct HandleTableCreate
{
    MTable *tab;
    int row;
    int col;
    int element_size;
    void **index;
    MMemory *memory;
};
void endTableCreate(void *info)
{
    struct HandleTableCreate *handle = (struct HandleTableCreate *)info;
    mException((handle->tab==NULL),EXIT,"invalid table");

    if(handle->index != NULL) mFree(handle->index);
    if(handle->memory!= NULL) mMemoryRelease(handle->memory);
    
    mFree(handle->tab);
}
#define HASH_TableCreate 0x56f55a7f
MTable *TableCreate(int row,int col,int element_size,void **data)
{
    MTable *tab = (MTable *)mMalloc(sizeof(MTable));
    memset(tab,0,sizeof(MTable));
    
    if(col <0) {col = 0;} tab->col = col;
    if(row <0) {row = 0;} tab->row = row;
    if(element_size<0) {element_size=0;} tab->element_size=element_size;

    tab->handle = mHandleCreate();
    MHandle *hdl=mHandle(tab,TableCreate);
    struct HandleTableCreate *handle = (struct HandleTableCreate *)(hdl->handle);
    handle->tab = tab;
    
    if(row == 0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        return tab;
    }
    
    handle->index = (void **)mMalloc(row*sizeof(void *));
    handle->row = row;

    tab->data=NULL;
    if(col == 0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        memset(handle->index,0,row*sizeof(void *));
    }
    else if(!INVALID_POINTER(data))
    {
        memcpy(handle->index,data,row*sizeof(void *));
        tab->data = handle->index;
    }
    else if(element_size>0)
    {
        if(handle->memory == NULL) handle->memory = mMemoryCreate(1,row*col*element_size,MORN_HOST);
        mException((handle->memory->num!=1),EXIT,"invalid table memory");
        mMemoryIndex(handle->memory,row,col*element_size,&(handle->index),1);

        handle->col = col;
        handle->element_size = element_size;
        tab->data = handle->index;
    }
    // else mException(1,EXIT,"invalid input");
    
    return tab;
}

void mTableRelease(MTable *tab)
{
    mException(INVALID_POINTER(tab),EXIT,"invalid input");
    
    if(!INVALID_POINTER(tab->handle))
        mHandleRelease(tab->handle);
}

void TableRedefine(MTable *tab,int row,int col,int element_size,void **data)
{
    mException((INVALID_POINTER(tab)),EXIT,"invalid input");
    
    struct HandleTableCreate *handle = (struct HandleTableCreate *)(((MHandle *)(tab->handle->data[0]))->handle);
    if(row<=0) row = tab->row;
    if(col<=0) col = tab->col;
    if(element_size<=0) element_size=tab->element_size;
    
    if((row!=tab->row)||(col!=tab->col)||(element_size!=handle->element_size)) mHandleReset(tab->handle);
    
    int same_size = (row<=tab->row)&&(col*element_size<=tab->col*handle->element_size);
    int reuse = (data==tab->data);
    int flag = (tab->row&&tab->col);
    tab->row = row;tab->col = col;tab->element_size=element_size;
    handle->element_size = element_size;
    
    if(same_size&&reuse) return;
    if(same_size&&(INVALID_POINTER(data))&&(handle->col>0)) return;
    
    mException(reuse&&flag&&(handle->col==0),EXIT,"invalid redefine");
    
    handle->col=0;
    if((row<=0)||(col<=0)||(element_size<=0)) 
    {
        mException((!INVALID_POINTER(data))&&(!reuse),EXIT,"invalid input");
        tab->data=NULL; 
        return;
    }
    
    if(reuse) data=NULL;
    
    if(row>handle->row)
    {
        if(handle->index != NULL)mFree(handle->index);
        handle->index = NULL;
    }
    if(handle->index == NULL) 
    {
        handle->index = (void **)mMalloc(row*sizeof(void *));
        handle->row = row;
    }
    tab->data = handle->index;
    
    if(!INVALID_POINTER(data)) {memcpy(handle->index,data,row*sizeof(void *));return;}
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(1,row*col*element_size,MORN_HOST);
    else mMemoryRedefine(handle->memory,1,row*col*element_size,MORN_HOST);
    mException((handle->memory->num!=1),EXIT,"invalid table memory");
    mMemoryIndex(handle->memory,row,col*element_size,&(handle->index),1);
    handle->col = col;
    handle->element_size = element_size;
}

void mTableCopy(MTable *src,MTable *dst)
{
    struct HandleTableCreate *handle = (struct HandleTableCreate *)(((MHandle *)(src->handle->data[0]))->handle);
    int element_size = handle->element_size;
    
    TableRedefine(dst,src->row,src->col,element_size,(void **)(dst->dataS8));
    for(int j=0;j<src->row;j++)
        memcpy(dst->data[j],src->data[j],src->col*element_size);
}

struct HandleArrayCreate
{
    MArray *array;
    int num;
    int element_size;
    MMemory *memory;
};
void endArrayCreate(void *info)
{
    struct HandleArrayCreate *handle = (struct HandleArrayCreate *)info;
    mException((handle->array == NULL),EXIT,"invalid array");
    
    if(handle->memory!= NULL) mMemoryRelease(handle->memory);
    
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
        handle->memory = mMemoryCreate(1,num*element_size,MORN_HOST);
        handle->num = num;
        handle->element_size = element_size;
        array->data = handle->memory->data[0];
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
        if(handle->memory==NULL) handle->memory = mMemoryCreate(1,num*element_size,MORN_HOST);
        else mMemoryRedefine(handle->memory,1,num*element_size,MORN_HOST);
        array->data = handle->memory->data[0];
    }
}

struct HandleStream
{
    char *read;
    char *write;
    pthread_mutex_t stream_mutex;
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
    pthread_mutex_lock(&(handle->stream_mutex));
    
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
    pthread_mutex_unlock(&(handle->stream_mutex));
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
        pthread_mutex_init(&(handle->stream_mutex),NULL);
        // handle->stream_mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    pthread_mutex_lock(&(handle->stream_mutex));
    
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
    pthread_mutex_unlock(&(handle->stream_mutex));
    return ((size-num)/buff->element_size);
}


