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

#ifdef _MSC_VER
#include <windows.h>
void mFileList(MList *list,const char *directory,const char *regular)
{
    mException(INVALID_POINTER(list)||INVALID_POINTER(directory),EXIT,"invalid input");
   
    char name[256];
    if(regular == NULL) sprintf(name,"%s\\*",directory);
    else                sprintf(name,"%s\\%s",directory,regular);
    
    mListClear(list);
   
    WIN32_FIND_DATA p;HANDLE h = FindFirstFile("e:\\test\\*.c",&p);
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
        if(pf == NULL)
            break;
        
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

