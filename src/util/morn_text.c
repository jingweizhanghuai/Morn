/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

#define HASH_Text 0x93a49bd4
void mFileText(MArray *text,const char *file_name,...)
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
    mObjectType(text)=HASH_Text;
    fread(text->text,size,1,f);
    text->text[size]=0;
    fclose(f);
}

int mTextFind0(MArray *text,MArray *str,int pos)
{
    char c=str->text[0];
    char *ptext=text->text;
    char *pstr = str->text+1;
    int size = str->num-1;
    for(int i=pos;i<=text->num-size;i+=16)
    {
        if(ptext[i   ]==c) {if(memcmp(ptext+i+ 1,pstr,size)==0) return i   ;}
        if(ptext[i+ 1]==c) {if(memcmp(ptext+i+ 2,pstr,size)==0) return i+ 1;}
        if(ptext[i+ 2]==c) {if(memcmp(ptext+i+ 3,pstr,size)==0) return i+ 2;}
        if(ptext[i+ 3]==c) {if(memcmp(ptext+i+ 4,pstr,size)==0) return i+ 3;}
        if(ptext[i+ 4]==c) {if(memcmp(ptext+i+ 5,pstr,size)==0) return i+ 4;}
        if(ptext[i+ 5]==c) {if(memcmp(ptext+i+ 6,pstr,size)==0) return i+ 5;}
        if(ptext[i+ 6]==c) {if(memcmp(ptext+i+ 7,pstr,size)==0) return i+ 6;}
        if(ptext[i+ 7]==c) {if(memcmp(ptext+i+ 8,pstr,size)==0) return i+ 7;}
        if(ptext[i+ 8]==c) {if(memcmp(ptext+i+ 9,pstr,size)==0) return i+ 8;}
        if(ptext[i+ 9]==c) {if(memcmp(ptext+i+10,pstr,size)==0) return i+ 9;}
        if(ptext[i+10]==c) {if(memcmp(ptext+i+11,pstr,size)==0) return i+10;}
        if(ptext[i+11]==c) {if(memcmp(ptext+i+12,pstr,size)==0) return i+11;}
        if(ptext[i+12]==c) {if(memcmp(ptext+i+13,pstr,size)==0) return i+12;}
        if(ptext[i+13]==c) {if(memcmp(ptext+i+14,pstr,size)==0) return i+13;}
        if(ptext[i+14]==c) {if(memcmp(ptext+i+15,pstr,size)==0) return i+14;}
        if(ptext[i+15]==c) {if(memcmp(ptext+i+16,pstr,size)==0) return i+15;}
    }
    return DFLT;
}

struct HandleTextFind
{
    int list[256];
    int step;
};
void endTextFind(struct HandleTextFind *handle) {NULL;}
#define HASH_TextFind 0xa0067c8d
int mTextFind(MArray *text,MArray *str,int pos)
{
    if((text->num<256)||(str->num<4)) {return mTextFind0(text,str,pos);}
    uint8_t *pstr = str->dataU8;
    uint8_t *ptext=text->dataU8;
    int str_size = str->num-1;
    
    int i,j;
    MHandle *hdl = mHandle(str,TextFind);
    struct HandleTextFind *handle = hdl->handle;
    if(hdl->valid==0)
    {
        uint8_t c = pstr[str_size];
        for(i=0;i<256;i++) handle->list[i]=str->num;
        handle->step=str->num;
        for(i=0;i<str_size;i++)
        {
            handle->list[pstr[i]]=str_size-i;
            if(pstr[i]==c) handle->step=str_size-i;
        }
        handle->list[c]=0;
        hdl->valid = 1;
    }
    int *list = handle->list;
    
    i=pos+str_size;
    while(i<text->num)
    {
        int s=list[ptext[i]];
        if(s) {i+=s;continue;}

        for(j=1;j<=str_size;j++)
        {
            if(ptext[i-j]!=pstr[str_size-j]) {i+=MAX(handle->step,list[ptext[i-j]]-j);break;}
            if(j==str_size) return (i-str_size);
        }
    }
    return DFLT;
}
