#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_math.h"

#define fread(Data,Size,Num,Fl) mException((fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error");

#define SYMBOL_S8  1
#define SYMBOL_U8  2
#define SYMBOL_S16 3
#define SYMBOL_U16 4
#define SYMBOL_S32 5
#define SYMBOL_U32 6
#define SYMBOL_F32 7
#define SYMBOL_F64 9
#define SYMBOL_S64 12
#define SYMBOL_U64 13
#define SYMBOL_MAT 14
#define SYMBOL_CPS 15

#define SAVE_TAG(type,size,f) {\
    short sdata;\
    int idata;\
    if(size<=4)\
    {\
        sdata = SYMBOL_##type;\
        fwrite(&sdata,2,1,f);\
        sdata = size;\
        fwrite(&sdata,2,1,f);\
    }\
    else\
    {\
        idata = SYMBOL_##type;\
        fwrite(&idata,4,1,f);\
        idata = size;\
        fwrite(&idata,4,1,f);\
    }\
}

/////////////////////////////////////////////////////////
// 接口功能：
//  保存.mat文件的文件头
//
// 参数：
//  (I)f(NO) - 文件指针
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mMATSaveHeader(FILE *f)
{
    char message[116];
    
    int buff_32;
    short buff_16;
    
    mException((INVALID_POINTER(f)),EXIT,"invalid input");
    
    fseek(f,0,SEEK_SET);
    
    sprintf(message,"M MAT-file\n");
    fwrite(message,1,116,f);
    buff_32 = 0x20202020;
    fwrite(&buff_32,4,1,f);
    fwrite(&buff_32,4,1,f);
    buff_16 = 0x0100;
    fwrite(&buff_16,2,1,f);
    buff_16 = 0x4d49;
    fwrite(&buff_16,2,1,f);
}

/////////////////////////////////////////////////////////
// 接口功能：
//  将文件存入.mat文件
//
// 参数：
//  (I)mat(NO) - 待保存的矩阵
//  (I)mat_name(NO) - 待保存的矩阵名
//  (I)f(NO) - 文件指针
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mMATSaveData(MMatrix *mat,char *mat_name,FILE *f)
{
    int buff_32;
    
    int mat_size;
    int size;
    
    int i,j;
    
    mat_size = strlen(mat_name);
    mException((INVALID_MAT(mat))||(mat_size==0)||(INVALID_POINTER(f)),EXIT,"invalid input");
    
    size = (8+8) + (8+8);
    if(mat_size<=4)
        size = size + (4+4);
    else
        size = size + (8+((mat_size + 7)&0xFFFFFFF8));
    
    size = size + (8+ ((mat->row * mat->col *sizeof(float)+7)&0xFFFFFFF8));
    
    SAVE_TAG(MAT,size,f);
    {
        SAVE_TAG(U32,8,f);
        buff_32 = 0x07; fwrite(&buff_32,4,1,f);
        buff_32 = 0;    fwrite(&buff_32,4,1,f);
        
        SAVE_TAG(S32,8,f);
        fwrite(&(mat->row),4,1,f);
        fwrite(&(mat->col),4,1,f);
        
        
        SAVE_TAG(S8,mat_size,f);
        if(mat_size<=4)
            fwrite(mat_name,1,4,f);
        else
            fwrite(mat_name,1,((mat_size+7)&0xFFFFFFF8),f);
        
        SAVE_TAG(F32,mat->row * mat->col *sizeof(float),f);
        for(i=0;i<mat->col;i++)
            for(j=0;j<mat->row;j++)
                fwrite(&(mat->data[j][i]),4,1,f);
            
        if((mat->row * mat->col)&0x01)
        {
            buff_32 = 0;
            fwrite(&buff_32,4,1,f);
        }
    }
}

// void mMATSave(MMatrix *mat[],char *mat_name[],int mat_num,const char *filename)
// {
    // mException((INVALID_POINTER(mat))||(INVALID_POINTER(mat_name)),EXIT,"invalid input");
    // if(mat_num <=0)mat_num = 1;
    
    // FILE *f = fopen(filename,"wb");
    // mException((f == NULL),EXIT,"file cannot open");
    
    // mMATSaveHeader(f);
    // for(int n=0;n<mat_num;n++)
    // {
        // mException((mat_name[n]==NULL)||(mat[n]==NULL),EXIT,"invalid input");
        // mMATSaveData(mat[n],mat_name[n],f);
    // }
    // fclose(f); 
// }

struct HandleMAT
{
    FILE *f;
};
void endMAT(void *info)
{
    struct HandleMAT *handle = info;
    fclose(handle->f);
}
#define HASH_MAT 0xa26c3e4d
void mMATWrite(MFile *file,MMatrix *mat,char *mat_name)
{
    mException(INVALID_POINTER(file),EXIT,"invalid input");
    MHandle *hdl; ObjectHandle(file,MAT,hdl);
    struct HandleMAT *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        if(handle->f==NULL) handle->f = fopen(file->filename,"wb+");
        mException((handle->f == NULL),EXIT,"file cannot open");
        mMATSaveHeader(handle->f);
        
        hdl->valid = 1;
    }
    fseek(handle->f,0,SEEK_END);
    mException((mat_name==NULL)||(mat==NULL),EXIT,"invalid input");
    mMATSaveData(mat,mat_name,handle->f);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  在.mat文件中寻找特定名称的矩阵
//
// 参数：
//  (I)f(NO) - .mat文件的指针
//  (I)matname(NO) - 需寻找的矩阵名
//  (O)row(NULL) - 所找到的矩阵的高度
//  (O)col(NULL) - 所找到的矩阵的宽度
//
// 返回值：
//  找到了相应的矩阵则返回1，没有找到则返回0
/////////////////////////////////////////////////////////
int mMATSearch(FILE *f,char *matname,int *row,int *col)
{
    char name[256];
    int type;
    int size;
    int buff[2];
    
    int lenth;
    
    int mat_size;
    
    int file_size;
    
    int mat_col,mat_row;
    
    mException((INVALID_POINTER(f))||(INVALID_POINTER(matname)),EXIT,"invalid input");
    if(INVALID_POINTER(row))
        row = &mat_row;
    if(INVALID_POINTER(col))
        col = &mat_col;
    
    mat_size = strlen(matname);
    
    fseek(f,0,SEEK_END);
    file_size = ftell(f);    
    
    fseek(f,128,SEEK_SET);
    
    // printf("file_size is %d\n",file_size);
    
    while(ftell(f)<file_size)
    {
        fread(&type,4,1,f);
        fread(&size,4,1,f);
        // printf("type is %d,size is %d         %d\n",type,size,ftell(f));        
        
        if(type != SYMBOL_MAT)
        {
            fseek(f,size,SEEK_CUR);
            continue;
        }
        
        fread(buff,1,8,f);
        fread(buff,1,8,f);
        fread(buff,1,8,f);
        
        fread(row,4,1,f);
        fread(col,4,1,f);
        
        if(mat_size <=4)
        {
            fread(buff,4,1,f);
            fread(name,1,4,f);
            lenth = 0;
        }
        else
        {
            fread(&type,4,1,f);
            fread(&lenth,4,1,f);
            
            if(lenth == mat_size)
            {
                lenth = ((lenth+7)&0xFFFFFFF8);                
                fread(name,1,lenth,f);
            }
            else
                lenth = 0;
        }
        name[mat_size] = '\0';
        
        if(strcmp(name,matname)==0)
            break;

        fseek(f,size-8-8-8-8-8-lenth,SEEK_CUR);
    }
    if(ftell(f)>=file_size)
    {
        *col = 0;
        *row = 0;
        return 0;
    }
    
    return 1;
}

void mMATReadData(FILE *f,char *matname,MMatrix *dst)
{
    int col,row;
    
    int data_type;
    int data_size;
    
    mException((INVALID_POINTER(f))||(INVALID_POINTER(dst)),EXIT,"file cannot open");
    
    
    if(!mMATSearch(f,matname,&row,&col))
        mException(1,EXIT,"cannot find such mat name");
    mMatrixRedefine(dst,row,col);
    
    fread(&data_type,4,1,f);
    fread(&data_size,4,1,f);
    mException((data_size < row*col*sizeof(float)),EXIT,"invalid file format");
  
    #define READ_DAT(type) {\
        int i,j;\
        type data_buff;\
        for(i=0;i<col;i++)\
            for(j=0;j<row;j++)\
            {\
                fread(&data_buff,sizeof(type),1,f);\
                dst->data[j][i] = (float)data_buff;\
            }\
    }

    if(data_type == SYMBOL_F32)
        READ_DAT(float)
    else if(data_type == SYMBOL_F64)
        READ_DAT(double)
    else if(data_type == SYMBOL_U8)
        READ_DAT(unsigned char)
    else if(data_type == SYMBOL_S8)
        READ_DAT(char)
    else if(data_type == SYMBOL_U16)
        READ_DAT(unsigned short)
    else if(data_type == SYMBOL_S16)
        READ_DAT(short)
    else if(data_type == SYMBOL_U32)
        READ_DAT(unsigned int)
    else if(data_type == SYMBOL_S32)
        READ_DAT(int)
    else
        mException(1,EXIT,"invalid file format");
}

void mMATRead(MFile *file,char *matname,MMatrix *dst)
{
    MHandle *hdl; ObjectHandle(file,MAT,hdl);
    struct HandleMAT *handle = hdl->handle;
    mException(hdl->valid == 0,EXIT,"no mat in file");
    
    mMATReadData(handle->f,matname,dst);
}




