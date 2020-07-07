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

char **mStringSplit(const char *str_in,const char *flag,MList *list)
{
    mException(INVALID_POINTER(str_in)||INVALID_POINTER(list)||INVALID_POINTER(flag),EXIT,"invalid input");
    
    int src_len = strlen(str_in)+1;
    
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
    
    return (char **)(list->data);
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
char *StringArgument(int argc,char **argv,const char *flag,char *format,void *p1,void *p2,void *p3,void *p4,void *p5,void *p6)
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
        if(argv[i][0] == '-') 
        {
            if(strspn(argv[i]+1,flag)>=flag_len)
            {
                if(strlen(argv[i]) > flag_len+1)
                    {result=argv[i]+1+flag_len+((*(argv[i]+1+flag_len)=='=')||(*(argv[i]+1+flag_len)==':'));break;}
                else if(i+1==argc)
                    return morn_string_argument;
                else if(argv[i+1][0]=='-')
                    return morn_string_argument;
            }
        }
        else
        {
            for(j=i;j>=1;j--) if(argv[j][0]=='-') break;
            if(j>0) if(strspn(argv[j]+1,flag)>=flag_len) {result=argv[i];break;}
        }
        i++;if(i==argc) {i=1;} if(i==idx) {return NULL;}
    }

    nextStringArgument:
    if(!INVALID_POINTER(format))
        sscanf(result,format,p1,p2,p3,p4,p5,p6);
    return result;
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
    