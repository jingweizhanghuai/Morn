#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

struct HandleSheetCreate
{
    MSheet *sheet;
    int row;
    
    int *col;
    void ***data;
    int *num;
    MInfo *row_info;
    
    MMemory *memory;
};
void endSheetCreate(void *info)
{
    struct HandleSheetCreate *handle = (struct HandleSheetCreate *)info;
    mException((handle->sheet == NULL),EXIT,"invalid sheet");
    
    if(handle->data!=NULL)
    {
        for(int j=0;j<handle->row;j++)
            if(handle->data[j] != NULL)
                mFree(handle->data[j]);
        mFree(handle->data);
    }
    if(handle->col != NULL) mFree(handle->col);
    if(handle->num != NULL) mFree(handle->num);
    if(handle->row_info != NULL) mFree(handle->row_info);
    if(handle->memory !=NULL) mMemoryRelease(handle->memory);
    
    mFree(handle->sheet);
}
#define HASH_SheetCreate 0x3cb067ca
MSheet *mSheetCreate(int row,int *col,void ***data)
{
    MSheet *sheet = (MSheet *)mMalloc(sizeof(MSheet));
    memset(sheet,0,sizeof(MSheet));
    
    MHandle *hdl; ObjectHandle(sheet,SheetCreate,hdl);
    struct HandleSheetCreate *handle = hdl->handle;
    handle->sheet = sheet;
    
    if(row<0) row = 0;
    handle->row = row;
    sheet->row = row;
    
    if(row == 0) 
    {
        mException((col != NULL)||(data!= NULL),EXIT,"invalid input");
        sheet->col = NULL;
        sheet->data = NULL;
        sheet->row_info = NULL;
        return sheet;
    }
    
    handle->data= (void ***)mMalloc(row*sizeof(void **));
    handle->num = (int *)mMalloc(row*sizeof(int));
    handle->col = (int *)mMalloc(row*sizeof(int));
    handle->row_info =   mMalloc(row*sizeof(MInfo));
    memset(handle->row_info,0,row*sizeof(MInfo));
    if(col!= NULL)
    {
        for(int j=0;j<row;j++)
        {
            if(col[j] <= 0) 
            {
                handle->data[j]= NULL;
                handle->num[j] = 0;
                handle->col[j] = 0;
                continue;
            }
            handle->data[j] = mMalloc(col[j]*sizeof(void *));
            handle->col[j] = col[j];
            handle->num[j] = col[j];
            
            if(data != NULL)
                memcpy(handle->data[j],data[j],col[j]*sizeof(void *));
            else
                memset(handle->data[j],0      ,col[j]*sizeof(void *));
        }
    }
    else
    {
        mException((data!=NULL),EXIT,"invalid input");
        memset(handle->data    ,0,row*sizeof(void **));
        memset(handle->num     ,0,row*sizeof(int));
        memset(handle->col     ,0,row*sizeof(int));
    }

    sheet->col = handle->col;
    sheet->data= handle->data;
    sheet->row_info = handle->row_info;
    
    return sheet;
}

void mSheetRelease(MSheet *sheet)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input source sheet");
    
    if(!INVALID_POINTER(sheet->handle))
        mHandleRelease(sheet->handle);
}

void mSheetRowAppend(MSheet *sheet,int n)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input source sheet");
    if(n<0) n = 1;
    
    struct HandleSheetCreate *handle= ((MHandle *)(sheet->handle->data[0]))->handle;
    if(sheet->row+n>handle->row)
    {
        int row = sheet->row + MAX(n,8);
        void ***handle_data   =(void ***)mMalloc(row*sizeof(void **));
        int    *handle_num    =   (int *)mMalloc(row*sizeof(int));
        int    *handle_col    =   (int *)mMalloc(row*sizeof(int));
        MInfo *handle_row_info= (MInfo *)mMalloc(row*sizeof(MInfo));
        if(handle->row >0)
        {
            memcpy(handle_data    ,handle->data    ,handle->row*sizeof(void **));
            memcpy(handle_num     ,handle->num     ,handle->row*sizeof(int));
            memcpy(handle_col     ,handle->col     ,handle->row*sizeof(int));
            memcpy(handle_row_info,handle->row_info,handle->row*sizeof(MInfo));
        }
        memset(handle_data    +handle->row,0,(row-handle->row)*sizeof(void **));
        memset(handle_num     +handle->row,0,(row-handle->row)*sizeof(int));
        memset(handle_col     +handle->row,0,(row-handle->row)*sizeof(int));
        memset(handle_row_info+handle->row,0,(row-handle->row)*sizeof(MInfo));
        
        if(handle->data    !=NULL) {mFree(handle->data);    } handle->data    = handle_data;
        if(handle->num     !=NULL) {mFree(handle->num);     } handle->num     = handle_num;
        if(handle->col     !=NULL) {mFree(handle->col);     } handle->col     = handle_col;
        if(handle->row_info!=NULL) {mFree(handle->row_info);} handle->row_info= handle_row_info;
        
        sheet->data    = handle->data;
        sheet->col     = handle->col;
        sheet->row_info= handle->row_info;
        
        handle->row = row;
    }
    sheet->row = sheet->row+n;
}

void mSheetColAppend(MSheet *sheet,int row,int n)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input source sheet");
    mException((row<0)||(row>=sheet->row),EXIT,"invalid input row");
    if(n<0) n = 1;
    
    struct HandleSheetCreate *handle= ((MHandle *)(sheet->handle->data[0]))->handle;
    if(sheet->col[row] + n > handle->num[row])
    {
        int col = sheet->col[row] + MAX(n,64);
        void **sheet_data = (void **)mMalloc(col*sizeof(void *));
        if(sheet->col[row] >0)
            memcpy(sheet_data,sheet->data[row],sheet->col[row]*sizeof(void *));
        memset(sheet_data + sheet->col[row],0,(col - sheet->col[row])*sizeof(void *));
        
        if(sheet->data[row]!=NULL) mFree(sheet->data[row]);
        handle->data[row] = sheet_data;
        handle->num[row] = col;
        sheet->data[row] = handle->data[row];
    }
    
    sheet->col[row] = sheet->col[row] + n;
}

struct HandleSheetWrite
{
    int write_size;
};
void endSheetWrite(void *info) {NULL;}
#define HASH_SheetWrite 0x815f53f7
void *mSheetWrite(MSheet *sheet,int row,int col,void *data,int size)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input source sheet");
    mException((size<=0)&&(INVALID_POINTER(data)),EXIT,"invalid input sheet element size");
    mException((row>sheet->row)||(row<0),EXIT,"invalid write location");
    if(size<=0) size = strlen(data)+1;
    
    struct HandleSheetCreate *handle0 = ((MHandle *)(sheet->handle->data[0]))->handle;
    
    if(row == sheet->row) 
        mSheetRowAppend(sheet,DFLT);
    
    mException((col>sheet->col[row]),EXIT,"invalid write location");
    
    if(col<0) col = sheet->col[row];
    if(col == sheet->col[row]) 
        mSheetColAppend(sheet,row,DFLT); 
    
    if(handle0->memory == NULL)
        handle0->memory = mMemoryCreate(DFLT,DFLT);
    sheet->data[row][col] = mMemoryWrite(handle0->memory,data,size);
    
    if((row!=sheet->row-1)&&(col!=sheet->col[row]-1))
    {
        MHandle *hdl; ObjectHandle(sheet,SheetWrite,hdl);
        struct HandleSheetWrite *handle = hdl->handle;
        handle->write_size += size;
        
        if(handle->write_size>16384)
        {
            int num = 0; for(int j=0;j<sheet->row;j++) num += sheet->col[j];
            void **index = mMalloc(sizeof(void *)*num);
            num = 0;
            for(int j=0;j<sheet->row;j++) 
            {
                memcpy(index+num,sheet->data[j],sheet->col[j]*sizeof(void *));
                num += sheet->col[j];
            }
            
            mMemoryCollect(handle0->memory,index,num);
            handle->write_size = 0;
            mFree(index);
        }
    }
    return (sheet->data[row][col]);
}
    
struct HandleSheetRead
{
    int row_order;
    int col_order;
};
void endSheetRead(void *info) {NULL;}
#define HASH_SheetRead 0x4c826020
void *mSheetRead(MSheet *sheet,int row,int col,void *data,int size)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input");
    mException((size <=0)||(INVALID_POINTER(data)),EXIT,"invalid input"); 
    
    MHandle *hdl; ObjectHandle(sheet,SheetRead,hdl);
    struct HandleSheetRead *handle = hdl->handle;

    if(row<0)
    {
        row = handle->row_order; 
        if(col>=0) 
        {
            mException((col>=sheet->col[row]),EXIT,"invalid input");
            handle->col_order = col;
        }
        else
        {
            if(handle->col_order < sheet->col[row]-1)
            {
                handle->col_order += 1;
                col = handle->col_order;
            }
            else
            {
                if(row == sheet->row-1) col = handle->col_order;
                else
                {
                    handle->row_order += 1;
                    row = handle->row_order;
                    handle->col_order = 0;
                    col = 0;
                }
            }
        }
    }
    else
    {
        mException((row>=sheet->row),EXIT,"invalid input");
        handle->row_order = row;
        if(col>=0) 
        {
            mException((col>=sheet->col[row]),EXIT,"invalid input");
            handle->col_order = col;
        }
        else
        {
            if(handle->col_order < sheet->col[row]-1)
                handle->col_order += 1;
            col = handle->col_order;
        }
    }
    if(data!=NULL)
    {
        if(size>0)memcpy(data,sheet->data[row][col],size);
        else      strcpy(data,sheet->data[row][col]);
    }
    return sheet->data[row][col];
}

void mSheetElementDelete(MSheet *sheet,int row,int col)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input");
    mException((row>=sheet->row),EXIT,"invalid input");
    mException((col>=sheet->col[row]),EXIT,"invalid input");
    memmove(sheet->data[row]+col,sheet->data[row]+col+1,(sheet->col[row]-col-1)*sizeof(void *));
}

void mSheetElementInsert(MSheet *sheet,int row,int col,void *data,int size)
{
    void *buff;
    
    mSheetWrite(sheet,row,DFLT,data,size);
    buff = sheet->data[row][sheet->col[row]-1];
    
    memmove(sheet->data[row]+col+1,sheet->data[row]+col,(sheet->col[row]-col-1)*sizeof(void *));
    sheet->data[row][col] = buff;
}

void mSheetReorder(MSheet *sheet)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input source sheet");
    
    int i,j,k;
    void *buff;
    for(j=0;j<sheet->row;j++)
    {
        void **data = sheet->data[j];
        int num = sheet->col[j];
        
        for(i=0;i<num;i++)
        {
            k = mRand(0,num);
            buff = data[i]; data[i] = data[k]; data[k] = buff;
        }
    }
}

struct HandleSheetRowList
{
    MList *list;
};
void endSheetRowList(void *info)
{
    struct HandleSheetRowList *handle = info;
    for(int i=0;i<handle->list->num;i++)
    {
        MList **list = handle->list->data[i];
        mListRelease(*list);
    }
    mListRelease(handle->list);
}
#define HASH_SheetRowList 0x40b550d6
MList *mSheetRowList(MSheet *sheet,int row)
{
    mException(INVALID_POINTER(sheet),EXIT,"invalid input source sheet");
    mException((row>=sheet->row),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(sheet,SheetRowList,hdl);
    struct HandleSheetRowList *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        if(handle->list == NULL) handle->list=mListCreate(DFLT,NULL);
        hdl->valid = 1;
    }
    
    MList *list = mListCreate(sheet->col[row],sheet->data[row]);
    memcpy(&(list->info),&(sheet->row_info[row]),sizeof(MInfo));
    mListWrite(handle->list,DFLT,&list,sizeof(MList *));
    
    return list;
}



