/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "morn_util.h"

int mStringRegular(const char *str1,const char *str2)
{
    int i,j;
    char *p;
    
    for(i=0;;i++)
    {
        
        if((str1[i] == 0)&&(str2[i] == 0))
            return 1;
        
        if(str1[i] == str2[i])
            continue;
        
        if((str1[i] == '?')&&(str2[i]!=0))
            continue;
        
        if((str2[i] == '?')&&(str1[i]!=0))
            continue;
        
        if(str1[i] == '*')
        {
            p = (char *)str1+i+1;
            if(*p==0)
                return 1;
            
            for(j=i;str2[j]!=0;j++)
                if(mStringRegular(p,str2+j))
                    return 1;
                
            return 0;
        }
        
        if(str2[i] == '*')
        {
            p = (char *)str2+i+1;
            if(*p==0)
                return 1;
            
            for(j=i;str1[j]!=0;j++)
                if(mStringRegular(p,str1+j))
                    return 1;
                
            return 0;
        }
        
        if(str1[i] != str2[i])
            return 0;
    }
}

// #define MORN_STRING_SPLIT_MODE(N) (((N)>0)?(N):(-1-(N)))
// int mStringSplit(char *in,const char *flag,char **out1,char **out2,int mode)
// {
//     if(in==NULL) return 0;
//     if((mode==DFLT)||(mode==0)) mode = 1;
    
//     *out1=in;*out2=NULL;
//     int l = strlen(flag);

//     if(mode>0)
//     {
//         int n = 0;
//         for(int i=0;in[i+l]!=0;i++)
//         {
//             if(strncmp(in+i,flag,l)==0)
//             {
//                 n++;if(n==mode)
//                 {
//                     in[i]=0;
//                     *out2=in+i+l;return 1;
//                 }
//             }
//         }
//     }
//     else
//     {
//         int n = -1;
//         for(int i=strlen(in)-l;i>=0;i--)
//         {
//             if(strncmp(in+i,flag,l)==0)
//             {
//                 n--;if(n==mode)
//                 {
//                     in[i]=0;
//                     *out2=in+i+l;return 1;
//                 }
//             }
//         }
//     }
//     return 0;
// }

struct HandleStringSplit
{
    MList *list;
};
void endStringSplit(struct HandleStringSplit *handle)
{
    if(handle->list!=NULL) mListRelease(handle->list);
}
#define HASH_StringSplit 0xecdfe96e
MList *mStringSplit(const char *str_in,const char *flag)
{
    mException(INVALID_POINTER(str_in)||INVALID_POINTER(flag),EXIT,"invalid input");
    int src_len = strlen(str_in);
    MHandle *hdl = mHandle(mMornObject((void *)str_in,src_len),StringSplit);
    struct HandleStringSplit *handle =(struct HandleStringSplit *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->list==NULL) handle->list = mListCreate();
        mListClear(handle->list);
        hdl->valid = 1;
    }
    MList *list = handle->list;
    src_len +=1;
    
    mListWrite(list,0,(void *)str_in,src_len);
    char *str = (char *)(list->data[0]);

    int *locate = (int *)mMalloc(src_len*sizeof(int));
    uint64_t flag_len = strlen(flag);
    int num;
    if(strspn(str,flag)<flag_len)
    {
        locate[0] = 0;
        num = 1;
    }
    else
        num = 0;
    
    for(int i=0;i<src_len;i++)
    {
        if(strspn(str+i,flag)>=flag_len)
        {
            if(strspn(str+i+flag_len,flag)<flag_len)
            {
                locate[num] = i+flag_len;
                num = num +1;
            }
            str[i] = 0;
            i=i+flag_len-1;
        }
    }
    
    if(num > list->num) mListAppend(list,num);
    for(int i=0;i<num;i++)
        list->data[i] = str+locate[i];
    list->num = num;
   
    mFree(locate);
    
    return list;
}

// int mStringCompare(const char *str1,const char *str2)
// {
    // int flag=1;
    // for(int i=0;;i++)
    // {
        // if((str1[i]==0) == (str2[i]!=0)) return 1;
        // if((str1[i]==0) || (str2[i]==0)) return 0;
       
        // if(str1[i] == str2[i]) continue;
     
        // if((str1[i]>='a')&&(str1[i]<='z'))
            // if(str2[i] - str1[i] == 'A'-'a')
                // continue;
            
        // if((str2[i]>='a')&&(str2[i]<='z'))
            // if(str1[i] - str2[i] == 'A'-'a')
                // continue;
        
        // return 1;
    // }
// }

void mStringReplace(char *src,char *dst,const char *replace_in,const char *replace_out)
{
    int num_in = strlen(replace_in);
    int num_out= strlen(replace_out);
    
    int n=0;
    int m=0;
    while(src[n]!='\0')
    {
        if(memcmp(src+n,replace_in,num_in)==0)
        {
            memcpy(dst+m,replace_out,num_out);
            n=n+num_in;
            m=m+num_out;
        }
        else
        {
            dst[m] = src[n];
            n=n+1;
            m=m+1;
        }
    }
    dst[m] = '\0';
}

char morn_string_argument[2]={'?',0};
int morn_string_arg_idx = 0;
char *m_StringArgument(int argc,char **argv,const char *flag,const char *format,...)
{
    if(argc<=1) return NULL;
    char *result;
    if(flag==NULL) {result=argv[1]+(argv[1][0]=='-'); goto nextStringArgument;}
    uint64_t flag_len=strlen(flag);

    int idx=morn_string_arg_idx+1;if(idx==argc) idx=1;
    int i,j;i=idx;
    while(1)
    {
        morn_string_arg_idx = i;
        if((argv[i][0] == '-')&&((argv[i][1]<'0')||(argv[i][1]>'9')))
        {
            if(strspn(argv[i]+1,flag)>=flag_len)
            {
                if(strlen(argv[i]) > flag_len+1)
                    {result=argv[i]+1+flag_len+((*(argv[i]+1+flag_len)=='=')||(*(argv[i]+1+flag_len)==':'));break;}
                else if(i+1==argc)
                    return morn_string_argument;
                else if((argv[i+1][0]=='-')&&((argv[i+1][1]<'0')||(argv[i+1][1]>'9')))
                    return morn_string_argument;
            }
        }
        else
        {
            for(j=i;j>=1;j--) if((argv[j][0]=='-')&&((argv[j][1]<'0')||(argv[j][1]>'9'))) break;
            if(j>0) if(strspn(argv[j]+1,flag)>=flag_len) {result=argv[i];break;}
        }
        i++;if(i==argc) {i=1;} if(i==idx) {return NULL;}
    }
 
    nextStringArgument:
    if(!INVALID_POINTER(format))
    {
        va_list argpara;
        va_start(argpara, format);
        vsscanf(result,format,argpara);
        va_end(argpara);
    }
    return result;
}


static int morn_atoi[9][10]={
    {0,100000000,200000000,300000000,400000000,500000000,600000000,700000000,800000000,900000000},
    {0, 10000000, 20000000, 30000000, 40000000, 50000000, 60000000, 70000000, 80000000, 90000000},
    {0,  1000000,  2000000,  3000000,  4000000,  5000000,  6000000,  7000000,  8000000,  9000000},
    {0,   100000,   200000,   300000,   400000,   500000,   600000,   700000,   800000,   900000},
    {0,    10000,    20000,    30000,    40000,    50000,    60000,    70000,    80000,    90000},
    {0,     1000,     2000,     3000,     4000,     5000,     6000,     7000,     8000,     9000},
    {0,      100,      200,      300,      400,      500,      600,      700,      800,      900},
    {0,       10,       20,       30,       40,       50,       60,       70,       80,       90},
    {0,        1,        2,        3,        4,        5,        6,        7,        8,        9}
};

int mAtoi(char *str)
{
    while(*str==' ') str++;
    int flag=0;unsigned char *s=(unsigned char *)str;
         if(*str=='-') {flag=1;s++;}
    else if(*str=='+')         s++;
    int data=morn_atoi[0][s[0]-'0'];if((s[1]<'0')||(s[1]>'9')) {data=data/100000000;return (flag)?(0-data):data;}
    data +=  morn_atoi[1][s[1]-'0'];if((s[2]<'0')||(s[2]>'9')) {data=data/10000000; return (flag)?(0-data):data;}
    data +=  morn_atoi[2][s[2]-'0'];if((s[3]<'0')||(s[3]>'9')) {data=data/1000000;  return (flag)?(0-data):data;}
    data +=  morn_atoi[3][s[3]-'0'];if((s[4]<'0')||(s[4]>'9')) {data=data/100000;   return (flag)?(0-data):data;}
    data +=  morn_atoi[4][s[4]-'0'];if((s[5]<'0')||(s[5]>'9')) {data=data/10000;    return (flag)?(0-data):data;}
    data +=  morn_atoi[5][s[5]-'0'];if((s[6]<'0')||(s[6]>'9')) {data=data/1000;     return (flag)?(0-data):data;}
    data +=  morn_atoi[6][s[6]-'0'];if((s[7]<'0')||(s[7]>'9')) {data=data/100;      return (flag)?(0-data):data;}
    data +=  morn_atoi[7][s[7]-'0'];if((s[8]<'0')||(s[8]>'9')) {data=data/10;       return (flag)?(0-data):data;}
    data +=               s[8]-'0' ;if((s[9]<'0')||(s[9]>'9')) {                    return (flag)?(0-data):data;}
    data=data*10+s[9]-'0';return (flag)?(0-data):data;
}

// int mAtoi(char *str)
// {
//     while(*str==' ') str++;
//     int flag=0;
//          if(*str=='-') {flag=1;str++;}
//     else if(*str=='+')         str++;
//     int n;for(n=0;(str[n]>='0')&&(str[n]<='9');n++);
//     unsigned char *s=(unsigned char *)(str+n-9);
//     int data=s[8]-'0';
//     for(int i=7;i>8-n;i--) data += morn_atoi[i][s[i]-'0'];
//     return (flag)?(0-data):data;
// }

static double morn_atof[17][10]={
    {0.0,0.1000000000000000,0.2000000000000000,0.3000000000000000,0.4000000000000000,0.5000000000000000,0.6000000000000000,0.7000000000000000,0.8000000000000000,0.9000000000000000},
    {0.0,0.0100000000000000,0.0200000000000000,0.0300000000000000,0.0400000000000000,0.0500000000000000,0.0600000000000000,0.0700000000000000,0.0800000000000000,0.0900000000000000},
    {0.0,0.0010000000000000,0.0020000000000000,0.0030000000000000,0.0040000000000000,0.0050000000000000,0.0060000000000000,0.0070000000000000,0.0080000000000000,0.0090000000000000},
    {0.0,0.0001000000000000,0.0002000000000000,0.0003000000000000,0.0004000000000000,0.0005000000000000,0.0006000000000000,0.0007000000000000,0.0008000000000000,0.0009000000000000},
    {0.0,0.0000100000000000,0.0000200000000000,0.0000300000000000,0.0000400000000000,0.0000500000000000,0.0000600000000000,0.0000700000000000,0.0000800000000000,0.0000900000000000},
    {0.0,0.0000010000000000,0.0000020000000000,0.0000030000000000,0.0000040000000000,0.0000050000000000,0.0000060000000000,0.0000070000000000,0.0000080000000000,0.0000090000000000},
    {0.0,0.0000001000000000,0.0000002000000000,0.0000003000000000,0.0000004000000000,0.0000005000000000,0.0000006000000000,0.0000007000000000,0.0000008000000000,0.0000009000000000},
    {0.0,0.0000000100000000,0.0000000200000000,0.0000000300000000,0.0000000400000000,0.0000000500000000,0.0000000600000000,0.0000000700000000,0.0000000800000000,0.0000000900000000},
    {0.0,0.0000000010000000,0.0000000020000000,0.0000000030000000,0.0000000040000000,0.0000000050000000,0.0000000060000000,0.0000000070000000,0.0000000080000000,0.0000000090000000},
    {0.0,0.0000000001000000,0.0000000002000000,0.0000000003000000,0.0000000004000000,0.0000000005000000,0.0000000006000000,0.0000000007000000,0.0000000008000000,0.0000000009000000},
    {0.0,0.0000000000100000,0.0000000000200000,0.0000000000300000,0.0000000000400000,0.0000000000500000,0.0000000000600000,0.0000000000700000,0.0000000000800000,0.0000000000900000},
    {0.0,0.0000000000010000,0.0000000000020000,0.0000000000030000,0.0000000000040000,0.0000000000050000,0.0000000000060000,0.0000000000070000,0.0000000000080000,0.0000000000090000},
    {0.0,0.0000000000001000,0.0000000000002000,0.0000000000003000,0.0000000000004000,0.0000000000005000,0.0000000000006000,0.0000000000007000,0.0000000000008000,0.0000000000009000},
    {0.0,0.0000000000000100,0.0000000000000200,0.0000000000000300,0.0000000000000400,0.0000000000000500,0.0000000000000600,0.0000000000000700,0.0000000000000800,0.0000000000000900},
    {0.0,0.0000000000000010,0.0000000000000020,0.0000000000000030,0.0000000000000040,0.0000000000000050,0.0000000000000060,0.0000000000000070,0.0000000000000080,0.0000000000000090},
    {0.0,0.0000000000000001,0.0000000000000002,0.0000000000000003,0.0000000000000004,0.0000000000000005,0.0000000000000006,0.0000000000000007,0.0000000000000008,0.0000000000000009},
    {0.0,0.0000000000000000,0.0000000000000000,0.0000000000000000,0.0000000000000000,0.0000000000000000,0.0000000000000000,0.0000000000000000,0.0000000000000000,0.0000000000000000},
};

static double morn_atof_k[20]={1.0,0.1,0.01,0.001,0.0001,0.00001,0.000001,0.0000001,0.00000001,0.000000001,0.0000000001,0.00000000001,0.000000000001,0.0000000000001,0.00000000000001,0.000000000000001,0.0000000000000001,0.00000000000000001,0.000000000000000001,0.0000000000000000001};
static double morn_atof_e[20]={1.0,10.0,100.0,1000.0,10000.0,100000.0,1000000.0,10000000.0,100000000.0,1000000000.0,10000000000.0,100000000000.0,1000000000000.0,10000000000000.0,100000000000000.0,1000000000000000.0,10000000000000000.0,100000000000000000.0,1000000000000000000.0,10000000000000000000.0};
double mAtof(char *p)
{
    while(*p==' ') p++;
    int flag=0;double d=0;
         if(*p=='-') {flag=1;p++;}
    else if(*p=='+')         p++;
    for(;(*p>='0')&&(*p<='9');p++) d=d*10.0+(*p-'0');
    if(*p=='.')
    {
        p++;int i;double v=0;
        for(i=0;i<20;i++) {if(p[i]!='0') break;} double k=morn_atof_k[i]; p=p+i;
        for(i=0;(p[i]>='0')&&(p[i]<='9');i++) {v=v+morn_atof[MIN(i,16)][p[i]-'0'];} p=p+i;
        d=v*k+d;
    }
    if((*p=='e')||(*p=='E'))
    {
        p++;int e=mAtoi(p);
        if(e>=0) {for(;e>= 20;e-=20) {d=d*100000000000000000000.0;} d=d*morn_atof_e[  e];}
        else     {for(;e<=-20;e+=20) {d=d*0.000000000000000000100;} d=d*morn_atof_k[0-e];}
    }
    return (flag)?(0-d):d;
}

#define fgets(Buff,N,F) mException((fgets(Buff,N,F)==NULL),EXIT,"file read error")
void mHelp(const char *helpfile,const char *name)
{
    FILE *f;
    char buff[2048];
    uint64_t len;
    
    len = strlen(name);
    
    f = fopen(helpfile,"r");
    
    while(!feof(f))
    {
        fgets(buff,2048,f);
        if(buff[0] == '[')
        {
            if((strspn(buff+1,name)>=len)&&(buff[len+1] == ']'))
                break;
        }
    }
    
    fgets(buff,2048,f);
    while((buff[0] != '[')&&(!feof(f)))
    {
        printf("%s",buff);
        fgets(buff,2048,f);
    }
    
    fclose(f);
}
    