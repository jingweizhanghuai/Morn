/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
char *StringArgument(int argc,char **argv,const char *flag,int n,...)
{
    mException((n>8),EXIT,"invalid para number %d,which must less than 8",n);
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
    if(n<=0) return result;
    
    void *value[8];memset(value+n-1,0,(9-n)*sizeof(void *));
    va_list para;va_start(para,n);
    char *string = (char *)va_arg(para,void *);
    mException((!INVALID_POINTER(string))&&(n==1),EXIT,"invalid input format");
    for(int i=0;i<n-1;i++) {value[i]=(void *)va_arg(para,void *);}
    va_end(para);
    
    sscanf(result,string,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7]);
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
    