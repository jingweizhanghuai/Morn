/*
Copyright (C) 2019-2021 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"
#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

struct KeyValue
{
    char *name;
    char *value;
};

struct Section
{
    char name[64];
    struct KeyValue **kv;
    int num;
};

struct HandleINI
{
    char *data;
    MSheet *sheet;
};
#define HASH_INI 0x877ae557
void endINI(struct HandleINI *handle)
{
    if(handle->data!=NULL) mFree(handle->data);
    if(handle->sheet!=NULL) mSheetRelease(handle->sheet);
}

void mINILoad(MList *list,char *filename)
{
    mListClear(list);
    MHandle *hdl = mHandle(list,INI);
    struct HandleINI *handle = (struct HandleINI *)(hdl->handle);
    endINI(handle);
    
    FILE *f = fopen(filename,"rb");
    mException((f == NULL),EXIT,"file cannot open");
    int filesize=fsize(f);
    handle->data = mMalloc(filesize+2);
    fread(handle->data,1,filesize,f);
    fclose(f);
    handle->data[filesize]='\n';
    handle->data[filesize+1]= 0;
    
    handle->sheet = mSheetCreate();
    MSheet *sheet = handle->sheet;
    hdl->valid = 1;

    int i;
    struct KeyValue kv;
    
    int flag = 0;
    char *p;
    for(p=handle->data;p[0]!='[';p++);
    for(;p<handle->data+filesize+1;p++)
    {
             if((p[0]== ' ')||(p[0]=='\t')) continue;
        else if((p[0]=='\n')||(p[0]=='\r'))
        {
            mException(flag==3,EXIT,"invalid ini");
            p[0] = 0;
        }
        else if((p[0]== ';')||(p[0]== '#'))
        {
            mException(flag==3,EXIT,"invalid ini");
            p[0] = 0;
            for(p=p+1;(p[0]!='\n')&&(p[0]!='\r');p++);
        }
        else if(p[0]=='=')
        {
            mException((flag!=1),EXIT,"invalid ini");flag = 2;
            for(i=-1;(p[i]==' ')||(p[i]=='\t')||(p[i]=='\n')||(p[i]=='\r');i--);
            p[i+1]=0;
        }
        else if(p[0]=='[') 
        {
            mException((flag!=0),EXIT,"invalid ini");flag = 3;
        }
        else if(p[0]==']')
        {
            mException((flag!=4),EXIT,"invalid ini");flag = 0;
            for(i=-1;(p[i]==' ')||(p[i]=='\t');i--);
            p[i+1]=0;
        }
        else if(flag==3)
        {
            mListAppend(list,DFLT);list->data[list->num-1] = p;
            mSheetRowAppend(sheet,DFLT);
            flag=4;
        }
        else if(flag==0)
        {
            kv.name = p;flag = 1;
        }
        else if(flag==2)
        {
            int j=0;
            for(i=0;(p[i]!='#')&&(p[i]!=';')&&(p[i]!='\n')&&(p[i]!='\r');i++)
                if((p[i]!=' ')&&(p[i]!='\t'))p[j++]=p[i];
            if(j<i) p[j]=0;
            kv.value=p;mSheetWrite(sheet,list->num-1,DFLT,&kv,sizeof(struct KeyValue));
            flag=0;
            p=p+i-1;
        }
    }
    struct Section section;
    for(i=0;i<list->num;i++)
    {
        char *p=(char *)(list->data[i]);strcpy(section.name,p);
        section.kv=(struct KeyValue **)(sheet->data[i]);
        section.num = sheet->col[i];
        mListWrite(list,i,&section,sizeof(struct Section));
    }
}

char *m_INIRead(MList *ini,const char *section,const char *key,const char *format,...)
{
    if(ini==NULL) return NULL;
    mException(INVALID_POINTER(section)||INVALID_POINTER(key),EXIT,"invalid input");
    
    for(int i=0;i<ini->num;i++)
    {
        struct Section *s = (struct Section *)(ini->data[i]);
        if(strcmp(section,s->name)==0)
        {
            for(int j=0;j<s->num;j++)
            {
                if(strcmp(key,s->kv[j]->name)==0)
                {
                    if(!INVALID_POINTER(format))
                    {
                        va_list inipara;
                        va_start(inipara,format);
                        vsscanf(s->kv[j]->value,format,inipara);
                        va_end(inipara);
                    }
                    return s->kv[j]->value;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

char *m_INIWrite(MList *ini,const char *section,const char *key,const char *format,...)
{
    mException(INVALID_POINTER(ini),EXIT,"invalid input");
    mException(INVALID_POINTER(section)||INVALID_POINTER(key)||INVALID_POINTER(format),EXIT,"invalid input");

    char data[1024];
    va_list inipara;
    va_start(inipara,format);
    vsprintf(data,format,inipara);
    va_end(inipara);
    int key_size=strlen(key)+1;
    int data_size=strlen(data)+1;

    MHandle *hdl=mHandle(ini,INI);
    struct HandleINI *handle = (struct HandleINI *)(hdl->handle);
    if(hdl->valid==0)
    {
        mException(ini->num!=0,EXIT,"invalid operate");
        if(handle->sheet==NULL) handle->sheet = mSheetCreate();
        hdl->valid = 1;
    }

    int i=0,j=0;
    struct Section *s=NULL;
    for(i=0;i<ini->num;i++)
    {
        s = (struct Section *)(ini->data[i]);
        if(strcmp(section,s->name)==0)
        {
            for(j=0;j<s->num;j++)
            {
                if(strcmp(key,s->kv[j]->name)==0) break;
            }
            break;
        }
    }
    if(i==ini->num)
    {
        s = mListWrite(ini,DFLT,NULL,sizeof(struct Section));
        strcpy(s->name,section);
        j=0;
    }
    
    MSheet *sheet = handle->sheet;
    struct KeyValue *p=mSheetWrite(sheet,i,j,NULL,sizeof(struct KeyValue)+key_size+data_size);
    p->name =(char *)(p+1);   memcpy(p->name , key, key_size);
    p->value=p->name+key_size;memcpy(p->value,data,data_size);
    s->kv=(struct KeyValue **)sheet->data[i];
    s->num=sheet->col[i];
    return p->value;
}

void m_INIDelete(MList *ini,const char *section,const char *key)
{
    mException(INVALID_POINTER(ini),EXIT,"invalid input");
    mException(INVALID_POINTER(section),EXIT,"invalid input");

    MHandle *hdl=mHandle(ini,INI);
    struct HandleINI *handle = (struct HandleINI *)(hdl->handle);
    mException(hdl->valid==0,EXIT,"invalid input");

    int i,j;
    struct Section *s;
    if(INVALID_POINTER(key))
    {
        for(i=0;i<ini->num;i++)
        {
            s = (struct Section *)(ini->data[i]);
            if(strcmp(section,s->name)==0) break;
        }
        if(i==ini->num) return;
        mListElementDelete(ini,i);
        mSheetDelete(handle->sheet,i);
    }
    else
    {
        for(i=0;i<ini->num;i++)
        {
            s = (struct Section *)(ini->data[i]);
            if(strcmp(section,s->name)==0)
            {
                for(j=0;j<s->num;j++)
                {
                    if(strcmp(key,s->kv[j]->name)==0)
                    {
                        mSheetDelete(handle->sheet,i,j);
                        s->num=s->num-1;
                        break;
                    }
                }
                return;
            }
        }
    }
}

void INIFileWrite(FILE *f,char *buff,int *size,char *format,...)
{
    unsigned int n;
    va_list args; 
    va_start(args,format);n=vsnprintf(buff-(*size),(*size),format,args);va_end(args);
    if(n>*size)
    {
        fwrite(buff-8192,1,8192-(*size),f);
        *size = 8192;
        va_start(args,format);n=vsnprintf(buff-8192,8192,format,args);va_end(args);
    }
    *size-=n;
}

void mINISave(MList *ini,char *filename)
{
    FILE *f=fopen(filename,"w");
    char *buff = mMalloc(8192);buff=buff+8192;
    int size = 8192;
    
    for(int i=0;i<ini->num;i++)
    {
        struct Section *s=ini->data[i];
        INIFileWrite(f,buff,&size,"[%s]\n",s->name);
        for(int j=0;j<s->num;j++)
            INIFileWrite(f,buff,&size,"%s=%s\n",s->kv[j]->name,s->kv[j]->value);
    }
    fwrite(buff-8192,1,8192-size,f);
    fclose(f);
    mFree(buff-8192);
}

MList *morn_INI_file=NULL;
struct HandleINIFile
{
    MList *ini;
};
void endINIFile(struct HandleINIFile *handle)
{
    if(handle->ini!=NULL) mListRelease(handle->ini);
    morn_INI_file = NULL;
}
#define HASH_INIFile 0xae889989
void mINIFile(char *ininame)
{
    MHandle *hdl = mHandle(mMornObject("Morn",DFLT),INIFile);
    struct HandleINIFile *handle = (struct HandleINIFile *)(hdl->handle);
    hdl->valid = 1;
    handle->ini = mListCreate();
    mINILoad(handle->ini,ininame);
    morn_INI_file = handle->ini;
}
MList *mINI()
{
    return morn_INI_file;
}






/*
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

char *m_INIRead(MObject *file,const char *section,const char *key,const char *format,...)
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
                    if(!INVALID_POINTER(format))
                    {
                        va_list inipara;
                        va_start(inipara,format);
                        vsscanf(kv->value,format,inipara);
                        va_end(inipara);
                    }
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
*/
