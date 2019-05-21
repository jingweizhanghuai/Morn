#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_math.h"

/////////////////////////////////////////////////////////
// 接口功能:
//  快速排序
//
// 参数：
//  (I/O)data(NO) - 需排序的数据序列
//  (I)num(NO) - 数据序列的数量
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mQuickSort(float *data,int num)
{
    int i,j;
    float thresh;
    
    mException((INVALID_POINTER(data)),EXIT,"invalid input");
    
    if(num <= 1)
        return;
    
    thresh = data[0];
    i=0;j=num-1;
    while(i<j)
    {
        while(data[j]>=thresh)
        {
            if(j==i)
                goto next_sort;
            j=j-1;
        }
        
        data[i] = data[j];
        
        while(data[i]<=thresh)
        {
            if(i==j)
                goto next_sort;
            i=i+1;
        }
        
        data[j] = data[i];
    }

next_sort:
    data[i] = thresh;
    mQuickSort(data,i);
    mQuickSort(data+i+1,num-i-1);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  带索引的快速排序
//
// 参数：
//  (I/O)data(NO) - 需排序的数据序列
//  (I/O)index(NO) - 数据序列的索引
//  (I)num(NO) - 数据序列的数量
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mQuickSortWithIndex(float *data,int *index,int num)
{
    int i,j;
    float thresh;
    int thresh_index;
    
    if(INVALID_POINTER(index))
    {
        mQuickSort(data,num);
        return;
    }
    
    mException((INVALID_POINTER(data)),EXIT,"invalid input");
    
    if(num <= 1)
        return;
    
    thresh = data[0];
    thresh_index = index[0];
    i=0;j=num-1;
    while(i<j)
    {
        while(data[j]>=thresh)
        {
            if(j==i)
                goto next_sort_with_index;
            j=j-1;
        }
        
        data[i] = data[j];
        index[i] = index[j];
        
        while(data[i]<=thresh)
        {
            if(i==j)
                goto next_sort_with_index;
            i=i+1;
        }
        
        data[j] = data[i];
        index[j] = index[i];
    }

next_sort_with_index:
    data[i] = thresh;
    index[i] = thresh_index;
    
    mQuickSortWithIndex(data,index,i);
    mQuickSortWithIndex(data+i+1,index+i+1,num-i-1);
}


void AscBubbleSort(float *data_in,float *data_out,int num)
{
    int i,j;
    int flag;
    float buff;
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    
    if(num <= 1)
        return;
    
    flag = 1;
    for(i=0;flag==1;i++)
    {
        flag = 0;
        for(j=i;j<num-i-1;j++)
        {
            if(data_in[j+1]<data_in[j])
            {
                buff = data_in[j];
                data_in[j] = data_in[j+1];
                data_in[j+1] = buff;
                flag = 1;
            }
        }
        for(j=num-i-1;j>i;j--)
        {
            if(data_in[j]<data_in[j-1])
            {
                buff = data_in[j];
                data_in[j] = data_in[j-1];
                data_in[j-1] = buff;
                flag = 1;
            }
        }
    }
}

void AscBubbleSortWithIndex(float *data_in,int *index_in,float *data_out,int *index_out,int num)
{
    int i,j;
    int flag;
    float data_buff;
    int index_buff;
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if(INVALID_POINTER(index_in)&&INVALID_POINTER(index_out))
    {
        AscBubbleSort(data_in,data_out,num);
        return;
    }
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    if(INVALID_POINTER(index_in))
    {
        for(i=0;i<num;i++)
            index_out[i] = i;
        index_in = index_out;
    }
    else if((!INVALID_POINTER(index_out))&&(index_in != index_out))
    {
        memcpy(index_out,index_in,num*sizeof(int));
        index_in = index_out;
    }
 
    if(num <= 1)
        return;
    
    flag = 1;
    for(i=0;flag==1;i++)
    {
        flag = 0;
        for(j=i;j<num-i-1;j++)
        {
            if(data_in[j+1]<data_in[j])
            {
                data_buff = data_in[j];
                data_in[j] = data_in[j+1];
                data_in[j+1] = data_buff;
                
                index_buff = index_in[j];
                index_in[j] = index_in[j+1];
                index_in[j+1] = index_buff;
                
                flag = 1;
            }
        }
        for(j=num-i-1;j>i;j--)
        {
            if(data_in[j]<data_in[j-1])
            {
                data_buff = data_in[j];
                data_in[j] = data_in[j-1];
                data_in[j-1] = data_buff;
                
                index_buff = index_in[j];
                index_in[j] = index_in[j-1];
                index_in[j-1] = index_buff;
                
                flag = 1;
            }
        }
    }
}

void mAscBubbleSort(float *data_in,int *index_in,float *data_out,int *index_out,int num)
{
    if((index_out == NULL)&&(index_in == NULL))
        AscBubbleSort(data_in,data_out,num);
    else
        AscBubbleSortWithIndex(data_in,index_in,data_out,index_out,num);
}


    
    

void AscQuickSort(float *data_in,float *data_out,int num)
{
    int i,j;
    float thresh;
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    
    if(num <= 1)
        return;
    
    thresh = data_in[0];
    i=0;j=num-1;
    while(i<j)
    {
        while(data_in[j]>=thresh)
        {
            if(j==i) goto AscQuickSort_next;
            j=j-1;
        }
        data_in[i] = data_in[j];
        
        while(data_in[i]<=thresh)
        {
            if(i==j) goto AscQuickSort_next;
            i=i+1;
        }
        data_in[j] = data_in[i];
    }

AscQuickSort_next:
    data_in[i] = thresh;
    AscQuickSort(data_in,NULL,i);
    AscQuickSort(data_in+i+1,NULL,num-i-1);
}

void AscQuickSortIndex(float *data_in,int *index_in,float *data_out,int *index_out,int num)
{
    int i,j;
    float thresh;
    int thresh_index;
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if(INVALID_POINTER(index_in)&&INVALID_POINTER(index_out))
    {
        AscQuickSort(data_in,data_out,num);
        return;
    }
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    if(INVALID_POINTER(index_in))
    {
        for(i=0;i<num;i++)
            index_out[i] = i;
        index_in = index_out;
    }
    else if((!INVALID_POINTER(index_out))&&(index_in != index_out))
    {
        memcpy(index_out,index_in,num*sizeof(int));
        index_in = index_out;
    }
 
    if(num <= 1)
        return;
    
    thresh = data_in[0];
    thresh_index = index_in[0];
    i=0;j=num-1;
    while(i<j)
    {
        while(data_in[j]>=thresh)
        {
            if(j==i)
                goto AscQuickSort_next;
            j=j-1;
        }
        
        data_in[i] = data_in[j];
        index_in[i] = index_in[j];
        
        while(data_in[i]<=thresh)
        {
            if(i==j)
                goto AscQuickSort_next;
            i=i+1;
        }
        
        data_in[j] = data_in[i];
        index_in[j] = index_in[i];
    }

AscQuickSort_next:
    data_in[i] = thresh;
    index_in[i] = thresh_index;
    AscQuickSortIndex(data_in,index_in,NULL,NULL,i);
    AscQuickSortIndex(data_in+i+1,index_in+i+1,NULL,NULL,num-i-1);
}

void mAscQuickSort(float *data_in,int *index_in,float *data_out,int *index_out,int num)
{
    if((index_in == NULL)&&(index_out == NULL))
        AscQuickSort(data_in,data_out,num);
    else
        AscQuickSortIndex(data_in,index_in,data_out,index_out,num);
}


// void mAscInsertionSort(float *data_in,int num)
// {
    // float buff;
    // int i,j;
    
    // if(data_in[0]>data_in[1]) 
    // {
        // buff = data_in[0]; 
        // data_in[0] = data_in[1]; 
        // data_in[1] = buff;
    // }
    
    // for(i=2;i<num;i++)
    // {
        // if(data_in[i]<data_in[i-1])
        // {
            // buff = data_in[i];
            // data_in[i] = data_in[i-1];
            // for(j=i-2;j>=0;j--)
            // {
                // if(buff>data_in[j])
                // {
                    // data_in[j+1] = buff;
                    // break;
                // }
                // else
                    // data_in[j+1] = data[j];
            // }
        // }
    // }
// }

void AscInsertionSort(float *data_in,float *data_out,int num)
{
    float buff;
    int i;
    int n1,n2,n;
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    
    if(num <= 1)
        return;
    
    if(data_in[0]>data_in[1]) 
    {
        buff = data_in[0]; 
        data_in[0] = data_in[1]; 
        data_in[1] = buff;
    }
    
    for(i=2;i<num;i++)
    {
        if(data_in[i]<data_in[i-1])
        {
            buff = data_in[i];
            if(buff <= data_in[0])
            {
                memmove(data_in+1,data_in,i*sizeof(float));
                data_in[0] = buff;
                continue;
            }
            
            n2 = i-1;
            n1 = 0;
            while(n2-n1>1)
            {
                n = (n1+n2)>>1;
                if(buff>data_in[n])
                    n1 = n;
                else
                    n2 = n;
            }
            memcpy(data_in+n2+1,data_in+n2,(i-n2)*sizeof(float));
            data_in[n2] = buff;
        }
    }
}

void AscInsertionSortWithIndex(float *data_in,int *index_in,float *data_out,int *index_out,int num)
{
    float data_buff;
    int index_buff;
    int i;
    int n1,n2,n;
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if(INVALID_POINTER(index_in)&&INVALID_POINTER(index_out))
    {
        AscInsertionSort(data_in,data_out,num);
        return;
    }
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    if(INVALID_POINTER(index_in))
    {
        for(i=0;i<num;i++)
            index_out[i] = i;
        index_in = index_out;
    }
    else if((!INVALID_POINTER(index_out))&&(index_in != index_out))
    {
        memcpy(index_out,index_in,num*sizeof(int));
        index_in = index_out;
    }
 
    if(num <= 1)
        return;
    
    if(data_in[0]>data_in[1]) 
    {
        data_buff = data_in[0]; 
        data_in[0] = data_in[1]; 
        data_in[1] = data_buff;
        
        index_buff = index_in[0];
        index_in[0] = index_in[1];
        index_in[1] = index_buff;
    }
    
    for(i=2;i<num;i++)
    {
        if(data_in[i]<data_in[i-1])
        {
            data_buff = data_in[i];
            index_buff = index_in[i];
            if(data_buff <= data_in[0])
            {
                memmove(data_in+1,data_in,i*sizeof(float));
                memmove(index_in+1,index_in,i*sizeof(int));
                data_in[0] = data_buff;
                index_in[0] = index_buff;
                continue;
            }
            
            n2 = i-1;
            n1 = 0;
            while(n2-n1>1)
            {
                n = (n1+n2)>>1;
                if(data_buff>data_in[n])
                    n1 = n;
                else
                    n2 = n;
            }
            memcpy(data_in+n2+1,data_in+n2,(i-n2)*sizeof(float));
            memcpy(index_in+n2+1,index_in+n2,(i-n2)*sizeof(int));
            data_in[n2] = data_buff;
            index_in[n2] = index_buff;
        }
    }
}

void mAscInsertionSort(float *data_in,int *index_in,float *data_out,int *index_out,int num)
{
    if((index_in==NULL)&&(index_out == NULL))
        AscInsertionSort(data_in,data_out,num);
    else
        AscInsertionSortWithIndex(data_in,index_in,data_out,index_out,num);
}
/*
void AscSort(float *data,float **buff,int num,void (*method)(float *,float *,int))
{    
    int i;
    int count0,count1,count2,count3,count4,count5,count6,count7,count8,count9;
    int n1,n2,n3,n4,n5,n6,n7,n8,n9;

    float edge[9];
    
    if(num<=32)
    {
        method(data,NULL,num);
        return;
    }
    
    method(data,edge,9);
    count0=0; count1=0; count2=0; count3=0; count4=0; count5=0; count6=0; count7=0; count8=0; count9=0;
    for(i=7;i<num;i++)
    {
             if(data[i]<edge[0]) {buff[0][count0] = data[i];count0=count0+1;}
        else if(data[i]<edge[1]) {buff[1][count1] = data[i];count1=count1+1;}
        else if(data[i]<edge[2]) {buff[2][count2] = data[i];count2=count2+1;}
        else if(data[i]<edge[3]) {buff[3][count3] = data[i];count3=count3+1;}
        else if(data[i]<edge[4]) {buff[4][count4] = data[i];count4=count4+1;}
        else if(data[i]<edge[5]) {buff[5][count5] = data[i];count5=count5+1;}
        else if(data[i]<edge[6]) {buff[6][count6] = data[i];count6=count6+1;}
        else if(data[i]<edge[7]) {buff[7][count7] = data[i];count7=count7+1;}
        else if(data[i]<edge[8]) {buff[8][count8] = data[i];count8=count8+1;}
        else                     {buff[9][count9] = data[i];count9=count9+1;}
    }
    
                                                  memcpy(data   ,buff[0],count0 *sizeof(float));
    n1 =     count0; data[n1] = edge[0]; n1=n1+1; memcpy(data+n1,buff[1],count1 *sizeof(float)); 
    n2 = n1 +count1; data[n2] = edge[1]; n2=n2+1; memcpy(data+n2,buff[2],count2 *sizeof(float)); 
    n3 = n2 +count2; data[n3] = edge[2]; n3=n3+1; memcpy(data+n3,buff[3],count3 *sizeof(float)); 
    n4 = n3 +count3; data[n4] = edge[3]; n4=n4+1; memcpy(data+n4,buff[4],count4 *sizeof(float));
    n5 = n4 +count4; data[n5] = edge[4]; n5=n5+1; memcpy(data+n5,buff[5],count5 *sizeof(float));
    n6 = n5 +count5; data[n6] = edge[5]; n6=n6+1; memcpy(data+n6,buff[6],count6 *sizeof(float));
    n7 = n6 +count6; data[n7] = edge[6]; n7=n7+1; memcpy(data+n7,buff[7],count7 *sizeof(float));
    n8 = n7 +count7; data[n8] = edge[7]; n8=n8+1; memcpy(data+n8,buff[8],count8 *sizeof(float));
    n9 = n8 +count8; data[n9] = edge[8]; n9=n9+1; memcpy(data+n9,buff[9],count9 *sizeof(float));
    
    AscSort(data   ,buff,count0,method);
    AscSort(data+n1,buff,count1,method);
    AscSort(data+n2,buff,count2,method);
    AscSort(data+n3,buff,count3,method);
    AscSort(data+n4,buff,count4,method);
    AscSort(data+n5,buff,count5,method);
    AscSort(data+n6,buff,count6,method);
    AscSort(data+n7,buff,count7,method);
    AscSort(data+n8,buff,count8,method);
    AscSort(data+n9,buff,count9,method);
}

void mAscSort(float *data_in,float *data_out,int num,void (*method)(float *,float *,int))
{
    float *buff[10];
    
    mException((INVALID_POINTER(data_in)),EXIT,"invalid input");
    
    if((!INVALID_POINTER(data_out))&&(data_out!=data_in))
    {
        memcpy(data_out,data_in,num*sizeof(float));
        data_in = data_out;
    }
    
    if(num <= 1)
        return;
    buff[0] = mMalloc(num*sizeof(float));
    buff[1] = mMalloc(num*sizeof(float));
    buff[2] = mMalloc(num*sizeof(float));
    buff[3] = mMalloc(num*sizeof(float));
    buff[4] = mMalloc(num*sizeof(float));
    buff[5] = mMalloc(num*sizeof(float));
    buff[6] = mMalloc(num*sizeof(float));
    buff[7] = mMalloc(num*sizeof(float));
    buff[8] = mMalloc(num*sizeof(float));
    buff[9] = mMalloc(num*sizeof(float));
    
    AscSort(data_in,buff,num,method);
    
    mFree(buff[0]);
    mFree(buff[1]);
    mFree(buff[2]);
    mFree(buff[3]);
    mFree(buff[4]);
    mFree(buff[5]);
    mFree(buff[6]);
    mFree(buff[7]);
    mFree(buff[8]);
    mFree(buff[9]);
}
*/


#define AscSort(Type,Data_in,Data_out,Num) {\
    int I,J;\
    Type Thresh;\
    \
    mException((INVALID_POINTER(Data_in)),EXIT,"invalid input");\
    \
    if((!INVALID_POINTER(Data_out))&&(Data_out!=Data_in))\
    {\
        memcpy(Data_out,Data_in,Num*sizeof(Type));\
        Data_in = Data_out;\
    }\
    \
    if(Num <= 1)\
        return;\
    \
    Thresh = Data_in[0];\
    I=0;J=Num-1;\
    while(I<J)\
    {\
        while(Data_in[J]>=Thresh)\
        {\
            if(J==I)\
                goto AscSort_next;\
            J=J-1;\
        }\
        \
        Data_in[I] = Data_in[J];\
        \
        while(Data_in[I]<=Thresh)\
        {\
            if(I==J)\
                goto AscSort_next;\
            I=I+1;\
        }\
        \
        Data_in[J] = Data_in[I];\
    }\
\
AscSort_next:\
    Data_in[I] = Thresh;\
    mAscSort##Type(Data_in,NULL,NULL,NULL,I);\
    mAscSort##Type(Data_in+I+1,NULL,NULL,NULL,Num-I-1);\
}

#define AscSortIndex(Type,Data_in,Index_in,Data_out,Index_out,Num) {\
    int I,J;\
    Type Thresh;\
    int Thresh_index;\
    \
    mException((INVALID_POINTER(Data_in)),EXIT,"invalid input");\
    \
    if(INVALID_POINTER(Index_in)&&INVALID_POINTER(Index_out))\
    {\
        AscSort(Type,Data_in,Data_out,Num);\
        return;\
    }\
    \
    if((!INVALID_POINTER(Data_out))&&(Data_out!=Data_in))\
    {\
        memcpy(Data_out,Data_in,Num*sizeof(Type));\
        Data_in = Data_out;\
    }\
    if(INVALID_POINTER(Index_in))\
    {\
        for(I=0;I<Num;I++)\
            Index_out[I] = I;\
        Index_in = Index_out;\
    }\
    else if((!INVALID_POINTER(Index_out))&&(Index_in != Index_out))\
    {\
        memcpy(Index_out,Index_in,Num*sizeof(int));\
        Index_in = Index_out;\
    }\
    \
    if(Num <= 1)\
        return;\
    \
    Thresh = Data_in[0];\
    Thresh_index = Index_in[0];\
    I=0;J=Num-1;\
    while(I<J)\
    {\
        while(Data_in[J]>=Thresh)\
        {\
            if(J==I)\
                goto AscSortIndex_next;\
            J=J-1;\
        }\
        \
        Data_in[I] = Data_in[J];\
        Index_in[I] = Index_in[J];\
        \
        while(Data_in[I]<=Thresh)\
        {\
            if(I==J)\
                goto AscSortIndex_next;\
            I=I+1;\
        }\
        \
        Data_in[J] = Data_in[I];\
        Index_in[J] = Index_in[I];\
    }\
\
AscSortIndex_next:\
    Data_in[I] = Thresh;\
    Index_in[I] = Thresh_index;\
    mAscSort##Type(Data_in,Index_in,NULL,NULL,I);\
    mAscSort##Type(Data_in+I+1,Index_in+I+1,NULL,NULL,Num-I-1);\
}

void mAscSortD64(        double *data_in,int *index_in,        double *data_out,int *index_out,int num) {AscSortIndex(D64,data_in,index_in,data_out,index_out,num);}
void mAscSortF32(         float *data_in,int *index_in,         float *data_out,int *index_out,int num) {AscSortIndex(F32,data_in,index_in,data_out,index_out,num);}
void mAscSortS32(           int *data_in,int *index_in,           int *data_out,int *index_out,int num) {AscSortIndex(S32,data_in,index_in,data_out,index_out,num);}
void mAscSortS16(         short *data_in,int *index_in,         short *data_out,int *index_out,int num) {AscSortIndex(S16,data_in,index_in,data_out,index_out,num);}
void mAscSortS8(           char *data_in,int *index_in,          char *data_out,int *index_out,int num) {AscSortIndex( S8,data_in,index_in,data_out,index_out,num);}
void mAscSortU32(  unsigned int *data_in,int *index_in,  unsigned int *data_out,int *index_out,int num) {AscSortIndex(U32,data_in,index_in,data_out,index_out,num);}
void mAscSortU16(unsigned short *data_in,int *index_in,unsigned short *data_out,int *index_out,int num) {AscSortIndex(U16,data_in,index_in,data_out,index_out,num);}
void mAscSortU8(  unsigned char *data_in,int *index_in, unsigned char *data_out,int *index_out,int num) {AscSortIndex( U8,data_in,index_in,data_out,index_out,num);}

struct HandleSequenceBorder {
    union
    {
        int data_S32[2];
        unsigned int data_U32[2];
        short data_S16[2];
        unsigned short data_U16[2];
        char data_S8[2];
        unsigned char data_U8[2];
        float data_F32[2];
        double data_D64[2];
    };
    int index[2];
};
#define HASH_SequenceBorderD64 0x01
#define HASH_SequenceBorderF32 0x02
#define HASH_SequenceBorderS32 0x03
#define HASH_SequenceBorderU32 0x04
#define HASH_SequenceBorderS16 0x05
#define HASH_SequenceBorderU16 0x06
#define HASH_SequenceBorderS8  0x07
#define HASH_SequenceBorderU8  0x08
#define HandleSequenceBorderD64 HandleSequenceBorder
#define HandleSequenceBorderF32 HandleSequenceBorder
#define HandleSequenceBorderS32 HandleSequenceBorder
#define HandleSequenceBorderU32 HandleSequenceBorder
#define HandleSequenceBorderS16 HandleSequenceBorder
#define HandleSequenceBorderU16 HandleSequenceBorder
#define HandleSequenceBorderS8  HandleSequenceBorder
#define HandleSequenceBorderU8  HandleSequenceBorder
void endSequenceBorderD64(void *handle) {mFree(handle);}
void endSequenceBorderF32(void *handle) {mFree(handle);}
void endSequenceBorderS32(void *handle) {mFree(handle);}
void endSequenceBorderU32(void *handle) {mFree(handle);}
void endSequenceBorderS16(void *handle) {mFree(handle);}
void endSequenceBorderU16(void *handle) {mFree(handle);}
void endSequenceBorderS8( void *handle) {mFree(handle);}
void endSequenceBorderU8( void *handle) {mFree(handle);}
#define SequenceBorder(Type,Proc,Data_in,Index_in,Min,Min_index,Max,Max_index) {\
    MHandle *hdl; ObjectHandle(Proc,SequenceBorder##Type,hdl);\
    struct HandleSequenceBorder *handle = hdl->handle;\
    if(hdl->valid == 0)\
    {\
        handle->data_##Type[0] = Data_in;\
        handle->data_##Type[1] = Data_in;\
        handle->index[0] = Index_in;\
        handle->index[1] = Index_in;\
        hdl->valid = 1;\
    }\
    else if(Data_in<handle->data_##Type[0])\
    {\
        handle->data_##Type[0] = Data_in;\
        handle->index[0] = Index_in;\
    }\
    else if(Data_in>handle->data_##Type[1])\
    {\
        handle->data_##Type[1] = Data_in;\
        handle->index[1] = Index_in;\
    }\
    \
    if(Min!=NULL)          *Min = handle->data_##Type[0];\
    if(Min_index!=NULL)    *Min_index = handle->index[0];\
    if(Max!=NULL)          *Max = handle->data_##Type[1];\
    if(Max_index!=NULL)    *Max_index = handle->index[1];\
}
void mSequenceBorderD64(MObject *proc,        double data_in,int index_in,        double *min,int *min_index,        double *max,int *max_index) {SequenceBorder(D64,proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderF32(MObject *proc,         float data_in,int index_in,         float *min,int *min_index,         float *max,int *max_index) {SequenceBorder(F32,proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderS32(MObject *proc,           int data_in,int index_in,           int *min,int *min_index,           int *max,int *max_index) {SequenceBorder(S32,proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *min,int *min_index,  unsigned int *max,int *max_index) {SequenceBorder(U32,proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderS16(MObject *proc,         short data_in,int index_in,         short *min,int *min_index,         short *max,int *max_index) {SequenceBorder(S16,proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *min,int *min_index,unsigned short *max,int *max_index) {SequenceBorder(U16,proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderS8( MObject *proc,          char data_in,int index_in,          char *min,int *min_index,          char *max,int *max_index) {SequenceBorder(S8, proc,data_in,index_in,min,min_index,max,max_index);}
void mSequenceBorderU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *min,int *min_index, unsigned char *max,int *max_index) {SequenceBorder(U8, proc,data_in,index_in,min,min_index,max,max_index);}


struct SortTree {
    union
    {
        double data_D64;
        float data_F32;
        S32 data_S32;
        U32 data_U32;
        S16 data_S16;
        U16 data_U16;
        S8  data_S8;
        U8  data_U8;
    };
    int index;
    
    struct SortTree *parent;
    struct SortTree *left;
    struct SortTree *right;
};

#define SortTreeResultAsc(Type,Tree,Data_out,Index_out,Order) {\
    int n;\
    \
    if(Tree->left != NULL)\
        SortTreeResultAsc##Type(Tree->left,Data_out,Index_out,Order);\
    \
    n = *Order;\
    if(Data_out != NULL)\
        Data_out[n] = Tree->data_##Type;\
    if(Index_out != NULL)\
        Index_out[n] = Tree->index;\
    *Order = n+1;\
    \
    if(Tree->right != NULL)\
        SortTreeResultAsc##Type(Tree->right,Data_out,Index_out,Order);\
}
void SortTreeResultAscD64(struct SortTree *tree,        double *data_out,int *index_out,int *order) {SortTreeResultAsc(D64,tree,data_out,index_out,order);}
void SortTreeResultAscF32(struct SortTree *tree,         float *data_out,int *index_out,int *order) {SortTreeResultAsc(F32,tree,data_out,index_out,order);}
void SortTreeResultAscS32(struct SortTree *tree,           int *data_out,int *index_out,int *order) {SortTreeResultAsc(S32,tree,data_out,index_out,order);}
void SortTreeResultAscU32(struct SortTree *tree,  unsigned int *data_out,int *index_out,int *order) {SortTreeResultAsc(U32,tree,data_out,index_out,order);}
void SortTreeResultAscS16(struct SortTree *tree,         short *data_out,int *index_out,int *order) {SortTreeResultAsc(S16,tree,data_out,index_out,order);}
void SortTreeResultAscU16(struct SortTree *tree,unsigned short *data_out,int *index_out,int *order) {SortTreeResultAsc(U16,tree,data_out,index_out,order);}
void SortTreeResultAscS8( struct SortTree *tree,          char *data_out,int *index_out,int *order) {SortTreeResultAsc(S8, tree,data_out,index_out,order);}
void SortTreeResultAscU8( struct SortTree *tree, unsigned char *data_out,int *index_out,int *order) {SortTreeResultAsc(U8, tree,data_out,index_out,order);}

#define SortTreeResultDesc(Type,Tree,Data_out,Index_out,Order) {\
    int n;\
    \
    if(Tree->right != NULL)\
        SortTreeResultDesc##Type(Tree->right,Data_out,Index_out,Order);\
    \
    n = *Order;\
    if(Data_out != NULL)\
        Data_out[n] = Tree->data_##Type;\
    if(Index_out != NULL)\
        Index_out[n] = Tree->index;\
    *Order = n+1;\
    \
    if(Tree->left != NULL)\
        SortTreeResultDesc##Type(Tree->left,Data_out,Index_out,Order);\
}
void SortTreeResultDescD64(struct SortTree *tree,        double *data_out,int *index_out,int *order) {SortTreeResultDesc(D64,tree,data_out,index_out,order);}
void SortTreeResultDescF32(struct SortTree *tree,         float *data_out,int *index_out,int *order) {SortTreeResultDesc(F32,tree,data_out,index_out,order);}
void SortTreeResultDescS32(struct SortTree *tree,           int *data_out,int *index_out,int *order) {SortTreeResultDesc(S32,tree,data_out,index_out,order);}
void SortTreeResultDescU32(struct SortTree *tree,  unsigned int *data_out,int *index_out,int *order) {SortTreeResultDesc(U32,tree,data_out,index_out,order);}
void SortTreeResultDescS16(struct SortTree *tree,         short *data_out,int *index_out,int *order) {SortTreeResultDesc(S16,tree,data_out,index_out,order);}
void SortTreeResultDescU16(struct SortTree *tree,unsigned short *data_out,int *index_out,int *order) {SortTreeResultDesc(U16,tree,data_out,index_out,order);}
void SortTreeResultDescS8( struct SortTree *tree,          char *data_out,int *index_out,int *order) {SortTreeResultDesc(S8, tree,data_out,index_out,order);}
void SortTreeResultDescU8( struct SortTree *tree, unsigned char *data_out,int *index_out,int *order) {SortTreeResultDesc(U8, tree,data_out,index_out,order);}


void SortTreeRelease(struct SortTree *tree)
{
    if(tree->left != NULL)
        SortTreeRelease(tree->left);
    if(tree->right != NULL)
        SortTreeRelease(tree->right);
    
    mFree(tree);
}
     
struct HandleSequenceSort {
    struct SortTree *tree;
    struct SortTree *thresh;
    int num;
};

void endSequenceSort(void *info)
{
    struct HandleSequenceSort *handle = info;
    if(handle->tree != NULL)
        SortTreeRelease(handle->tree);
}
#define endSequenceAscSortD64 endSequenceSort
#define endSequenceAscSortF32 endSequenceSort
#define endSequenceAscSortS32 endSequenceSort
#define endSequenceAscSortU32 endSequenceSort
#define endSequenceAscSortS16 endSequenceSort
#define endSequenceAscSortU16 endSequenceSort
#define endSequenceAscSortS8  endSequenceSort
#define endSequenceAscSortU8  endSequenceSort

#define HASH_SequenceAscSortD64 0xeb3a3c23
#define HASH_SequenceAscSortF32 0xfa75b7a2
#define HASH_SequenceAscSortS32 0xb358b1ab
#define HASH_SequenceAscSortU32 0x52715d65
#define HASH_SequenceAscSortS16 0x3f5c89f5
#define HASH_SequenceAscSortU16 0xc66d851b
#define HASH_SequenceAscSortS8  0x3f741d0a
#define HASH_SequenceAscSortU8  0xc377ea6c
#define HandleSequenceAscSortD64 HandleSequenceSort
#define HandleSequenceAscSortF32 HandleSequenceSort
#define HandleSequenceAscSortS32 HandleSequenceSort
#define HandleSequenceAscSortU32 HandleSequenceSort
#define HandleSequenceAscSortS16 HandleSequenceSort
#define HandleSequenceAscSortU16 HandleSequenceSort
#define HandleSequenceAscSortS8  HandleSequenceSort
#define HandleSequenceAscSortU8  HandleSequenceSort

#define HASH_SequenceDescSortD64 0xe0b40deb
#define HASH_SequenceDescSortF32 0xefb4678a
#define HASH_SequenceDescSortS32 0x4896deb3
#define HASH_SequenceDescSortU32 0x47eb2f2d
#define HASH_SequenceDescSortS16 0xb49a8b5d
#define HASH_SequenceDescSortU16 0xdbe78283
#define HASH_SequenceDescSortS8  0x866a1bc2
#define HASH_SequenceDescSortU8  0x0a6de924
#define HandleSequenceDescSortD64 HandleSequenceSort
#define HandleSequenceDescSortF32 HandleSequenceSort
#define HandleSequenceDescSortS32 HandleSequenceSort
#define HandleSequenceDescSortU32 HandleSequenceSort
#define HandleSequenceDescSortS16 HandleSequenceSort
#define HandleSequenceDescSortU16 HandleSequenceSort
#define HandleSequenceDescSortS8  HandleSequenceSort
#define HandleSequenceDescSortU8  HandleSequenceSort

#define mSequenceSort(Type,Flag,Proc,Data_in,Index_in,Data_out,Index_out,Num) {\
    struct SortTree *tree;\
    struct SortTree *node;\
    \
    int order;\
    \
    MHandle *hdl; ObjectHandle(Proc,SequenceAscSort##Type,hdl);\
    struct HandleSequenceSort *handle = hdl->handle;\
    hdl->valid = 1;\
    \
    node = (struct SortTree *)mMalloc(sizeof(struct SortTree));\
    node->data_##Type = Data_in;\
    node->index = Index_in;\
    node->left = NULL;\
    node->right = NULL;\
    \
    if(handle->tree == NULL)\
        handle->tree = node;\
    else\
    {\
        tree = handle->tree;\
        \
        while(1)\
        {\
            if(Data_in <= tree->data_##Type)\
            {\
                if(tree->left == NULL)\
                {\
                    tree->left = node;\
                    break;\
                }\
                else\
                    tree = tree->left;\
            }\
            else\
            {\
                if(tree->right == NULL)\
                {\
                    tree->right = node;\
                    break;\
                }\
                else\
                    tree = tree->right;\
            }\
        }\
    }\
    handle->num = handle->num +1;\
    if(Num != NULL)\
        *Num = handle->num;\
    \
    if((Data_out != NULL)||(Index_out!=NULL))\
    {\
        order = 0;\
        SortTreeResult##Flag##Type(handle->tree,Data_out,Index_out,&order);\
    }\
}
    
void mSequenceAscSortD64(MObject *proc,        double data_in,int index_in,        double *data_out,int *index_out,int *num) {mSequenceSort(D64,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortF32(MObject *proc,         float data_in,int index_in,         float *data_out,int *index_out,int *num) {mSequenceSort(F32,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortS32(MObject *proc,           int data_in,int index_in,           int *data_out,int *index_out,int *num) {mSequenceSort(S32,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *data_out,int *index_out,int *num) {mSequenceSort(U32,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortS16(MObject *proc,         short data_in,int index_in,         short *data_out,int *index_out,int *num) {mSequenceSort(S16,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *data_out,int *index_out,int *num) {mSequenceSort(U16,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortS8( MObject *proc,          char data_in,int index_in,          char *data_out,int *index_out,int *num) {mSequenceSort(S8 ,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceAscSortU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *data_out,int *index_out,int *num) {mSequenceSort(U8 ,Asc,proc,data_in,index_in,data_out,index_out,num);}

void mSequenceDescSortD64(MObject *proc,        double data_in,int index_in,        double *data_out,int *index_out,int *num) {mSequenceSort(D64,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortF32(MObject *proc,         float data_in,int index_in,         float *data_out,int *index_out,int *num) {mSequenceSort(F32,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortS32(MObject *proc,           int data_in,int index_in,           int *data_out,int *index_out,int *num) {mSequenceSort(S32,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *data_out,int *index_out,int *num) {mSequenceSort(U32,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortS16(MObject *proc,         short data_in,int index_in,         short *data_out,int *index_out,int *num) {mSequenceSort(S16,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *data_out,int *index_out,int *num) {mSequenceSort(U16,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortS8( MObject *proc,          char data_in,int index_in,          char *data_out,int *index_out,int *num) {mSequenceSort(S8 ,Asc,proc,data_in,index_in,data_out,index_out,num);}
void mSequenceDescSortU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *data_out,int *index_out,int *num) {mSequenceSort(U8 ,Asc,proc,data_in,index_in,data_out,index_out,num);}

#define endSequenceMinSubsetD64 endSequenceSort
#define endSequenceMinSubsetF32 endSequenceSort
#define endSequenceMinSubsetS32 endSequenceSort
#define endSequenceMinSubsetU32 endSequenceSort
#define endSequenceMinSubsetS16 endSequenceSort
#define endSequenceMinSubsetU16 endSequenceSort
#define endSequenceMinSubsetS8  endSequenceSort
#define endSequenceMinSubsetU8  endSequenceSort
#define HASH_SequenceMinSubsetD64 0x97ee81e0
#define HASH_SequenceMinSubsetF32 0x98df1e59
#define HASH_SequenceMinSubsetS32 0x3ffca730
#define HASH_SequenceMinSubsetU32 0x40e37896
#define HASH_SequenceMinSubsetS16 0xd3f8fa86
#define HASH_SequenceMinSubsetU16 0xace72540
#define HASH_SequenceMinSubsetS8  0x6252d47d
#define HASH_SequenceMinSubsetU8  0xee4f1ceb
#define HandleSequenceMinSubsetD64 HandleSequenceSort
#define HandleSequenceMinSubsetF32 HandleSequenceSort
#define HandleSequenceMinSubsetS32 HandleSequenceSort
#define HandleSequenceMinSubsetU32 HandleSequenceSort
#define HandleSequenceMinSubsetS16 HandleSequenceSort
#define HandleSequenceMinSubsetU16 HandleSequenceSort
#define HandleSequenceMinSubsetS8  HandleSequenceSort
#define HandleSequenceMinSubsetU8  HandleSequenceSort

#define SequenceMinSubset(Type,Proc,Data_in,Index_in,Num,Data_out,Index_out) {\
    struct SortTree *tree;\
    struct SortTree *node;\
    Type Limit;\
    \
    int order;\
    if(Num <= 1)\
    {\
        mSequenceBorder##Type(Proc,Data_in,Index_in,Data_out,Index_out,NULL,NULL);\
        Limit = *Data_out;\
        return Limit;\
    }\
    \
    MHandle *hdl; ObjectHandle(Proc,SequenceMinSubset##Type,hdl);\
    struct HandleSequenceSort *handle = hdl->handle;\
    hdl->valid = 1;\
    \
    if(handle->num<Num)\
    {\
        node = (struct SortTree *)mMalloc(sizeof(struct SortTree));\
    }\
    else if(Data_in >= handle->thresh->data_##Type)\
    {\
        if((Data_out != NULL)||(Index_out!=NULL))\
        {\
            order = 0;\
            SortTreeResultAsc##Type(handle->tree,Data_out,Index_out,&order);\
        }\
        Limit = handle->thresh->data_##Type;\
        return Limit;\
    }\
    else\
    {\
        node = handle->thresh;\
        if(node->parent != NULL)\
        {\
            node->parent->right = node->left;\
            if(node->left != NULL)\
                node->left->parent = node->parent;\
            handle->thresh = node->parent;\
        }\
        else\
        {\
            handle->tree = node->left;\
            node->left->parent = NULL;\
            handle->thresh = node->left;\
        }\
    }\
    \
    node->data_##Type = Data_in;\
    node->index = Index_in;\
    node->left = NULL;\
    node->right = NULL;\
    \
    if(handle->tree == NULL)\
    {\
        node->parent = NULL;\
        handle->tree = node;\
        handle->thresh = node;\
    }\
    else\
    {\
        tree = handle->tree;\
        \
        while(1)\
        {\
            if(Data_in <= tree->data_##Type)\
            {\
                if(tree->left == NULL)\
                {\
                    node->parent = tree;\
                    tree->left = node;\
                    break;\
                }\
                else\
                    tree = tree->left;\
            }\
            else\
            {\
                if(tree->right == NULL)\
                {\
                    node->parent = tree;\
                    tree->right = node;\
                    break;\
                }\
                else\
                    tree = tree->right;\
            }\
        }\
    }\
    handle->num = handle->num +1;\
    \
    while(handle->thresh->right != NULL)\
        handle->thresh = handle->thresh->right;\
    Limit = handle->thresh->data_##Type;\
    \
    if((Data_out != NULL)||(Index_out!=NULL))\
    {\
        order = 0;\
        SortTreeResultAsc##Type(handle->tree,Data_out,Index_out,&order);\
    }\
    return Limit;\
}

        double mSequenceMinSubsetD64(MObject *proc,        double data_in,int index_in,        double *data_out,int *index_out,int num) {SequenceMinSubset(D64,proc,data_in,index_in,num,data_out,index_out);}
         float mSequenceMinSubsetF32(MObject *proc,         float data_in,int index_in,         float *data_out,int *index_out,int num) {SequenceMinSubset(F32,proc,data_in,index_in,num,data_out,index_out);}
           int mSequenceMinSubsetS32(MObject *proc,           int data_in,int index_in,           int *data_out,int *index_out,int num) {SequenceMinSubset(S32,proc,data_in,index_in,num,data_out,index_out);}
  unsigned int mSequenceMinSubsetU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *data_out,int *index_out,int num) {SequenceMinSubset(U32,proc,data_in,index_in,num,data_out,index_out);}
         short mSequenceMinSubsetS16(MObject *proc,         short data_in,int index_in,         short *data_out,int *index_out,int num) {SequenceMinSubset(S16,proc,data_in,index_in,num,data_out,index_out);}
unsigned short mSequenceMinSubsetU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *data_out,int *index_out,int num) {SequenceMinSubset(U16,proc,data_in,index_in,num,data_out,index_out);}
          char mSequenceMinSubsetS8( MObject *proc,          char data_in,int index_in,          char *data_out,int *index_out,int num) {SequenceMinSubset(S8, proc,data_in,index_in,num,data_out,index_out);}
 unsigned char mSequenceMinSubsetU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *data_out,int *index_out,int num) {SequenceMinSubset(U8, proc,data_in,index_in,num,data_out,index_out);}

#define endSequenceMaxSubsetD64 endSequenceSort
#define endSequenceMaxSubsetF32 endSequenceSort
#define endSequenceMaxSubsetS32 endSequenceSort
#define endSequenceMaxSubsetU32 endSequenceSort
#define endSequenceMaxSubsetS16 endSequenceSort
#define endSequenceMaxSubsetU16 endSequenceSort
#define endSequenceMaxSubsetS8  endSequenceSort
#define endSequenceMaxSubsetU8  endSequenceSort
#define HASH_SequenceMaxSubsetD64 0x0a6c43ce
#define HASH_SequenceMaxSubsetF32 0x9d7f8a87
#define HASH_SequenceMaxSubsetS32 0x981be396
#define HASH_SequenceMaxSubsetU32 0x97351230
#define HASH_SequenceMaxSubsetS16 0x041f9040
#define HASH_SequenceMaxSubsetU16 0x2b316586
#define HASH_SequenceMaxSubsetS8  0x58eb83eb
#define HASH_SequenceMaxSubsetU8  0xccef3b7d
#define HandleSequenceMaxSubsetD64 HandleSequenceSort
#define HandleSequenceMaxSubsetF32 HandleSequenceSort
#define HandleSequenceMaxSubsetS32 HandleSequenceSort
#define HandleSequenceMaxSubsetU32 HandleSequenceSort
#define HandleSequenceMaxSubsetS16 HandleSequenceSort
#define HandleSequenceMaxSubsetU16 HandleSequenceSort
#define HandleSequenceMaxSubsetS8  HandleSequenceSort
#define HandleSequenceMaxSubsetU8  HandleSequenceSort
 
#define SequenceMaxSubset(Type,Proc,Data_in,Index_in,Num,Data_out,Index_out) {\
    struct SortTree *tree;\
    struct SortTree *node;\
    Type Limit;\
    \
    int order;\
    if(Num <= 1)\
    {\
        mSequenceBorder##Type(Proc,Data_in,Index_in,NULL,NULL,Data_out,Index_out);\
        Limit = *Data_out;\
        return Limit;\
    }\
    \
    MHandle *hdl; ObjectHandle(Proc,SequenceMaxSubset##Type,hdl);\
    struct HandleSequenceSort *handle = hdl->handle;\
    hdl->valid = 1;\
    \
    if(handle->num<Num)\
    {\
        node = (struct SortTree *)mMalloc(sizeof(struct SortTree));\
    }\
    else if(Data_in <= handle->thresh->data_##Type)\
    {\
        if((Data_out != NULL)||(Index_out!=NULL))\
        {\
            order = 0;\
            SortTreeResultDesc##Type(handle->tree,Data_out,Index_out,&order);\
        }\
        Limit = handle->thresh->data_##Type;\
        return Limit;\
    }\
    else\
    {\
        node = handle->thresh;\
        if(node->parent != NULL)\
        {\
            node->parent->left = node->right;\
            if(node->right != NULL)\
                node->right->parent = node->parent;\
            handle->thresh = node->parent;\
        }\
        else\
        {\
            handle->tree = node->right;\
            node->right->parent = NULL;\
            handle->thresh = node->right;\
        }\
    }\
    \
    node->data_##Type = Data_in;\
    node->index = Index_in;\
    node->left = NULL;\
    node->right = NULL;\
    \
    if(handle->tree == NULL)\
    {\
        node->parent = NULL;\
        handle->tree = node;\
        handle->thresh = node;\
    }\
    else\
    {\
        tree = handle->tree;\
        \
        while(1)\
        {\
            if(Data_in <= tree->data_##Type)\
            {\
                if(tree->left == NULL)\
                {\
                    node->parent = tree;\
                    tree->left = node;\
                    break;\
                }\
                else\
                    tree = tree->left;\
            }\
            else\
            {\
                if(tree->right == NULL)\
                {\
                    node->parent = tree;\
                    tree->right = node;\
                    break;\
                }\
                else\
                    tree = tree->right;\
            }\
        }\
    }\
    handle->num = handle->num +1;\
    \
    while(handle->thresh->left != NULL)\
        handle->thresh = handle->thresh->left;\
    Limit = handle->thresh->data_##Type;\
    \
    if((Data_out != NULL)||(Index_out!=NULL))\
    {\
        order = 0;\
        SortTreeResultDesc##Type(handle->tree,Data_out,Index_out,&order);\
    }\
    return Limit;\
}

        double mSequenceMaxSubsetD64(MObject *proc,        double data_in,int index_in,        double *data_out,int *index_out,int num) {SequenceMaxSubset(D64,proc,data_in,index_in,num,data_out,index_out);}
         float mSequenceMaxSubsetF32(MObject *proc,         float data_in,int index_in,         float *data_out,int *index_out,int num) {SequenceMaxSubset(F32,proc,data_in,index_in,num,data_out,index_out);}
           int mSequenceMaxSubsetS32(MObject *proc,           int data_in,int index_in,           int *data_out,int *index_out,int num) {SequenceMaxSubset(S32,proc,data_in,index_in,num,data_out,index_out);}
  unsigned int mSequenceMaxSubsetU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *data_out,int *index_out,int num) {SequenceMaxSubset(U32,proc,data_in,index_in,num,data_out,index_out);}
         short mSequenceMaxSubsetS16(MObject *proc,         short data_in,int index_in,         short *data_out,int *index_out,int num) {SequenceMaxSubset(S16,proc,data_in,index_in,num,data_out,index_out);}
unsigned short mSequenceMaxSubsetU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *data_out,int *index_out,int num) {SequenceMaxSubset(U16,proc,data_in,index_in,num,data_out,index_out);}
          char mSequenceMaxSubsetS8( MObject *proc,          char data_in,int index_in,          char *data_out,int *index_out,int num) {SequenceMaxSubset(S8, proc,data_in,index_in,num,data_out,index_out);}
 unsigned char mSequenceMaxSubsetU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *data_out,int *index_out,int num) {SequenceMaxSubset(U8, proc,data_in,index_in,num,data_out,index_out);}

#define SequenceAscSortResult(Type,Flag,Proc,Data_out,Index_out) {\
    mException((Data_out ==NULL)&&(Index_out==NULL),EXIT,"invalid input");\
    \
    MHandle *hdl; ObjectHandle(Proc,SequenceAscSort##Type,hdl);\
    struct HandleSequenceSort *handle = hdl->handle;\
    mException((hdl->valid == 0),EXIT,"invalid input");\
    \
    int order = 0;\
    SortTreeResult##Flag##Type(handle->tree,Data_out,Index_out,&order);\
}
void mSequenceAscSortResultD64(MObject *proc,        double *data_out,int *index_out) {SequenceAscSortResult(D64,Asc,proc,data_out,index_out);}
void mSequenceAscSortResultF32(MObject *proc,         float *data_out,int *index_out) {SequenceAscSortResult(F32,Asc,proc,data_out,index_out);}
void mSequenceAscSortResultS32(MObject *proc,           int *data_out,int *index_out) {SequenceAscSortResult(S32,Asc,proc,data_out,index_out);}
void mSequenceAscSortResultU32(MObject *proc,  unsigned int *data_out,int *index_out) {SequenceAscSortResult(U32,Asc,proc,data_out,index_out);}
void mSequenceAscSortResultS16(MObject *proc,         short *data_out,int *index_out) {SequenceAscSortResult(S16,Asc,proc,data_out,index_out);}
void mSequenceAscSortResultU16(MObject *proc,unsigned short *data_out,int *index_out) {SequenceAscSortResult(U16,Asc,proc,data_out,index_out);}
void mSequenceAscSortResultS8( MObject *proc,          char *data_out,int *index_out) {SequenceAscSortResult(S8, Asc,proc,data_out,index_out);}
void mSequenceAscSortResultU8( MObject *proc, unsigned char *data_out,int *index_out) {SequenceAscSortResult(U8, Asc,proc,data_out,index_out);}

void mSequenceDescSortResultD64(MObject *proc,        double *data_out,int *index_out) {SequenceAscSortResult(D64,Desc,proc,data_out,index_out);}
void mSequenceDescSortResultF32(MObject *proc,         float *data_out,int *index_out) {SequenceAscSortResult(F32,Desc,proc,data_out,index_out);}
void mSequenceDescSortResultS32(MObject *proc,           int *data_out,int *index_out) {SequenceAscSortResult(S32,Desc,proc,data_out,index_out);}
void mSequenceDescSortResultU32(MObject *proc,  unsigned int *data_out,int *index_out) {SequenceAscSortResult(U32,Desc,proc,data_out,index_out);}
void mSequenceDescSortResultS16(MObject *proc,         short *data_out,int *index_out) {SequenceAscSortResult(S16,Desc,proc,data_out,index_out);}
void mSequenceDescSortResultU16(MObject *proc,unsigned short *data_out,int *index_out) {SequenceAscSortResult(U16,Desc,proc,data_out,index_out);}
void mSequenceDescSortResultS8( MObject *proc,          char *data_out,int *index_out) {SequenceAscSortResult(S8, Desc,proc,data_out,index_out);}
void mSequenceDescSortResultU8( MObject *proc, unsigned char *data_out,int *index_out) {SequenceAscSortResult(U8, Desc,proc,data_out,index_out);}

#define mSequenceMinSubsetResultD64 mSequenceAscSortResultD64
#define mSequenceMinSubsetResultF32 mSequenceAscSortResultF32
#define mSequenceMinSubsetResultS32 mSequenceAscSortResultS32
#define mSequenceMinSubsetResultU32 mSequenceAscSortResultU32
#define mSequenceMinSubsetResultS16 mSequenceAscSortResultS16
#define mSequenceMinSubsetResultU16 mSequenceAscSortResultU16
#define mSequenceMinSubsetResultS8  mSequenceAscSortResultS8
#define mSequenceMinSubsetResultU8  mSequenceAscSortResultU8

#define mSequenceMaxSubsetResultD64 mSequenceDescSortResultD64
#define mSequenceMaxSubsetResultF32 mSequenceDescSortResultF32
#define mSequenceMaxSubsetResultS32 mSequenceDescSortResultS32
#define mSequenceMaxSubsetResultU32 mSequenceDescSortResultU32
#define mSequenceMaxSubsetResultS16 mSequenceDescSortResultS16
#define mSequenceMaxSubsetResultU16 mSequenceDescSortResultU16
#define mSequenceMaxSubsetResultS8  mSequenceDescSortResultS8
#define mSequenceMaxSubsetResultU8  mSequenceDescSortResultU8
 
#define MinSubset(Type,Data_in,Index_in,Num_in,Data_out,Index_out,Num_out,Limit) {\
    Type Limit=0;\
    int I;\
    MObject *Proc;\
    Proc = mObjectCreate(NULL);\
    if((Index_in == NULL)||(Index_out==NULL))\
    {\
        for(I=0;I<Num_in;I++)\
           Limit = mSequenceMinSubset##Type(Proc,Data_in[I],I,NULL,NULL,Num_out);\
    }\
    else\
    {\
        for(I=0;I<Num_in;I++)\
           Limit = mSequenceMinSubset##Type(Proc,Data_in[I],Index_in[I],NULL,NULL,Num_out);\
    }\
    mSequenceMinSubsetResult##Type(Proc,Data_out,Index_out);\
    mObjectRelease(Proc);\
    return Limit;\
}
    
        double mMinSubsetD64(        double *data_in,int *index_in,int num_in,        double *data_out,int *index_out,int num_out) MinSubset(D64,data_in,index_in,num_in,data_out,index_out,num_out,limit)
         float mMinSubsetF32(         float *data_in,int *index_in,int num_in,         float *data_out,int *index_out,int num_out) MinSubset(F32,data_in,index_in,num_in,data_out,index_out,num_out,limit)
           int mMinSubsetS32(           int *data_in,int *index_in,int num_in,           int *data_out,int *index_out,int num_out) MinSubset(S32,data_in,index_in,num_in,data_out,index_out,num_out,limit)
  unsigned int mMinSubsetU32(  unsigned int *data_in,int *index_in,int num_in,  unsigned int *data_out,int *index_out,int num_out) MinSubset(U32,data_in,index_in,num_in,data_out,index_out,num_out,limit)
         short mMinSubsetS16(         short *data_in,int *index_in,int num_in,         short *data_out,int *index_out,int num_out) MinSubset(S16,data_in,index_in,num_in,data_out,index_out,num_out,limit)
unsigned short mMinSubsetU16(unsigned short *data_in,int *index_in,int num_in,unsigned short *data_out,int *index_out,int num_out) MinSubset(U16,data_in,index_in,num_in,data_out,index_out,num_out,limit)
          char mMinSubsetS8(           char *data_in,int *index_in,int num_in,          char *data_out,int *index_out,int num_out) MinSubset(S8 ,data_in,index_in,num_in,data_out,index_out,num_out,limit)
 unsigned char mMinSubsetU8(  unsigned char *data_in,int *index_in,int num_in, unsigned char *data_out,int *index_out,int num_out) MinSubset(U8 ,data_in,index_in,num_in,data_out,index_out,num_out,limit)

#define MaxSubset(Type,Data_in,Index_in,Num_in,Data_out,Index_out,Num_out,Limit) {\
    Type Limit=0;\
    int I;\
    MObject *Proc;\
    Proc = mObjectCreate(NULL);\
    if((Index_in == NULL)||(Index_out==NULL))\
    {\
        for(I=0;I<Num_in;I++)\
           Limit = mSequenceMaxSubset##Type(Proc,Data_in[I],I,NULL,NULL,Num_out);\
    }\
    else\
    {\
        for(I=0;I<Num_in;I++)\
           Limit = mSequenceMaxSubset##Type(Proc,Data_in[I],Index_in[I],NULL,NULL,Num_out);\
    }\
    mSequenceMaxSubsetResult##Type(Proc,Data_out,Index_out);\
    mObjectRelease(Proc);\
    return Limit;\
}

        double mMaxSubsetD64(        double *data_in,int *index_in,int num_in,        double *data_out,int *index_out,int num_out) MaxSubset(D64,data_in,index_in,num_in,data_out,index_out,num_out,limit)
         float mMaxSubsetF32(         float *data_in,int *index_in,int num_in,         float *data_out,int *index_out,int num_out) MaxSubset(F32,data_in,index_in,num_in,data_out,index_out,num_out,limit)
           int mMaxSubsetS32(           int *data_in,int *index_in,int num_in,           int *data_out,int *index_out,int num_out) MaxSubset(S32,data_in,index_in,num_in,data_out,index_out,num_out,limit)
  unsigned int mMaxSubsetU32(  unsigned int *data_in,int *index_in,int num_in,  unsigned int *data_out,int *index_out,int num_out) MaxSubset(U32,data_in,index_in,num_in,data_out,index_out,num_out,limit)
         short mMaxSubsetS16(         short *data_in,int *index_in,int num_in,         short *data_out,int *index_out,int num_out) MaxSubset(S16,data_in,index_in,num_in,data_out,index_out,num_out,limit)
unsigned short mMaxSubsetU16(unsigned short *data_in,int *index_in,int num_in,unsigned short *data_out,int *index_out,int num_out) MaxSubset(U16,data_in,index_in,num_in,data_out,index_out,num_out,limit)
          char mMaxSubsetS8(           char *data_in,int *index_in,int num_in,          char *data_out,int *index_out,int num_out) MaxSubset(S8 ,data_in,index_in,num_in,data_out,index_out,num_out,limit)
 unsigned char mMaxSubsetU8(  unsigned char *data_in,int *index_in,int num_in, unsigned char *data_out,int *index_out,int num_out) MaxSubset(U8 ,data_in,index_in,num_in,data_out,index_out,num_out,limit)

