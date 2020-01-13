/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
char *mStringArgument(int argc,char **argv,const char *flag,int *ok)
{
    uint64_t flag_len=0,argv_len=0;
    
    if(!INVALID_POINTER(flag))
        flag_len = strlen(flag);
    
    if(!INVALID_POINTER(ok))
        *ok = 1;
    
    for(int i=1;i<argc;i++)
    {
        if(argv[i][0] == '-')
        {
            argv_len = strlen(argv[i]+1);
            if(flag != NULL)
            {
                if(strspn(argv[i]+1,flag)>=flag_len)
                {
                    if(argv_len > flag_len)
                        return (argv[i]+1+flag_len);
                    else if(i == argc-1)
                        return NULL;
                    else if(argv[i+1][0] != '-')
                        return argv[i+1];
                    else
                        return NULL;
                }
            }
            i=i+(argv_len<=1);
        }
        else
        {
            if(flag == NULL)
                return argv[i];
        }
    }
    if(!INVALID_POINTER(ok))
        *ok = 0;
    return NULL;
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
    
        
























