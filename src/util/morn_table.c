/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

struct HandleTableCreate
{
    MTable *tab;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    
    int row;
    int col;
    int element_size;
    void **index;
    MMemoryBlock *memory;
};
void endTableCreate(struct HandleTableCreate *handle)
{
    mException((handle->tab==NULL),EXIT,"invalid table");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->index != NULL) mFree(handle->index);
    if(handle->memory!= NULL) mMemoryBlockRelease(handle->memory);
    memset(handle->tab,0,sizeof(MTable));
    // mFree(((MList **)(handle->tab))-1);
}
#define HASH_TableCreate 0x56f55a7f
MTable *TableCreate(int row,int col,int element_size,void **data)
{
    MTable *tab = ObjectAlloc(sizeof(MTable));

    if(col <0) {col = 0;} tab->col = col;
    if(row <0) {row = 0;} tab->row = row;
    if(element_size<0) {element_size=0;} tab->element_size=element_size;

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
        if(handle->memory == NULL) handle->memory = mMemoryBlockCreate(row*col*element_size,MORN_HOST);

        char *p = (char *)(handle->memory->data);
        for(int j=0;j<row;j++) {handle->index[j]=p;p+=col*element_size;}
    
        handle->col = col;
        handle->element_size = element_size;
        tab->data = handle->index;
    }
    // else mException(1,EXIT,"invalid input");
    mPropertyFunction(tab,"device",mornMemoryDevice,handle->memory);
    
    return tab;
}

void mTableRelease(MTable *tab)
{
    ObjectFree(tab);
}

void TableRedefine(MTable *tab,int row,int col,int element_size,void **data)
{
    mException((INVALID_POINTER(tab)),EXIT,"invalid input");
    
    struct HandleTableCreate *handle = (struct HandleTableCreate *)(ObjHandle(tab,0)->handle);
    if(row<=0) row = tab->row;
    if(col<=0) col = tab->col;
    if(element_size<=0) element_size=tab->element_size;
    
    if((row!=tab->row)||(col!=tab->col)||(element_size!=handle->element_size)) mHandleReset(tab);
    
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

    if(handle->memory!=NULL) mMemoryBlockRelease(handle->memory);
    handle->memory = mMemoryBlockCreate(row*col*element_size,MORN_HOST);
    char *p = (char *)(handle->memory->data);
    for(int j=0;j<row;j++) {handle->index[j]=p;p+=col*element_size;}
    
    handle->col = col;
    handle->element_size = element_size;
}

void mTableCopy(MTable *src,MTable *dst)
{
    struct HandleTableCreate *handle = (struct HandleTableCreate *)(ObjHandle(src,0)->handle);
    int element_size = handle->element_size;
    
    TableRedefine(dst,src->row,src->col,element_size,(void **)(dst->dataS8));
    for(int j=0;j<src->row;j++)
        memcpy(dst->data[j],src->data[j],src->col*element_size);
}

void mTableWipe(MTable *tab)
{
    for(int j=0;j<tab->row;j++) memset(tab->dataS8[j],0,tab->col*tab->element_size);
}

