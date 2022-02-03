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



/*
#if defined(_WIN64)||defined(_WIN32)
#include <Windows.h>
#define m_Exist(ID) (GetProcessVersion(ID/1000)!=0)
#define m_Open(Filename) CreateFile(Filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,0)
#define m_Close(File) CloseHandle(File)
#define m_Seek(Handle,Locate) do{\
    if((Locate<0)||(handle->file_size==0)) handle->file_size=SetFilePointer(Handle->file,0,NULL,FILE_END);\
    if(Locate>0) {SetFilePointer(File,Locate,NULL,FILE_BEGIN);handle->file_size=MAX(Locate,handle->file_size);\
}\
    
    SetFilePointer(File,MAX(0,Locate),NULL,(Locate>=0)?FILE_BEGIN:FILE_END)

#define m_Read(File,Locate,Pointer,Size) do{int A;\
    do{A=SetFilePointer(File,Locate,NULL,FILE_BEGIN);}while(A!=Locate);\
    do{A=ReadFile(File,Pointer,Size,NULL,NULL);}while(A==0);\
}while(0)
#define m_Write(File,Locate,Pointer,Size) do{int A;\
    do{A=SetFilePointer(File,Locate,NULL,FILE_BEGIN);}while(A!=Locate);\
    do{A=WriteFile(File,Pointer,Size,NULL,NULL);}while(A==0);\
}while(0)
#define m_Lock(File)   LockFile(File,0,0,2*sizeof(int),0)
#define m_Unlock(File) UnlockFile(File,0,0,2*sizeof(int),0)
// #define m_Fsize(File) GetFileSize(File,NULL)
#define m_Fsize(File) SetFilePointer(File,0,NULL,FILE_END)
#define m_Mmap(Handle,Pointer,Size,Offset) do{\
    HANDLE Map = CreateFileMapping(File,NULL,PAGE_READWRITE,0,Size,NULL);\
    Pointer = MapViewOfFile(Map,FILE_MAP_ALL_ACCESS,0,0,Size);\
    CloseHandle(Map);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Munmap(Pointer,Size) UnmapViewOfFile(Pointer);
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <signal.h>
#define m_Exist(ID) (kill((ID/1000),0)==0)
#define m_Open(Filename) open(Filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
#define m_Close(File) close(File)
#define m_Read(File,Locate,Pointer,Size) do{\
    mException((lseek(File,Locate,SEEK_SET)!=Locate),EXIT,"error with file lseek");\
    mException((read(File,Pointer,Size)!=Size),EXIT,"error with file read");\
}while(0)
#define m_Write(File,Locate,Pointer,Size) do{\
    mException((lseek(File,Locate,SEEK_SET)!=Locate),EXIT,"error with file lseek");\
    mException((write(File,Pointer,Size)!=Size),EXIT,"error with file write");\
}while(0)
#define m_Lock(File)   flock(File,LOCK_EX)
#define m_Unlock(File) flock(File,LOCK_UN)
#define m_Fsize(File) lseek(File,1,SEEK_END)
#define m_Mmap(File,Pointer,Size) do{\
    Pointer=mmap(NULL,(Size),PROT_READ|PROT_WRITE,MAP_SHARED,File,0);\
    mException(Pointer==NULL,EXIT,"error with mmap");\
}while(0)
#define m_Munmap(Pointer,Size) munmap(Pointer,Size);
#endif

struct HandleFileRW
{
    #if defined(_WIN64)||defined(_WIN32)
    HANDLE file;
    #else
    int file;
    #endif
    int filesize;
    char *fileptr;
    int fileidle;
    
    char *block;
    int block_offset;
    int block_size;
};
void beginFileRW(MFile *file)//char *filename,struct HandleFileRW *handle)
{
    int *p = file->filename-sizeof(void *)*2;
    if(p[0]==HASH_FileRW) return (struct HandleFileRW *)(p+2);
    MHandle *hdl = mHandle(file,FileRW);
    struct HandleFileRW *handle = (struct HandleFileRW *)(hdl->handle);
    
    handle->file = m_Open(file->filename);
    m_Seek(handle,DFLT);
    
    memmove(file->filename+16,file->filename,256);
    p=file->filename-sizeof(void *)*2;
    p[0]=HASH_FileRW;
    memcpy(p+2,handle,sizeof(void *));

    return handle;
}
void mFileWrite(MFile *file,void *data,int size)
{
    MHandle *handle = beginFileRW(file);
    if(handle->block==NULL)
    {
        handle->block_size   =(size/4096+1)*4096;
        handle->block_offset = handle->filesize;
        
        m_Seek(handle,handle->filesize+handle->block_size);
        m_Write(handle,' ',1);
        m_Seek(handle,handle->filesize);
        handle->block = m_Mmap(handle,handle->block_offset,handle->block_size);
        handle->fileptr = handle->block;
    }
    mem
}
*/