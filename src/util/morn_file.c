#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_math.h"

#define fread(Data,Size,Num,Fl) mException((fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error");
#define fwrite(Data,Size,Num,Fl) mException((fwrite(Data,Size,Num,Fl)!=Num),EXIT,"write file error");

struct Chunk
{
    uint32_t hash;
    int32_t  size;
    int locate;
};

struct HandleMORNRead
{
    FILE *f;
    MList *list;
    uint32_t hash;
    int size;
};
void endMORNRead(void *info)
{
    struct HandleMORNRead *handle = (struct HandleMORNRead *)info;
    if(handle->f != NULL)
        fclose(handle->f);
    if(handle->list != NULL)
        mListRelease(handle->list);
}
#define HASH_MORNRead 0x32e067b9

void MORNList(FILE *f,MList *list)
{
    fseek(f,0,SEEK_SET);
    
    char chunk_name[4];
    fread(chunk_name,4,1,f);
    mException((memcmp(chunk_name,"RIFF",4)!=0),EXIT,"invalid file format");
    
    int file_size;
    fread(&file_size,4,1,f);
    
    fread(chunk_name,4,1,f);
    mException((memcmp(chunk_name,"MORN",4)!=0),EXIT,"invalid file format");

    int locate = 4+4+4;
    struct Chunk chunk;
    while(1)
    {
        fread(&(chunk.hash),1,4,f);
        fread(&(chunk.size),1,4,f);
        // printf("chunk.hash is %x,chunk.size is %d\n",chunk.hash,chunk.size);
        locate = locate + 4+4;
        chunk.locate = locate;
        mListWrite(list,DFLT,&chunk,sizeof(struct Chunk));
        
        locate = locate + chunk.size;
        if(locate>file_size)
            return;
        
        if(fseek(f,chunk.size,SEEK_CUR)!=0)
            return;
    }
}
    

int mMORNSize(MFile *file,const char *name)
{
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(file,MORNRead,hdl);
    struct HandleMORNRead *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        mException(INVALID_POINTER(file->object),EXIT,"invalid input");
        if(handle->f!=NULL) fclose(handle->f);
        handle->f = fopen(file->object,"rb");
        if(handle->f == NULL) return 0;
       
        if(handle->list == NULL)
            handle->list = mListCreate(DFLT,NULL);
        
        char chunk_name[4];
        fread(chunk_name,4,1,handle->f);
        mException((memcmp(chunk_name,"RIFF",4)!=0),EXIT,"invalid file format");
        
        int file_size;
        fread(&file_size,4,1,handle->f);
        
        fread(chunk_name,4,1,handle->f);
        mException((memcmp(chunk_name,"MORN",4)!=0),EXIT,"invalid file format");

        int locate = 4+4+4;
        struct Chunk chunk;
        while(1)
        {
            fread(&(chunk.hash),1,4,handle->f);
            fread(&(chunk.size),1,4,handle->f);
            // printf("chunk.hash is %x,chunk.size is %d\n",chunk.hash,chunk.size);
            locate = locate + 4+4;
            chunk.locate = locate;
            mListWrite(handle->list,DFLT,&chunk,sizeof(struct Chunk));
            
            locate = locate + chunk.size;
            if(locate>file_size)
                break;
            
            if(fseek(handle->f,chunk.size,SEEK_CUR)!=0)
                break;
        }

        hdl->valid = 1;
    }
    
    uint32_t hash = mHash(name,DFLT);
    if(handle->hash!=hash)
    {
        int i;
        for(i=0;i<handle->list->num;i++)
        {
            struct Chunk *ck = (struct Chunk *)(handle->list->data[i]);
            if(ck->hash==hash)
            {
                handle->hash=ck->hash;
                handle->size = ck->size;
                fseek(handle->f,ck->locate,SEEK_SET);
                
                break;
            }
        }
        if(i==handle->list->num)
            return 0;
    }
   
    return handle->size;
}

void mMORNRead(MFile *file,const char *name,void **data,int num,int size)
{
    int chunk_size = mMORNSize(file,name);
    
    if(num<=0) num=1;
    if((num==1)&&(size<=0)) size = chunk_size;
    else mException((size<=0),EXIT,"invalid input");
    
    mException((chunk_size<size*num),EXIT,"no enough data for %s,size need %d,chunk_size is %d",name,size*num,chunk_size);
    
    MHandle *hdl; ObjectHandle(file,MORNRead,hdl);
    struct HandleMORNRead *handle = hdl->handle;
    
    // fseek(handle->f,handle->locate,SEEK_SET);
    
    // printf("locate is %d\n",ftell(handle->f));
    for(int i=0;i<num;i++)
        fread(data[i],size,1,handle->f);
    
    // handle->locate = handle->locate + size*num;
    handle->size = handle->size - size*num;
}

struct HandleMORNWrite
{
    FILE *f;
    int size;
};
void endMORNWrite(void *info)
{
    struct HandleMORNWrite *handle = (struct HandleMORNWrite *)info;
    if(handle->f != NULL)
    {
        fseek(handle->f,4,SEEK_SET);
        fwrite(&(handle->size),4,1,handle->f);
        fclose(handle->f);
    }
}
#define HASH_MORNWrite 0x2287bd92

void mMORNWrite(MFile *file,const char *name,void **data,int num,int size)
{
    mException(INVALID_POINTER(file)||(size<=0),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(file,MORNWrite,hdl);
    struct HandleMORNWrite *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        mException(INVALID_POINTER(file->object),EXIT,"invalid input");
        if(handle->f!=NULL) fclose(handle->f);
        handle->f = fopen(file->object,"wb");
        mException((handle->f == NULL),EXIT,"file cannot open");
        handle->size = 4;
      
        fwrite("RIFF",1,4,handle->f);
        fwrite(&(handle->size),4,1,handle->f);
        fwrite("MORN",1,4,handle->f);
        
        hdl->valid =1;
    }
    fseek(handle->f,0,SEEK_END);
    
    int hash = mHash(name,DFLT);
    fwrite(&hash,1,4,handle->f);
    int chunk_size = size*num;
    
    fwrite(&chunk_size,4,1,handle->f);
    handle->size = handle->size + chunk_size + 4+4;
    
    if(num<=0) num=1;
    for(int i=0;i<num;i++)
        fwrite(data[i],1,size,handle->f);
}
