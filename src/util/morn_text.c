/*
Copyright (C) 2019-2024 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

// #define HASH_Text 0x93a49bd4
char *m_Text(MString *text,const char *file_name,...)
{
    char filename[256];
    va_list namepara;
    va_start (namepara,file_name);
    vsnprintf(filename,256,file_name,namepara);
    va_end(namepara);
    FILE *f = fopen(filename,"rb");
    mException(f==NULL,EXIT,"can not find file %s",filename);
    int size = fsize(f);
    mArrayRedefine(text,size+1,1,NULL);
    // mObjectType(text)=HASH_Text;
    fread(text->string,size,1,f);
    text->string[size]=0;
    fclose(f);
    return text->string;
}

struct HandleStringSearch
{
    char str[1024];
    int str_size;
    int list[256];
    int step;
    int pos;
};
void endStringSearch(struct HandleStringSearch *handle) {NULL;}
#define HASH_StringSearch 0x7a53de7a
char *m_StringSearch(MArray *text,char *str,int pos)
{
    int i,j;int str_size;
    uint8_t *pstr=(uint8_t *)str;
    uint8_t *ptext=text->dataU8;

    MHandle *hdl = mHandle(text,StringSearch);
    struct HandleStringSearch *handle = hdl->handle;
    if((hdl->valid==0)||(strcmp(handle->str,str)!=0))
    {
        handle->str_size=strlen(str);str_size=handle->str_size-1;
        memcpy(handle->str,pstr,str_size+2);
        handle->pos=-1;
        hdl->valid = 1;
        if((text->num<256)||(str_size<8))
        {
            char *p; if(pos<0) pos=0;
            if(str_size)p=strstr(text->string+pos,str);
            else        p=strchr(text->string+pos,str[0]);
            if(p!=NULL) {handle->pos=p-text->string;} else {handle->pos=text->num;}
            return p;
        }

        uint8_t c = pstr[str_size];
        for(i=0;i<256;i++) handle->list[i]=str_size+1;
        handle->step=str_size+1;
        for(i=0;i<str_size;i++)
        {
            handle->list[pstr[i]]=str_size-i;
            if(pstr[i]==c) handle->step=str_size-i;
        }
        handle->list[c]=0;

    }
    int *list = handle->list;
    str_size=handle->str_size-1;
    if(pos<0) pos=handle->pos+1;
    if((text->num<256)||(str_size<8))
    {
        char *p;
        if(str_size)p=strstr(text->string+pos,str);
        else        p=strchr(text->string+pos,str[0]);
        if(p!=NULL) {handle->pos=p-text->string;} else {handle->pos=text->num;}
        return p;
    }
    
    i=pos+str_size;
    while(i<text->num)
    {
        int s=list[ptext[i]];
        if(s) {i+=s;continue;}

        for(j=1;j<=str_size;j++)
        {
            if(ptext[i-j]!=pstr[str_size-j]) {i+=MAX(handle->step,list[ptext[i-j]]-j);break;}
            if(j==str_size) {handle->pos=i-str_size; return ((char *)ptext+handle->pos);}
        }
    }
    handle->pos=text->num;
    return NULL;
}
