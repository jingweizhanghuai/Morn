/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"
#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

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
    struct HandleINIRead *handle = (struct HandleINIRead *)info;
    if(handle->data != NULL) mFree(handle->data);
    if(handle->list !=NULL) mListRelease(handle->list);
    if(handle->sheet!=NULL) mSheetRelease(handle->sheet);
}
#define HASH_INIRead 0x5baa94c1     
void INIOpen(MObject *file,struct HandleINIRead *handle)
{
    FILE *f = fopen((const char *)(file->object),"rb");
    mException((f == NULL),EXIT,"file cannot open");
    fseek(f,0,SEEK_END);
    int file_size = ftell(f);
    // printf("file_size is %d\n",file_size);
    fseek(f,0,SEEK_SET);
    if(handle->size < file_size)
    {
        if(handle->data!=NULL)
            mFree(handle->data);
        handle->data = (char *)mMalloc(file_size+1);
        handle->size = file_size;
    }
    fread(handle->data,file_size,1,f);
    handle->data[file_size]=0;
    fclose(f);
    
    // printf("%s\n",handle->data);
    // printf("handle->data[8] is %c(%d)\n",handle->data[8],handle->data[8]);
    // printf("handle->data[9] is %c(%d)\n",handle->data[9],handle->data[9]);
    // printf("handle->data[10] is %c(%d)\n",handle->data[10],handle->data[10]);
    
    if(handle->list == NULL) handle->list = mListCreate(DFLT,NULL);
    if(handle->sheet== NULL) handle->sheet= mSheetCreate(DFLT,NULL,NULL);
    MList *list = handle->list;
    MSheet *sheet = handle->sheet;
    
    struct KeyValue kv;
    
    int flag = 0;
    for(char *p=handle->data;p<handle->data+file_size;p++)
    {
        // printf("%x,flag is %d,%c\n",p-handle->data,flag,p[0]);
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

char *INIRead(MObject *file,const char *section,const char *key,char *format,void *p1,void *p2,void *p3,void *p4,void *p5,void *p6,void *p7,void *p8,void *p9,void *p10,void *p11,void *p12,void *p13,void *p14)
{
    int i,j;
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    mException(INVALID_POINTER(section)||INVALID_POINTER(key),EXIT,"invalid input");
    
    MHandle *hdl=mHandle(file,INIRead);
    struct HandleINIRead *handle = (struct HandleINIRead *)(hdl->handle);
    if(hdl->valid == 0)
    {
        INIOpen(file,handle);
        hdl->valid = 1;
    }
    MList  *list = handle->list;
    MSheet *sheet= handle->sheet;
    
    struct KeyValue *kv;
    for(i=0;i<list->num;i++)
        if(strcmp(section,(char *)(list->data[i]))==0)
        {
            for(j=0;j<sheet->col[i];j++)
            {
                kv = (struct KeyValue *)(sheet->data[i][j]);
                if(strcmp(key,kv->name)==0)
                {
                    if(!INVALID_POINTER(format)) sscanf(kv->value,format,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14);
                    return kv->value;
                }
            }
            if(j==sheet->col[i])
                return NULL;
        }
    return NULL;
}

MList *mINISection(MFile *file)
{
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    MHandle *hdl=mHandle(file,INIRead);
    struct HandleINIRead *handle = (struct HandleINIRead *)(hdl->handle);
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
    struct HandleINIKey *handle = (struct HandleINIKey *)info;
    if(handle->key != NULL) mListRelease(handle->key);
}
#define HASH_INIKey 0x5b939d8c
MList *mINIKey(MFile *file,const char *section)
{
    int i,j;
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    mException(INVALID_POINTER(section),EXIT,"invalid input");
    
    MHandle *hdl=mHandle(file,INIRead);
    struct HandleINIRead *handle0 = (struct HandleINIRead *)(hdl->handle);
    if(hdl->valid == 0)
    {
        INIOpen(file,handle0);
        hdl->valid = 1;
    }
    MList  *list = handle0->list;
    MSheet *sheet= handle0->sheet;
    
    hdl = mHandle(file,INIKey);
    struct HandleINIKey *handle = (struct HandleINIKey *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->key = mListCreate(DFLT,NULL);
        hdl->valid = 1;
    }
    
    struct KeyValue *kv;
    for(i=0;i<list->num;i++)
    {
        if(strcmp(section,(char *)(list->data[i]))==0)
        {
            if(handle->key->num<sheet->col[i])
                mListAppend(handle->key,sheet->col[i]);
            handle->key->num = sheet->col[i];
            
            for(j=0;j<sheet->col[i];j++)
            {
                kv = (struct KeyValue *)(sheet->data[i][j]);
                handle->key->data[j] = kv->name;
            }
            return handle->key;
        }
    }
    return NULL;
}

