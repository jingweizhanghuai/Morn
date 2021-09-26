/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_image.h"
#include "morn_help.h"

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

int main(int argc,char *argv[])
{
    ToolHelp(imageformat);
    
    char *file_in = mStringArgument(argc,argv,"i" ,NULL);
    char *file_out= mStringArgument(argc,argv,"o" ,NULL);
    char *dir_in  = mStringArgument(argc,argv,"di",NULL);
    char *dir_out = mStringArgument(argc,argv,"do",NULL);
    char *type_in = mStringArgument(argc,argv,"ti",NULL);
    char *type_out= mStringArgument(argc,argv,"to",NULL);
    
    mException((file_in !=NULL)&&(dir_in !=NULL),EXIT, "input file name or dir name");
    mException((file_out!=NULL)&&(dir_out!=NULL),EXIT,"output file name or dir name");
    mException((file_in !=NULL)&&(dir_out!=NULL),EXIT, "input file name or dir name");
    mException((file_out!=NULL)&&(dir_in !=NULL),EXIT,"output file name or dir name");
    mException((type_out==NULL)&&(file_out==NULL),EXIT,"invalid output type");
    
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    
    char *type=NULL;
    char filename[256];
    if(file_in!=NULL)
    {
        mImageLoad(src,file_in);
        
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
             if(strcasecmp(type,"jpg" )==0) mImageLoad(src,filename);
        else if(strcasecmp(type,"png" )==0) mImageLoad(src,filename);
        else if(strcasecmp(type,"bmp" )==0) mImageLoad(src,filename);
        else if(strcasecmp(type,"jpeg")==0) mImageLoad(src,filename);
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
