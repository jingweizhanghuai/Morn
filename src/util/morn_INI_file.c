#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

struct KeyValue
{
    char *name;
    char *value;
};

struct HandleINIRead
{
    char *data;
    int size;
    
    MList *list;
    MSheet *sheet;
};
void endINIRead(void *info)
{
    struct HandleINIRead *handle = info;
    if(handle->list !=NULL) mListRelease(handle->list);
    if(handle->sheet!=NULL) mSheetRelease(handle->sheet);
}
#define HASH_INIRead 0x5baa94c1     
void INIOpen(MObject *file,struct HandleINIRead *handle)
{
    FILE *f = fopen(file->object,"r");
    mException((f == NULL),EXIT,"file cannot open");
    fseek(f,0,SEEK_END);
    int file_size = ftell(f);
    fseek(f,0,SEEK_SET);
    if(handle->size < file_size)
    {
        if(handle->data!=NULL)
            mFree(handle->data);
        handle->data = mMalloc(file_size+1);
        handle->size = file_size;
    }
    fread(handle->data,file_size,1,f);
    handle->data[file_size]=0;
    fclose(f);
    
    if(handle->list == NULL) handle->list = mListCreate(DFLT,NULL);
    if(handle->sheet== NULL) handle->sheet= mSheetCreate(DFLT,NULL,NULL);
    MList *list = handle->list;
    MSheet *sheet = handle->sheet;
    
    struct KeyValue kv;
    
    int flag = 0;
    for(char *p=handle->data;p<handle->data+file_size;p++)
    {
        // printf("flag is %d,%c\n",flag,p[0]);
        if((p[0] == '\n')||(p[0] == '\r'))
            {p[0] = 0;flag = 0;continue;}
        
        if(flag == 7) continue;
        
        switch(p[0])
        {
            case(' '):
            {
                p[0]=0;continue;
            }
            case('\t'):
            {
                p[0]=0;continue;
            }
            case('='):
            {
                mException((flag!=1),EXIT,"invalid ini");
                flag = 2;p[0]=0;continue;
            }
            case('['):
            {
                mException((flag!=0),EXIT,"invalid ini");
                flag = 4;p[0]=0;continue;
            }
            case(']'):
            {
                mException((flag!=5),EXIT,"invalid ini");
                flag = 6;p[0]=0;continue;
            }
            case(';'):
            {
                mException((flag!=0)&&(flag!=3)&&(flag!=6),EXIT,"invalid ini");
                flag = 7;p[0]=0;continue;
            }
            case('#'):
            {
                mException((flag!=0)&&(flag!=2)&&(flag!=5),EXIT,"invalid ini");
                flag = 7;p[0]=0;continue;
            }
            default:
            {
                switch(flag)
                {
                    case(0):
                    {
                        kv.name = p;
                        flag = 1;continue;
                    }
                    case(2):
                    {
                        kv.value = p;
                        mSheetWrite(sheet,list->num-1,DFLT,&kv,sizeof(struct KeyValue));kv.name=NULL;kv.value=NULL;
                        flag = 3;continue;
                    }
                    case(4):
                    {
                        mListAppend(list,DFLT);list->data[list->num-1] = p;
                        mSheetRowAppend(sheet,DFLT);
                        flag = 5;continue;
                    }
                    default:
                        NULL;
                }
            }
        }
        
    }
}

char *mINIRead(MObject *file,const char *section,const char *key)
{
    int i,j;
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    mException(INVALID_POINTER(section)||INVALID_POINTER(key),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(file,INIRead,hdl);
    struct HandleINIRead *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        INIOpen(file,handle);
        hdl->valid = 1;
    }
    MList  *list = handle->list;
    MSheet *sheet= handle->sheet;
    
    struct KeyValue *kv;
    for(i=0;i<list->num;i++)
    {
        if(strcmp(section,list->data[i])==0)
        {
            for(j=0;j<sheet->col[i];j++)
            {
                kv = sheet->data[i][j];
                if(strcmp(key,kv->name)==0)
                    return kv->value;
            }
            if(j==sheet->col[i])
                return NULL;
        }
    }
    return NULL;
}

MList *mINISection(MFile *file)
{
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    MHandle *hdl; ObjectHandle(file,INIRead,hdl);
    struct HandleINIRead *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        INIOpen(file,handle);
        hdl->valid = 1;
    }
    return handle->list;
}

struct HandleINIKey
{
    MList *key;
};
void endINIKey(void *info)
{
    struct HandleINIKey *handle = info;
    if(handle->key != NULL) mListRelease(handle->key);
}
#define HASH_INIKey 0x5b939d8c
MList *mINIKey(MFile *file,const char *section)
{
    int i,j;
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    mException(INVALID_POINTER(section),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(file,INIRead,hdl);
    struct HandleINIRead *handle0 = hdl->handle;
    if(hdl->valid == 0)
    {
        INIOpen(file,handle0);
        hdl->valid = 1;
    }
    MList  *list = handle0->list;
    MSheet *sheet= handle0->sheet;
    
    ObjectHandle(file,INIKey,hdl);
    struct HandleINIKey *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        handle->key = mListCreate(DFLT,NULL);
        hdl->valid = 1;
    }
    
    struct KeyValue *kv;
    for(i=0;i<list->num;i++)
    {
        if(strcmp(section,list->data[i])==0)
        {
            if(handle->key->num<sheet->col[i])
                mListAppend(handle->key,sheet->col[i]-handle->key->num);
            handle->key->num = sheet->col[i];
            
            for(j=0;j<sheet->col[i];j++)
            {
                kv = sheet->data[i][j];
                handle->key->data[j] = kv->name;
            }
            return handle->key;
        }
    }
    return NULL;
}

