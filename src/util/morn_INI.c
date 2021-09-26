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
    char *name;
    struct KeyValue **kv;
    int num;
};

MList *morn_INI_list=NULL;
MFile *morn_INI_file=NULL;
void mINIFile(MFile *file) {morn_INI_file=file;}
MList *mINI() 
{
    mException((morn_INI_file==NULL)||(morn_INI_list==NULL),EXIT,"invalid input");
    return morn_INI_list;
}

struct HandleINI
{
    char *data;
    MList *list;
    MSheet *sheet;
};
#define HASH_INI 0x877ae557
void endINI(struct HandleINI *handle)
{
    if(handle->data !=NULL) mFree(handle->data);
    if(handle->list !=NULL) mListRelease(handle->list);
    if(handle->sheet!=NULL) mSheetRelease(handle->sheet);
}
MList *m_INILoad(MFile *file)
{
    MHandle *hdl;
    if(file!=NULL) hdl = mHandle(file,INI);
    else
    {
        file=morn_INI_file;mException(file==NULL,EXIT,"invalid input");
        hdl = mHandle(mMornObject("Morn",DFLT),INI);
    }
    
    struct HandleINI *handle = (struct HandleINI *)(hdl->handle);
    endINI(handle);
    
    FILE *f = fopen(file->filename,"rb");
    mException((f == NULL),EXIT,"file cannot open");
    int filesize=fsize(f);
    handle->data = mMalloc(filesize+2);
    fread(handle->data,1,filesize,f);
    fclose(f);
    handle->data[filesize]='\n';
    handle->data[filesize+1]= 0;
    
    handle->list  = mListCreate(); MList  *list  = handle->list;
    handle->sheet = mSheetCreate();MSheet *sheet = handle->sheet;
    
    hdl->valid = 1;

    int i;
    struct Section section;
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
            section.name = p;mListWrite(list,DFLT,&section,sizeof(struct Section));
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
            kv.value=p;mSheetWrite(sheet,sheet->row-1,DFLT,&kv,sizeof(struct KeyValue));
            flag=0;
            p=p+i-1;
        }
    }
    
    for(i=0;i<list->num;i++)
    {
        struct Section *p=(struct Section *)(list->data[i]);
        p->kv=(struct KeyValue **)(sheet->data[i]);
        p->num = sheet->col[i];
    }
    if(file==morn_INI_file) morn_INI_list=list;
    return list;
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






