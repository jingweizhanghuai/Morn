/*
Copyright (C) 2019-2021 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"
#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

static MSheet *morn_INI_sheet=NULL;
const char  *morn_INI_file=NULL;
struct HandleINIFile
{
    MSheet *sheet;
};
void endINIFile(struct HandleINIFile *handle)
{
    if(handle->sheet!=NULL) mSheetRelease(handle->sheet);
}
#define HASH_INIFile 0xae889989
void mINIFile(const char *filename) 
{
    MHandle *hdl = mHandle("INI",INIFile);
    struct HandleINIFile *handle= (struct HandleINIFile *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->sheet==NULL) handle->sheet = mSheetCreate();
        hdl->valid = 1;
    }
    morn_INI_file=filename;
    morn_INI_sheet = handle->sheet;
    if(morn_INI_sheet!=NULL) mSheetClear(morn_INI_sheet);
    mINILoad(morn_INI_sheet,filename);
}
MSheet *mINI() 
{
    mException((morn_INI_file==NULL)||(morn_INI_sheet==NULL),EXIT,"invalid input");
    return morn_INI_sheet;
}

struct HandleINI
{
    char *data;
};
#define HASH_INI 0x877ae557
void endINI(struct HandleINI *handle)
{
    if(handle->data !=NULL) mFree(handle->data);
}
void mINILoad(MSheet *sheet,const char *ininame,...)
{
    char filename[256];
    va_list val;va_start(val,ininame);vsprintf(filename,ininame,val);va_end(val);

    MHandle *hdl = mHandle(sheet,INI);
    struct HandleINI *handle = (struct HandleINI *)(hdl->handle);
    endINI(handle);
    
    FILE *f = fopen(filename,"rb");
    mException((f == NULL),EXIT,"file %s cannot open",filename);
    int filesize=fsize(f);
    handle->data = mMalloc(filesize+2);
    fread(handle->data,1,filesize,f);
    fclose(f);
    handle->data[filesize]='\n';
    handle->data[filesize+1]= 0;
    
    hdl->valid = 1;

    int i;
    int row=-1,col=0;
    
    
    int flag=0;
    char *p;
    for(p=handle->data;p[0]!='[';p++);
    for(;p<handle->data+filesize+1;p++)
    {
             if((p[0]== ' ')||(p[0]=='\t')) p[0]=0;
        else if((p[0]=='\n')||(p[0]=='\r')){mException(flag==3,EXIT,"invalid ini");p[0] = 0;}
        else if((p[0]== ';')||(p[0]== '#')){mException(flag==3,EXIT,"invalid ini");p[0] = 0;for(p=p+1;(p[0]!='\n')&&(p[0]!='\r');p++)p[0]=0;}
        else if( p[0]=='=' )               {mException(flag!=1,EXIT,"invalid ini");p[0]=0;flag=2;}
        else if( p[0]=='[' )               {mException(flag!=0,EXIT,"invalid ini");       flag=3;}
        else if( p[0]==']' )               {mException(flag!=4,EXIT,"invalid ini");p[0]=0;flag=0;}
        else if(flag==3)
        {
            row+=1;col=-1;
            mSheetRowAppend(sheet,row+1);sheet->info[row]=p;
            flag=4;
        }
        else if(flag==0)
        {
            col+=1;mSheetColAppend(sheet,row,col+1);sheet->data[row][col]=p;
            flag = 1;
        }
        else if(flag==2)
        {
            int j=0;
            for(i=0;(p[i]!='#')&&(p[i]!=';')&&(p[i]!='\n')&&(p[i]!='\r');i++)
                if((p[i]!=' ')&&(p[i]!='\t'))p[j++]=p[i];
            p[j]=0;
            flag=0;
            p=p+i;
        }
    }
}

char *m_INIRead(MSheet *ini,const char *section,const char *key,const char *format,...)
{
    if(ini==NULL) return NULL;
    mException(INVALID_POINTER(section)||INVALID_POINTER(key),EXIT,"invalid input");
    
    for(int i=0;i<ini->row;i++)
    {
        if(strcmp(section,(char *)(ini->info[i]))==0)
        {
            for(int j=0;j<ini->col[i];j++)
            {
                if(strcmp(key,ini->data[i][j])==0)
                {
                    char *value = (char *)(ini->data[i][j])+strlen(ini->data[i][j]);for(;*value==0;value++);
                    if(!INVALID_POINTER(format))
                    {
                        va_list inipara;
                        va_start(inipara,format);
                        vsscanf(value,format,inipara);
                        va_end(inipara);
                    }
                    return value;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

char *m_INIWrite(MSheet *ini,const char *section,const char *key,const char *format,...)
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

    int i=0,j=0;
    for(i=0;i<ini->row;i++)
    {
        if(strcmp(section,ini->info[i])==0)
        {
            for(j=0;j<ini->col[i];j++)
            {
                if(strcmp(key,ini->data[i][j])==0) break;
            }
            break;
        }
    }
    if(i==ini->row)
    {
        mSheetInfoWrite(ini,i,(void *)section,DFLT);
        j=0;
    }
    
    char *p=mSheetWrite(ini,i,j,NULL,key_size+data_size);
    memcpy(p,key,key_size);memcpy(p+key_size,data,data_size);
    return p+key_size;
}

char *mINIValue(void *data) 
{
    char *key = (char *)data;
    char *value = key+strlen(key);for(;*value==0;value++);
    return value;
}

void m_INIDelete(MSheet *ini,const char *section,const char *key)
{
    mException(INVALID_POINTER(ini),EXIT,"invalid input");
    mException(INVALID_POINTER(section),EXIT,"invalid input");

    int i,j;
    if(INVALID_POINTER(key))
    {
        for(i=0;i<ini->row;i++) {if(strcmp(section,ini->info[i])==0) {mSheetDelete(ini,i);break;}}
        return;
    }
    
    for(i=0;i<ini->row;i++)
    {
        if(strcmp(section,ini->info[i])==0)
        {
            for(j=0;j<ini->col[i];j++)
            {
                if(strcmp(key,ini->data[i][j])==0)
                {
                    mSheetDelete(ini,i,j);
                    break;
                }
            }
            return;
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

void mINISave(MSheet *ini,char *filename)
{
    FILE *f=fopen(filename,"w");
    char *buff = mMalloc(8192);buff=buff+8192;
    int size = 8192;
    
    for(int i=0;i<ini->row;i++)
    {
        INIFileWrite(f,buff,&size,"[%s]\n",ini->info[i]);
        for(int j=0;j<ini->col[i];j++)
        {
            char *key = ini->data[i][j];
            INIFileWrite(f,buff,&size,"%s=%s\n",key,mINIValue(key));
        }
    }
    fwrite(buff-8192,1,8192-size,f);
    fclose(f);
    mFree(buff-8192);
}

