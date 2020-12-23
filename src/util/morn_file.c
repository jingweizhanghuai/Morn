/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_math.h"

#define  fread(Data,Size,Num,Fl) mException(((int) fread(Data,Size,Num,Fl)!=Num),EXIT, "read file error");
#define fwrite(Data,Size,Num,Fl) mException(((int)fwrite(Data,Size,Num,Fl)!=Num),EXIT,"write file error");

struct Chunk
{
    uint32_t ID;
    int32_t  size;
    int locate;
};
struct HandleMORNFile
{
    FILE *f;
    MList *list;
    int filesize;
};
void endMORNFile(struct HandleMORNFile *handle)
{
    if(handle->f != NULL)
    {
        fseek(handle->f,4,SEEK_SET);
        fwrite(&(handle->filesize),4,1,handle->f);
        fclose(handle->f);
    }
    if(handle->list != NULL) mListRelease(handle->list);
}
#define HASH_MORNFile 0x32e067b9

struct HandleMORNFile *MORNInit(MFile *file)
{
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    MHandle *hdl=mHandle(file,MORNFile);
    struct HandleMORNFile *handle = (struct HandleMORNFile *)(hdl->handle);
    if(hdl->valid == 0)
    {
        mException(INVALID_POINTER(file->filename),EXIT,"invalid input");
        if(handle->list == NULL) handle->list = mListCreate();
        
        if(handle->f==NULL) handle->f = fopen((const char *)(file->filename),"rb+");
        if(handle->f==NULL) handle->f = fopen((const char *)(file->filename),"wb+");
        mException(handle->f == NULL,EXIT,"cannot open %s",file->filename);
        fseek(handle->f,0,SEEK_SET);
        
        int filesize=fsize(handle->f);
        if(filesize==0)
        {
            handle->filesize = 4;
            fwrite("RIFF",1,4,handle->f);
            fwrite(&(handle->filesize),4,1,handle->f);
            fwrite("MORN",1,4,handle->f);
        }
        else
        {
            int ID;fread(&ID,1,4,handle->f);
            mException((memcmp(&ID,"RIFF",4)!=0),EXIT,"invalid file format");
            
            fread(&handle->filesize,4,1,handle->f);
            
            fread(&ID,4,1,handle->f);
            mException((memcmp(&ID,"MORN",4)!=0),EXIT,"invalid file format");

            int locate = 4+4+4;
            struct Chunk chunk;
            while(1)
            {
                fread(&(chunk.ID  ),1,4,handle->f);
                fread(&(chunk.size),1,4,handle->f);
                locate = locate + 4+4;chunk.locate = locate;
                mListWrite(handle->list,DFLT,&chunk,sizeof(struct Chunk));
                
                locate = locate + chunk.size;
                if(locate>=filesize)break;
                if(fseek(handle->f,chunk.size,SEEK_CUR)!=0)break;
            }
        }

        hdl->valid = 1;
    }
    return handle;
}

struct Chunk *MornChunk(struct HandleMORNFile *handle,int ID)
{
    struct Chunk *ck;
    for(int i=0;i<handle->list->num;i++)
    {
        ck = (struct Chunk *)(handle->list->data[i]);
        if(ck->ID==ID) return ck;
    }
    return NULL;
}

int mMORNSize(MFile *file,int ID)
{
    struct HandleMORNFile *handle = MORNInit(file);
    struct Chunk *ck=MornChunk(handle,ID);
    if(ck==NULL) return 0;
    return ck->size;
}

int mMORNRead(MFile *file,int ID,void **data,int num,int size)
{
    struct HandleMORNFile *handle = MORNInit(file);
    struct Chunk *ck=MornChunk(handle,ID);
    if(ck==NULL) return MORN_FAIL;
    
    if( num<=0) num=1;
    if(size<=0) {mException((ck->size%num!=0),EXIT,"invalid input");size=ck->size/num;}
    mException((ck->size<size*num),EXIT,"no enough data for %d,size need %d,chunk size is %d",ID,size*num,ck->size);

    fseek(handle->f,ck->locate,SEEK_SET);
    for(int i=0;i<num;i++) fread(data[i],size,1,handle->f);
    return ck->size;
}

int mMORNWrite(MFile *file,int ID,void **data,int num,int size)
{
    if(num<=0) num=1;
    struct HandleMORNFile *handle = MORNInit(file);
    struct Chunk *ck=MornChunk(handle,ID);
    if(ck!=NULL)
    {
        if(size<=0) {mException((ck->size%num!=0),EXIT,"invalid input");size=ck->size/num;}
        mException(ck->size!=num*size,EXIT,"invalid input");
        
        fseek(handle->f,ck->locate,SEEK_SET);
        for(int i=0;i<num;i++)fwrite(data[i],1,size,handle->f);
        return ck->size;
    }
    
    mException((size<=0),EXIT,"invalid input");
    
    fseek(handle->f,0,SEEK_END);
    fwrite(&ID,1,4,handle->f);
    int chunk_size=size*num;fwrite(&chunk_size,4,1,handle->f);
    handle->filesize = handle->filesize + chunk_size+4+4;
    
    struct Chunk chunk;
    chunk.size = chunk_size;
    chunk.ID = ID;
    chunk.locate = ftell(handle->f);
    mListWrite(handle->list,DFLT,&chunk,sizeof(struct Chunk));
    
    for(int i=0;i<num;i++)fwrite(data[i],1,size,handle->f);
    return chunk_size;
}
