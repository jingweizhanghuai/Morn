/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_math.h"

struct HandleVectorCreate
{
    MVector *vec;
    int size;
    float *memory;
};
void endVectorCreate(void *info)
{
    struct HandleVectorCreate *handle = (struct HandleVectorCreate *)info;
    mException((handle->vec==NULL),EXIT,"invalid vector");
    if(handle->memory != NULL) mFree(handle->memory);
    mFree(handle->vec);
}
#define HASH_VectorCreate 0xfc0b887c
MVector *mVectorCreate(int size,float *data)
{
    MVector *vec;
    vec = (MVector *)mMalloc(sizeof(MVector));
    memset(vec,0,sizeof(MVector));   
    
    if(size<0) size = 0;
    vec->size = size;
    
    MHandle *hdl; ObjectHandle(vec,VectorCreate,hdl);
    struct HandleVectorCreate *handle = (struct HandleVectorCreate *)(hdl->handle);
    handle->vec = vec;
    
    if(size==0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
    }
    else if(INVALID_POINTER(data))
    {
        handle->memory = (float *)mMalloc(size*sizeof(float));
        handle->size = size;
        vec->data = handle->memory;
    }
    else
        vec->data = data;
    
    return vec;
}
 
void mVectorRelease(MVector *vec)
{   
    mException(INVALID_POINTER(vec),EXIT,"invalid input");
    
    if(!INVALID_POINTER(vec->handle))
        mHandleRelease(vec->handle);
}

void mVectorRedefine(MVector *vec,int size,float *data)
{
    mException(INVALID_POINTER(vec),EXIT,"invalid input");
    
    if(size <= 0) size = vec->size;
    if(size!=vec->size)mHandleReset(vec->handle);
    int same_size = (size <= vec->size);
    int reuse = (data==vec->data);
    int flag = (vec->size >0);
    
    vec->size=size;
    
    if(same_size&&reuse) return;
    struct HandleVectorCreate *handle = (struct HandleVectorCreate *)(((MHandle *)(vec->handle->data[0]))->handle);
    if(same_size&&(data==NULL)&&(handle->size>0)) return;
    mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");
    
    handle->size=0;
    if(size <= 0) {mException((data!=NULL)&&(!reuse),EXIT,"invalid input"); vec->data = NULL; return;}
    
    if(reuse) data=NULL;
    if(data!=NULL){vec->data = data; return;}
    
    if(size>handle->size)
    {
        if(handle->memory!=NULL) mFree(handle->memory);
        handle->memory = (float *)mMalloc(size*sizeof(float));
        handle->size = size;
        vec->data = handle->memory;
    }
}

void PrintMat(MMatrix *mat)
{
    int i,j;
    for(j=0;j<mat->row;j++)
    {
        for(i=0;i<mat->col;i++)
            printf("%f\t",mat->data[j][i]);
        printf("\n");
    }
}

struct HandleMatrixCreate
{
    MMatrix *mat;
    int row;
    int col;
    float **index;
    MMemory *memory;
};
void endMatrixCreate(void *info)
{
    struct HandleMatrixCreate *handle = (struct HandleMatrixCreate *)info;
    mException((handle->mat == NULL),EXIT,"invalid matrix");
    
    if(handle->index != NULL) mFree(handle->index);
    if(handle->memory!= NULL) mMemoryRelease(handle->memory);
    
    mFree(handle->mat);
}
#define HASH_MatrixCreate 0xe48fad76
MMatrix *mMatrixCreate(int row,int col,float **data)
{
    MMatrix *mat = (MMatrix *)mMalloc(sizeof(MMatrix));
    memset(mat,0,sizeof(MMatrix));
    
    if(col <0) col = 0;
    if(row <0) row = 0;
    
    mat->col = col;
    mat->row = row;
    
    MHandle *hdl; ObjectHandle(mat,MatrixCreate,hdl);
    struct HandleMatrixCreate *handle = (struct HandleMatrixCreate *)(hdl->handle);
    handle->mat = mat;
    
    if(row == 0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        return mat;
    }
    
    if(!INVALID_POINTER(data))
    {
        mat->data = data;
        return mat;
    }
    
    handle->index = (float **)mMalloc(row*sizeof(float *));
    handle->row = row;

    if(col == 0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        memset(handle->index,0,row*sizeof(float *));
    }
    else 
    {
        if(handle->memory == NULL)
            handle->memory = mMemoryCreate(row,col*sizeof(float));
        mMemoryIndex(handle->memory,row,col*sizeof(float),(void **)(handle->index));
        
        handle->col = col;
    }
    mat->data = handle->index;
    
    return mat;
}

void mMatrixRelease(MMatrix *mat)
{
    mException(INVALID_POINTER(mat),EXIT,"invalid input");
    
    if(!INVALID_POINTER(mat->handle))
        mHandleRelease(mat->handle);
}

void mMatrixRedefine(MMatrix *mat,int row,int col,float **data)
{
    mException((INVALID_POINTER(mat)),EXIT,"invalid input");
    
    if(row<=0) row = mat->row;
    if(col<=0) col = mat->col;
    if((row!=mat->row)||(col!=mat->col)) mHandleReset(mat->handle);
    
    int same_size=((row<=mat->row)&&(col<=mat->col));
    int reuse = (data==mat->data);
    int flag=(mat->row)&&(mat->col);
    
    mat->row = row;
    mat->col = col;
    if(same_size&&reuse) return;
    struct HandleMatrixCreate *handle= (struct HandleMatrixCreate *)(((MHandle *)(mat->handle->data[0]))->handle);
    if(same_size&&(data==NULL)&&(handle->col>0)) return;
    mException(reuse&&flag&&(handle->col==0),EXIT,"invalid redefine");
    
    handle->col=0;
    if((row == 0)||(col==0)) {mException((data!=NULL)&&(!reuse),EXIT,"invalid input"); mat->data=NULL; return;}
    
    if(reuse) data=NULL;
    if(row>handle->row)
    {
        if(handle->index != NULL)
            mFree(handle->index);
        handle->index = NULL;
    }
    if(handle->index == NULL)
    {
        handle->index = (float **)mMalloc(row*sizeof(float *));
        handle->row = row;
    }
    mat->data = handle->index;
    
    if(data!=NULL) {memcpy(handle->index,data,row*sizeof(float *));return;}
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(row,col*sizeof(float));
    mMemoryIndex(handle->memory,row,col*sizeof(float),(void **)(handle->index));
    handle->col = col;
}

void mUnitMatrix(MMatrix *mat,int size)
{
    if(size<0)
    {
        size = mat->row;
        mException(size!=mat->col,EXIT,"invalid input");
    }
    else mMatrixRedefine(mat,size,size,mat->data);
       
    int i;
    #pragma omp parallel for
    for(i=0;i<size;i++)
    {
        memset(mat->data[i],0,size*sizeof(float));
        mat->data[i][i] = 1.0f;
    }
}

void mMatrixTranspose(MMatrix *mat,MMatrix *dst)
{
    int i,j;
    int dst_col,dst_row;
    MMatrix *p;
    
    dst_col = mat->row;
    dst_row = mat->col;
    
    mException((INVALID_MAT(mat)),EXIT,"invalid input");
    
    p = dst;
    if((INVALID_POINTER(dst))||(dst==mat)) dst = mMatrixCreate(dst_row,dst_col,NULL);
    else mMatrixRedefine(dst,dst_row,dst_col,dst->data);
    
    for(i=0;i<dst_row-8;i=i+8)
    {
        for(j=0;j<dst_col;j++)
        {
            dst->data[i][j] = mat->data[j][i]; 
            dst->data[i+1][j] = mat->data[j][i+1]; 
            dst->data[i+2][j] = mat->data[j][i+2]; 
            dst->data[i+3][j] = mat->data[j][i+3]; 
            dst->data[i+4][j] = mat->data[j][i+4]; 
            dst->data[i+5][j] = mat->data[j][i+5]; 
            dst->data[i+6][j] = mat->data[j][i+6]; 
            dst->data[i+7][j] = mat->data[j][i+7];
        }
    }
    // #pragma omp parallel for
    for(;i<dst_row;i++)
        for(j=0;j<dst_col;j++)
            dst->data[i][j] = mat->data[j][i];
        
    if(p!=dst)
    {
        mMatrixExchange(mat,dst);
        mMatrixRelease(dst);
    }
}

void VectorAdd(float *vec1,float *vec2,float *dst,int num)
{
    int i;
    for(i=0;i<num;i++)
        dst[i] = vec1[i]+vec2[i];
}

void mVectorAdd(MVector *vec1,MVector *vec2,MVector *dst)
{
    int i;
    mException((INVALID_VEC(vec1)||INVALID_VEC(vec2)||(vec1->size !=vec2->size)),EXIT,"invalid input");

    if(INVALID_POINTER(dst)) dst = vec1;
    else mVectorRedefine(dst,vec1->size,dst->data);
    
    for(i=0;i<vec1->size;i++)
        dst->data[i] = vec1->data[i] + vec2->data[i];
}

void mMatrixAdd(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    int j;
    mException(INVALID_MAT(mat1)||INVALID_MAT(mat2),EXIT,"invalid input");
    mException((mat1->row!=mat2->row)||(mat1->col!=mat2->col),EXIT,"invalid input");
    if(INVALID_POINTER(dst)) dst = mat1;
    else mMatrixRedefine(dst,mat1->row,mat1->col,dst->data);
    #pragma omp parallel for
    for(j=0;j<dst->row;j++)
        for(int i=0;i<dst->col;i++)
            dst->data[j][i] = mat1->data[j][i]+mat2->data[j][i];
}

void mMatrixSub(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    int j;
    mException(INVALID_MAT(mat1)||INVALID_MAT(mat2),EXIT,"invalid input");
    mException((mat1->row!=mat2->row)||(mat1->col!=mat2->col),EXIT,"invalid input");
    if(INVALID_POINTER(dst)) dst = mat1;
    else mMatrixRedefine(dst,mat1->row,mat1->col,dst->data);
    #pragma omp parallel for
    for(j=0;j<dst->row;j++)
        for(int i=0;i<dst->col;i++)
            dst->data[j][i] = mat1->data[j][i]+mat2->data[j][i];
}

float VectorMul(float *vec1,float *vec2,int num)
{
    int i;
    float sum;
    
    sum = 0.0f;
    for(i=0;i<num;i++)
        sum = sum + vec1[i]*vec2[i];
    
    return sum;
}

float mVectorMul(MVector *vec1,MVector *vec2)
{
    int i;
    float result;
    mException((INVALID_VEC(vec1)||INVALID_VEC(vec2)||(vec1->size !=vec2->size)),EXIT,"invalid input");

    result = 0.0f;
    for(i=0;i<vec1->size;i++)
        result = result + vec1->data[i]*vec2->data[i];

    return result;
}

void VectorScalarMul(float *vec1,float *vec2,float *dst,int num)
{
    int i;
    for(i=0;i<num;i++)
        dst[i] = vec1[i]*vec2[i];
}

void mVectorScalarMul(MVector *vec1,MVector *vec2,MVector *dst)
{
    int i;
    mException((INVALID_VEC(vec1)||INVALID_VEC(vec2)||(vec1->size !=vec2->size)),EXIT,"invalid input");

    if(INVALID_POINTER(dst)) dst = vec1;
    else mVectorRedefine(dst,vec1->size,dst->data);
    
    for(i=0;i<vec1->size;i++)
        dst->data[i] = vec1->data[i] * vec2->data[i];
}

void MatrixVectorMul(MMatrix *mat,float *vec,float *dst)
{
    int i,j;
    int num_in,num_out;
    
    num_in = mat->col;
    num_out = mat->row;
    memset(dst,0,num_out*sizeof(float));
    for(i=0;i<num_out;i++)
        for(j=0;j<num_in;j++)
            dst[i] = dst[i] + vec[j]*mat->data[i][j];
}

void mMatrixVectorMul(MMatrix *mat,MVector *vec,MVector *dst)
{
    int i,j;
    int num_in;
    int num_out;
    
    MVector *p;
    
    p = dst;
    
    mException((INVALID_MAT(mat)||INVALID_VEC(vec)),EXIT,"invalid input");
    
    num_in = mat->col;
    mException((vec->size != num_in),EXIT,"invalid input");
    
    num_out = mat->row;
    if(INVALID_POINTER(dst)||(dst == vec)) dst = mVectorCreate(num_out,NULL);
    else mVectorRedefine(dst,num_out,dst->data);
       
    
    for(i=0;i<num_out;i++)
    {
        dst->data[i] = 0.0f;
        for(j=0;j<num_in;j++)
            dst->data[i] = dst->data[i] + vec->data[j]*mat->data[i][j];
    }
    
    if(p!=dst)
    {
        mVectorExchange(dst,vec);
        mVectorRelease(dst);
    }
}

void VectorMatrixMul(float *vec,MMatrix *mat,float *dst)
{
    int i,j;
    int num_in,num_out;
    
    num_in = mat->row;
    num_out = mat->col;
    
    memset(dst,0,num_out*sizeof(float));
    for(j=0;j<num_in;j++)
        for(i=0;i<num_out;i++)
            dst[i] = dst[i] + vec[j]*mat->data[j][i];
}

void mVectorMatrixMul(MVector *vec,MMatrix *mat,MVector *dst)
{
    int i,j;
    int num_in;
    int num_out;
    
    MVector *p;
    
    mException(((INVALID_MAT(mat))||(INVALID_VEC(vec))),EXIT,"invalid input");

    p = dst;
    
    num_in = mat->row;
    mException((vec->size != num_in),EXIT,"invalid input");
    
    num_out = mat->col;
    if(INVALID_POINTER(dst)||(dst == vec)) dst = mVectorCreate(num_out,NULL);
    else mVectorRedefine(dst,num_out,dst->data);
    
    for(i=0;i<num_out;i++)
    {
        dst->data[i] = 0.0f;
        for(j=0;j<num_in;j++)
            dst->data[i] = dst->data[i] + vec->data[j]*mat->data[j][i];
    }
    
    if(p!=dst)
    {
        mVectorExchange(vec,dst);
        mVectorRelease(dst);
    }
}

void mMatrixScalarMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    int i,j;
    
    mException((INVALID_MAT(mat1)||INVALID_MAT(mat2)),EXIT,"invalid input");
    mException((mat1->row != mat2->row)||(mat1->col != mat2->col),EXIT,"invalid input");

    if(INVALID_POINTER(dst)) dst = mat1;
    else mMatrixRedefine(dst,mat1->row,mat1->col,dst->data);
    
    for(j=0;j<mat1->row;j++)
        for(i=0;i<mat1->col;i++)
            dst->data[j][i] = mat1->data[j][i]*mat2->data[j][i];
}
    
/*
void mMatrixMul0(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    int i,j,k;
    int dst_col,dst_row;
    int num;
    MMatrix *p;
    float data;
    float *psrc,*pdst;
    
    mException((INVALID_MAT(mat1))||(INVALID_MAT(mat2)),EXIT,"invalid input");
    mException((mat1->col != mat2->row),EXIT,"invalid operate");
    num = mat1->col;
    dst_col = mat2->col;
    dst_row = mat1->row;
    
    p = dst;
    if((INVALID_POINTER(dst))||(dst==mat1)||(dst==mat2)) dst = mMatrixCreate(dst_row,dst_col,NULL);
    else mMatrixRedefine(dst,dst_row,dst_col,dst->data);
    
    for(j=0;j<dst_row;j++)
    {
        pdst = dst->data[j];
        {
            data = mat1->data[j][0];
            psrc = mat2->data[0];
            for(k=0;k<dst_col-8;k=k+8)
            {
                pdst[k] = data*psrc[k];
                pdst[k+1] = data*psrc[k+1];
                pdst[k+2] = data*psrc[k+2];
                pdst[k+3] = data*psrc[k+3];
                pdst[k+4] = data*psrc[k+4];
                pdst[k+5] = data*psrc[k+5];
                pdst[k+6] = data*psrc[k+6];
                pdst[k+7] = data*psrc[k+7];
            }
            for(;k<dst_col;k++)
                pdst[k] = data*psrc[k];
        }
        for(i=1;i<num;i++)
        {
            data = mat1->data[j][i];
            psrc = mat2->data[i];
            for(k=0;k<dst_col-8;k=k+8)
            {
                pdst[k] += data*psrc[k];
                pdst[k+1] += data*psrc[k+1];
                pdst[k+2] += data*psrc[k+2];
                pdst[k+3] += data*psrc[k+3];
                pdst[k+4] += data*psrc[k+4];
                pdst[k+5] += data*psrc[k+5];
                pdst[k+6] += data*psrc[k+6];
                pdst[k+7] += data*psrc[k+7];
            }
            for(;k<dst_col;k++)
                pdst[k] += data*psrc[k];
        }
    }
    
    if(p != dst)
    {
        if(p == mat2)
        {
            mMatrixExchange(mat2,dst);
            mMatrixRelease(dst);
        }
        else
        {
            mMatrixExchange(mat1,dst);
            mMatrixRelease(dst);
        }
    }
}
*/

void mMatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    mException((INVALID_MAT(mat1))||(INVALID_MAT(mat2)),EXIT,"invalid input");
    mException((mat1->col != mat2->row),EXIT,"invalid operate");
    int num = mat1->col;
    int dst_col = mat2->col;
    int dst_row = mat1->row;

    MMatrix *p = dst;
    if((INVALID_POINTER(dst))||(dst==mat1)||(dst==mat2)) dst = mMatrixCreate(dst_row,dst_col,NULL);
    else mMatrixRedefine(dst,dst_row,dst_col,dst->data);

    int flag = num&0x03; if(flag==0) flag = 4;
    
    int i,j,k;
    float data1,data2,data3,data4;
    float *psrc1,*psrc2,*psrc3,*psrc4;
    float *pdst;

    #pragma omp parallel for
    for(j=0;j<dst_row;j++)
    {
        pdst = dst->data[j];
        data1 = mat1->data[j][0];
        psrc1 = mat2->data[0];
        for(k=0;k<dst_col;k++)
            pdst[k] = data1*psrc1[k];
        for(i=1;i<flag;i++)
        {
            data1 = mat1->data[j][i];
            psrc1 = mat2->data[i];
            for(k=0;k<dst_col;k++)
                pdst[k] += data1*psrc1[k];
        }
        for(i=flag;i<num;i=i+4)
        {
            data1 = mat1->data[j][i];
            psrc1 = mat2->data[i];
            data2 = mat1->data[j][i+1];
            psrc2 = mat2->data[i+1];
            data3 = mat1->data[j][i+2];
            psrc3 = mat2->data[i+2];
            data4 = mat1->data[j][i+3];
            psrc4 = mat2->data[i+3];
            for(k=0;k<dst_col;k++)
                pdst[k] += data1*psrc1[k]+data2*psrc2[k]+data3*psrc3[k]+data4*psrc4[k];
        }
    }

    if(p != dst)
    {
        if(p == mat2)
        {
            mMatrixExchange(mat2,dst);
            mMatrixRelease(dst);
        }
        else
        {
            mMatrixExchange(mat1,dst);
            mMatrixRelease(dst);
        }
    }
}

float mMatrixDetValue(MMatrix *mat)
{
    int i,j,k;
    int num;
    double *buff;
    double w;
    double value;
    double **data;
    
    mException((INVALID_MAT(mat)),EXIT,"invalid input");
    num = mat->row;
    mException((mat->col != num),EXIT,"invalid operate");  
    
    if(num == 1)
        return mat->data[0][0];
    if(num == 2)
    {
        value = mat->data[0][0]*mat->data[1][1]-mat->data[1][0]*mat->data[0][1];
        return value;
    }   
    if(num == 3)
    {
        value = mat->data[0][0]*(mat->data[1][1]*mat->data[2][2]-mat->data[1][2]*mat->data[2][1])
              + mat->data[0][1]*(mat->data[1][2]*mat->data[2][0]-mat->data[1][0]*mat->data[2][2])
              + mat->data[0][2]*(mat->data[1][0]*mat->data[2][1]-mat->data[1][1]*mat->data[2][0]);
        return value;
    }
    
    data = (double **)mMalloc((num+1)*sizeof(double *));
    data[0] = (double *)mMalloc(num*num*sizeof(double));
    for(j=0;j<num;j++)
    {
        for(i=0;i<num;i++)
            data[j][i] = (double)(mat->data[j][i]);
        data[j+1] = data[j]+num;
    }
    
    // data = mMatrixCreate(num,num,NULL);
    // for(j=0;j<num;j++)
        // memcpy(data->data[j],mat->data[j],num*sizeof(float));
    
    // PrintMat(data);

    // #define DATA(y,x) (data[y][x])
    
    value = 1.0;
    // 高斯消元
    for(k=0;k<num;k++)
    {
        if(data[k][k]==0)
        {
            for(j=k+1;j<num;j++)
                if(data[j][k]!=0)
                {
                    buff = data[k];
                    data[k] = data[j];
                    data[j] = buff;
                    
                    value = 0-value;
                    break;
                }
                
            // 如果无解则返回0
            if(j==num)
            {
                mFree(data);
                mFree(data[0]);
                return 0.0f;
            }
        }
        
        value = value * data[k][k];
        
        for(j=k+1;j<num;j++)
            // if(data[j][k]!=0)
            {
                w = data[j][k]/data[k][k];
                for(i=k+1;i<num;i++)
                    data[j][i] = data[j][i]-w*data[k][i];                
            }
        
        // printf("aaaaaaaaaa %d:%f\n",k,data[k][k]);
        // PrintMat(data);
    }

    mFree(data[0]);
    mFree(data);
    return (float)value;
}

int mMatrixInverse(MMatrix *mat,MMatrix *inv)
{
    int i,j,k;
    int num;
    double *buff;
    double w;
    double **data;
    
    // MMatrix *data;
    MMatrix *p;

    mException((INVALID_MAT(mat)),EXIT,"invalid input");
    num = mat->row;
    mException((mat->col != num),EXIT,"invalid operate");
    
    p = inv;
    if((INVALID_POINTER(inv))||(inv == mat)) inv = mMatrixCreate(num,num,NULL);
    else mMatrixRedefine(inv,num,num,inv->data);
    
    data = (double **)mMalloc((num+1)*sizeof(double *));
    data[0] = (double *)mMalloc((num+num)*num*sizeof(double));
    for(j=0;j<num;j++)
    {
        for(i=0;i<num;i++)
            data[j][i] = (double)(mat->data[j][i]);
        memset(data[j]+num,0,num*sizeof(double));
        data[j][num+j] = -1.0;
        data[j+1] = data[j]+num+num;
    }
    
    // data = mMatrixCreate(num,num+num,NULL);
    // for(j=0;j<num;j++)
    // {
        // memcpy(data->data[j],mat->data[j],num*sizeof(float));
        // memset(data->data[j] + num,0,num*sizeof(float));
        // data->data[j][num+j] = -1.0;
    // }
    
    // PrintMat(data);
    // #define DATA(y,x) (data->data[y][x])
    // 高斯消元
    for(k=0;k<num;k++)
    {
        if(data[k][k]==0)
        {
            for(j=k+1;j<num;j++)
                if(data[j][k]!=0)
                {
                    buff = data[k];
                    data[k] = data[j];
                    data[j] = buff;
                    break;
                }
                
            // 如果无解则返回0
            if(j==num)
            {
                mFree(data);
                mFree(data[0]);
                if(p!=inv)
                    mMatrixRelease(inv);
                return 0.0;
            }
        }
        
        for(j=k+1;j<num;j++)
            if(data[j][k]!=0)
            {
                w = data[j][k]/data[k][k];
                // data[j][k]=0;
                for(i=k+1;i<num+num;i++)
                    data[j][i] = data[j][i]-w*data[k][i];                
            }
        
        // printf("aaaaaaaaaa %d\n",k);
        // PrintMat(data);
    }
    
    for(k=0;k<num;k++)
    {
        for(j=num-1;j>=0;j--)
        {
            data[j][num+k] = 0-data[j][num+k];
            for(i=num-1;i>j;i--)
                data[j][num+k] = data[j][num+k] - data[j][i]*data[i][num+k];
            
            data[j][num+k] = data[j][num+k]/data[j][j];
            inv->data[j][k] = (float)data[j][num+k];
        }
    }
    mFree(data[0]);
    mFree(data);
    
    if(p!=inv)
    {
        mMatrixExchange(inv,mat);
        mMatrixRelease(inv);
    }
    
    return 1;
}


int mLinearEquation(MMatrix *mat,float *answer)
{
    int i,j,k;
    int num;    
    float *buff;
    float w;
    MMatrix *data;
    
    mException((INVALID_MAT(mat))||(INVALID_POINTER(answer)),EXIT,"invalid input");
    mException(((mat->col - mat->row)!=1),EXIT,"invalid operate");
    num = mat->row;
    
    if(num == 1)
    {
        mException((mat->data[0][0]==0.0f),EXIT,"invalid input");
        *answer = (0.0f-mat->data[0][1])/mat->data[0][0];
        return 1;
    }
    
    data = mMatrixCreate(num,num+1,NULL);
    for(j=0;j<num;j++)
        memcpy(data->data[j],mat->data[j],(num+1)*sizeof(float));
        
    #define DATA(y,x) (data->data[y][x])
        
    // 高斯消元
    for(k=0;k<num;k++)
    {
        if(DATA(k,k)==0)
        {
            for(j=k+1;j<num;j++)
                if(DATA(j,k)!=0)
                {
                    buff = data->data[k];
                    data->data[k] = data->data[j];
                    data->data[j] = buff;
                    break;
                }
                
            // 如果无解则返回0
            if(j==num)
            {
                mMatrixRelease(data);
                return 0;
            }
        }
        
        for(j=k+1;j<num;j++)
            if(DATA(j,k)!=0)
            {
                w = DATA(j,k)/DATA(k,k);
                DATA(j,k)=0;
                for(i=k+1;i<num+1;i++)
                    DATA(j,i) = DATA(j,i)-w*DATA(k,i);                
            }
    }
        
    // 答案求解
    for(j=num-1;j>=0;j--)
    {
        answer[j] = 0-DATA(j,num);
        for(i=num-1;i>j;i--)
            answer[j] = answer[j] - DATA(j,i)*answer[i];
        answer[j] = answer[j]/DATA(j,j);
    }

    mMatrixRelease(data);
    return 1;        
}
