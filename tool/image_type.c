/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_image.h"
#include "morn_help.h"

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

int main(int argc,char *argv[])
{
    HELP_INFOMATION("ImageType");
    
    char *file_in = mStringArgument(argc,argv,"i" ,NULL);
    char *file_out= mStringArgument(argc,argv,"o" ,NULL);
    char *dir_in  = mStringArgument(argc,argv,"di",NULL);
    char *dir_out = mStringArgument(argc,argv,"do",NULL);
    char *type_in = mStringArgument(argc,argv,"ti",NULL);
    char *type_out= mStringArgument(argc,argv,"to",NULL);
    
    mException((file_in !=NULL)&&(dir_in !=NULL),EXIT, "input file name or dir name");
    mException((file_out!=NULL)&&(dir_out!=NULL),EXIT,"output file name or dir name");
    mException((file_in !=NULL)&&(dir_out!=NULL),EXIT," input file name or dir name");
    mException((file_out!=NULL)&&(dir_in !=NULL),EXIT,"output file name or dir name");
    mException((type_out==NULL)&&(file_out==NULL),EXIT,"invalid output type");
    
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    
    char *type=NULL;
    char filename[256];
    if(file_in!=NULL)
    {
        mImageLoad(file_in,src);
        
        if(file_out==NULL) 
        {
            sprintf(filename,"%s",file_in);
            for(int i=strlen(filename);i>0;i--)if(filename[i]=='.') {type=filename+i+1;break;}
            
            strcpy(type,type_out);
            file_out=filename;
        }
        mImageSave(src,file_out);
        
        mImageRelease(src);
        return 1;
    }
   
    mException(dir_in==NULL,EXIT,"no input");
    
    MList *list=NULL;
    list = mListCreate(DFLT,NULL);
    char regular[8];sprintf(regular,"*.%s",type_in);
    mFileList(list,dir_in,regular);
    
    if(dir_out == NULL) dir_out = dir_in;
    
    for(int j=0;j<list->num;j++)
    {
        sprintf(filename,"%s/%s",dir_in,(char *)(list->data[j]));
        
        for(int i=strlen(filename);i>0;i--)if(filename[i]=='.') {type=filename+i+1;break;}
             if(strcasecmp(type,"jpg" )==0) mImageLoad(filename,src);
        else if(strcasecmp(type,"png" )==0) mImageLoad(filename,src);
        else if(strcasecmp(type,"bmp" )==0) mImageLoad(filename,src);
        else if(strcasecmp(type,"jpeg")==0) mImageLoad(filename,src);
        else continue;
        
        printf("%s\n",filename);
        
        if(dir_out!=NULL) 
        {
            sprintf(filename,"%s/%s",dir_out,(char *)(list->data[j]));
            for(int i=strlen(filename);i>0;i--)if(filename[i]=='.') {type=filename+i+1;break;}
            strcpy(type,type_out);
        }
        
        mImageSave(src,filename);
    }
    
    mImageRelease(src);
    mListRelease(list);
    return 1;
}
    
