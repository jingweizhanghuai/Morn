/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_util.h"

#ifdef _MSC_VER
#include <windows.h>
void mFileList(MList *list,const char *directory,const char *regular)
{
    mException(INVALID_POINTER(list)||INVALID_POINTER(directory),EXIT,"invalid input");
   
    char name[256];
    if(regular == NULL) sprintf(name,"%s\\*",directory);
    else                sprintf(name,"%s\\%s",directory,regular);
    
    mListClear(list);
   
    WIN32_FIND_DATA p;HANDLE h = FindFirstFile(name,&p);
    int len = strlen(p.cFileName);
    mListWrite(list,0,p.cFileName,len+1);
    ((char *)(list->data[0]))[len] = 0;
    
    for(int n=1;;n++)
    {
        if(FindNextFile(h,&p)==0) break;
    
        len = strlen(p.cFileName);
        mListWrite(list,n,p.cFileName,len+1);
        ((char *)(list->data[n]))[len] = 0;
    }
}

#elif defined __GNUC__

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
void mFileList(MList *list,const char *directory,const char *regular)
{
    mException(INVALID_POINTER(list)||INVALID_POINTER(directory),EXIT,"invalid input");
    
    if(regular!=NULL)
    {
        if((regular[0]=='*')&&(regular[1] == 0))
            regular = NULL;
    }
    
    DIR *dir = opendir(directory);
    
    int flag = 0;
    int n = 0;
    while(1)
    {
        struct dirent *pf = readdir(dir);
        if(pf == NULL) break;
        
        if(flag<2)
        {
            if((strcmp(pf->d_name,".")==0)||(strcmp(pf->d_name,"..")==0))
            {
                flag = flag+1;
                continue;
            }
        }
        // printf("%d: %s\n",n,pf->d_name);
        
        if(!INVALID_POINTER(regular))
            if(!mStringRegular(regular,pf->d_name))
                continue;
        
        int len = strlen(pf->d_name);
        mListWrite(list,n,pf->d_name,len+1);
        ((char *)(list->data[n]))[len] = 0;
        n=n+1;
    }
    
    list->num = n;
    closedir(dir);
}
#endif

