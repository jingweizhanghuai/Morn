#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

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
    struct HandleTableCreate *handle = info;
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
    
    if(col <0) col = 0;
    if(row <0) row = 0;
    
    tab->col = col;
    tab->row = row;
    
    MHandle *hdl; ObjectHandle(tab,TableCreate,hdl);
    
    struct HandleTableCreate *handle = hdl->handle;
    handle->tab = tab;
    
    if(row == 0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        return tab;
    }
    
    if(!INVALID_POINTER(data))
    {
        tab->data = data;
        return tab;
    }
    
    handle->index = (void **)mMalloc(row*sizeof(void *));
    handle->row = row;
    
    if(col == 0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        memset(handle->index,0,row*sizeof(void *));
    }
    else 
    {
        if(handle->memory == NULL) handle->memory = mMemoryCreate(row,col*element_size);
        mMemoryIndex(handle->memory,row,col*element_size,(void **)(handle->index));

        handle->col = col;
        handle->element_size = element_size;
        tab->data = handle->index;
    }
    
    return tab;
}

void mTableRelease(MTable *tab)
{
    mException(INVALID_POINTER(tab),EXIT,"invalid input");
    
    if(!INVALID_POINTER(tab->handle))
        mHandleRelease(tab->handle);
}

void TableRedefine(MTable *tab,int row,int col,int element_size)
{
    mException((INVALID_POINTER(tab)),EXIT,"invalid input");
    
    struct HandleTableCreate *handle;
    handle = ((MHandle *)(tab->handle->data[0]))->handle;
    
    if(row<=0) row = tab->row;
    if(col<=0) col = tab->col;
    if(element_size<=0) element_size = handle->element_size;
    if((row==tab->row)&&(col==tab->col)&&(element_size==handle->element_size))
        return;
    
    tab->row = row;
    tab->col = col;
    mHandleReset(tab->handle);
    if((row == 0)||(col==0)) {tab->data=NULL; return;}
    
    if(row>handle->row)
    {
        if(handle->index != NULL)
            mFree(handle->index);
        handle->index = NULL;
        
        handle->col = 0;
    }
    
    if(handle->index == NULL)
        handle->index = (void **)mMalloc(row*sizeof(void *));
    handle->row = row;
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(row,col*element_size);
    mMemoryIndex(handle->memory,row,col*element_size,handle->index);
    
    handle->col = col;
    handle->element_size = element_size;
    
    tab->data = handle->index;
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
    struct HandleArrayCreate *handle = info;
    mException((handle->array == NULL),EXIT,"invalid array");
    
    if(handle->memory!= NULL) mMemoryRelease(handle->memory);
    
    mFree(handle->array);
}
#define HASH_ArrayCreate 0xb3feafa4
MArray *ArrayCreate(int num,int element_size,void *data)
{
    MArray *array = (MArray *)mMalloc(sizeof(MArray));
    memset(array,0,sizeof(MArray));   
    
    if(num<0) num = 0;
    array->num = num;
    
    MHandle *hdl; ObjectHandle(array,ArrayCreate,hdl);
    struct HandleArrayCreate *handle = hdl->handle;
    handle->array = array;
    
    if(num==0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
    }
    else if(INVALID_POINTER(data))
    {
        handle->memory = mMemoryCreate(1,num*element_size);
        handle->num = num;
        handle->element_size = element_size;
        array->data = (void *)(handle->memory->data[0]);
    }
    else
        array->data = data;
    
    return array;
}

void mArrayRelease(MArray *array)
{   
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    
    if(!INVALID_POINTER(array->handle))
        mHandleRelease(array->handle);
    
    mFree(array);
}

void ArrayRedefine(MArray *array,int num,int element_size)
{
    mException(INVALID_POINTER(array),EXIT,"invalid input");
    
    struct HandleArrayCreate *handle;
    handle = ((MHandle *)(array->handle->data[0]))->handle;
    
    if(num <= 0) num = array->num;
    if(num == array->num)
        return;
    
    array->num = num;
    mHandleReset(array->handle);
    if(num == 0) {array->data = NULL; return;}
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(1,num*element_size);
    mMemoryIndex(handle->memory,1,num*element_size,&(array->data));
    
    handle->num = num;
    handle->element_size = element_size;
}



