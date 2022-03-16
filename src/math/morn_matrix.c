/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_math.h"

struct HandleVectorCreate
{
    MVector *vec;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    
    int size;
    MMemory *memory;
};
void endVectorCreate(struct HandleVectorCreate *handle)
{
    mException((handle->vec==NULL),EXIT,"invalid vector");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->memory != NULL) mMemoryRelease(handle->memory);
    memset(handle->vec,0,sizeof(MVector));
    // mFree(((MList **)(handle->vec))-1);
}
#define HASH_VectorCreate 0xfc0b887c
MVector *VectorCreate(int size,float *data)
{
    MVector *vec = (MVector *)ObjectAlloc(sizeof(MVector));
    MHandle *hdl=mHandle(vec,VectorCreate);
    struct HandleVectorCreate *handle = (struct HandleVectorCreate *)(hdl->handle);
    handle->vec = vec;
    
    if(size<0) size = 0;
    vec->size = size;
    if(size==0)
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
    }
    else if(INVALID_POINTER(data))
    {
        handle->memory = mMemoryCreate(1,size*sizeof(float),MORN_HOST);
        handle->size = size;
        vec->data = handle->memory->data[0];
    }
    else
        vec->data = data;

    mPropertyFunction(vec,"device",mornMemoryDevice,handle->memory);
    
    return vec;
}
 
void mVectorRelease(MVector *vec)
{
    ObjectFree(vec);
}

void VectorRedefine(MVector *vec,int size,float *data)
{
    mException(INVALID_POINTER(vec),EXIT,"invalid input");
    
    if(size <= 0) size = vec->size;
    if(size!=vec->size)mHandleReset(vec);
    int same_size = (size <= vec->size);
    int reuse = (data==vec->data);
    int flag = (vec->size >0);
    
    vec->size=size;
    if(same_size&&reuse) return;

    struct HandleVectorCreate *handle = (struct HandleVectorCreate *)(ObjHandle(vec,0)->handle);
    if(same_size&&(data==NULL)&&(handle->size>0)) return;
    mException(reuse&&flag&&(handle->size==0),EXIT,"invalid redefine");
    
    handle->size=0;
    if(size <= 0) {mException((data!=NULL)&&(!reuse),EXIT,"invalid input"); vec->data = NULL; return;}
    
    if(reuse) data=NULL;
    if(data!=NULL){vec->data = data; return;}
    
    if(size>handle->size)
    {
        handle->size = size;
        if(handle->memory==NULL)
        {
            handle->memory = mMemoryCreate(1,size*sizeof(float),MORN_HOST);
            mPropertyFunction(vec,"device",mornMemoryDevice,handle->memory);
        }
        else mMemoryRedefine(handle->memory,1,size*sizeof(float),DFLT);
        vec->data = handle->memory->data[0];
    }
}

void _PrintMat(char *name,MMatrix *mat)
{
    printf("%s:row=%d,col=%d:\n",name,mat->row,mat->col);
    int i,j;
    for(j=0;j<mat->row;j++)
    {
        for(i=0;i<mat->col;i++)
            printf("\t%f",mat->data[j][i]);
        printf("\n");
    }
}

struct HandleMatrixCreate
{
    MMatrix *mat;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    
    int row;
    int col;
    float **index;
    MMemory *memory;
};
void endMatrixCreate(struct HandleMatrixCreate *handle)
{
    mException((handle->mat == NULL),EXIT,"invalid matrix");
    if(handle->property!= NULL) mChainRelease(handle->property);
    if(handle->index   != NULL) mFree(handle->index);
    if(handle->memory  != NULL) mMemoryRelease(handle->memory);

    memset(handle->mat,0,sizeof(MMatrix));
    // mFree(((MList **)(handle->mat))-1);
}
#define HASH_MatrixCreate 0xe48fad76
MMatrix *MatrixCreate(int row,int col,float **data)
{
    MMatrix *mat = (MMatrix *)ObjectAlloc(sizeof(MMatrix));
    MHandle *hdl=mHandle(mat,MatrixCreate);
    struct HandleMatrixCreate *handle = (struct HandleMatrixCreate *)(hdl->handle);
    handle->mat = mat;
    
    if(col <0) {col = 0;} mat->col = col;
    if(row <0) {row = 0;} mat->row = row;
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
        if(handle->memory == NULL)handle->memory = mMemoryCreate(1,row*col*sizeof(float),MORN_HOST);
        mException(handle->memory->num!=1,EXIT,"invalid image memory");
        mMemoryIndex(handle->memory,row,col*sizeof(float),(void ***)(&(handle->index)),1);
        handle->col = col;
    }
    mat->data = handle->index;
    mPropertyFunction(mat,"device",mornMemoryDevice,handle->memory);
    
    return mat;
}

MMemoryBlock *mMatrixMemory(MMatrix *mat)
{
    int size = mat->row*mat->col*sizeof(float);
    float *data = &(mat->data[0][0]);
    
    struct HandleMatrixCreate *handle= (struct HandleMatrixCreate *)(ObjHandle(mat,0)->handle);
    if(handle->memory == NULL)
    {
        handle->memory = mMemoryCreate(1,size,MORN_HOST);
        mPropertyFunction(mat,"device",mornMemoryDevice,handle->memory);
    }
    MMemoryBlock *mem = handle->memory->data[0];
    if(mem->size!=size)
        mMemoryIndex(handle->memory,mat->row,mat->col*sizeof(float),(void ***)(&(handle->index)),1);
    // printf("handle->memory->data[0]
    
    if(mem->data!=data) memcpy(mem->data,data,size);
    
    return mem;
}

void mMatrixRelease(MMatrix *mat)
{
    ObjectFree(mat);
}

void MatrixRedefine(MMatrix *mat,int row,int col,float **data)
{
    mException((INVALID_POINTER(mat)),EXIT,"invalid input");
    
    if(row<=0) row = mat->row;
    if(col<=0) col = mat->col;
    if((row!=mat->row)||(col!=mat->col)) mHandleReset(mat);
    
    int same_size=((row<=mat->row)&&(col<=mat->col));
    int reuse = (data==mat->data);
    int flag=(mat->row)&&(mat->col);
    
    mat->row = row;
    mat->col = col;
    if(same_size&&reuse) return;
    
    struct HandleMatrixCreate *handle= (struct HandleMatrixCreate *)(ObjHandle(mat,0)->handle);
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
    
    if(!INVALID_POINTER(data)) {memcpy(handle->index,data,row*sizeof(float *));return;}
    
    if(handle->memory == NULL) 
    {
        handle->memory = mMemoryCreate(1,row*col*sizeof(float),MORN_HOST);
        mPropertyFunction(mat,"device",mornMemoryDevice,handle->memory);
    }
    else    mMemoryRedefine(handle->memory,1,row*col*sizeof(float),DFLT);
    mMemoryIndex(handle->memory,row,col*sizeof(float),(void ***)(&(handle->index)),1);
    handle->col = col;
}

void m_UnitMatrix(MMatrix *mat,int size)
{
    mException(mat==NULL,EXIT,"invalid input matrix");
    if(size<0)
    {
        size = mat->row;
        mException(size!=mat->col,EXIT,"invalid input");
    }
    else mMatrixRedefine(mat,size,size,mat->data);
       
    int i;
    // #pragma omp parallel for
    for(i=0;i<size;i++)
    {
        memset(mat->data[i],0,size*sizeof(float));
        mat->data[i][i] = 1.0f;
    }
}

void m_MatrixTranspose(MMatrix *mat,MMatrix *dst)
{
    mException((INVALID_MAT(mat)),EXIT,"invalid input");
    
    int i,j;
    int dst_col = mat->row;
    int dst_row = mat->col;
    
    MMatrix *p = dst;
    if((INVALID_POINTER(dst))||(dst==mat)) dst = mMatrixCreate(dst_row,dst_col,NULL);
    else mMatrixRedefine(dst,dst_row,dst_col,dst->data);

    float **s=mat->data;
    float **d=dst->data;

    int r=dst_row&0x03;
    int c=dst_col&0x03;
    for(j=0;j<r;j++)
    {
        for(i=0;i<c;i++) d[j][i]=s[i][j];
        for(i=c;i<dst_col;i+=4)
        {
            d[j][i]=s[i][j];d[j][i+1]=s[i+1][j];d[j][i+2]=s[i+2][j];d[j][i+3]=s[i+3][j];
        }
    }
    
    for(j=r;j<dst_row;j+=4)
    {
        for(i=0;i<c;i++)
        {
            d[j][i]=s[i][j];d[j+1][i]=s[i][j+1];d[j+2][i]=s[i][j+2];d[j+3][i]=s[i][j+3];
        }
        for(i=c;i<dst_col;i+=4)
        {
            d[j  ][i]=s[i][j  ];d[j  ][i+1]=s[i+1][j  ];d[j  ][i+2]=s[i+2][j  ];d[j  ][i+3]=s[i+3][j  ];
            d[j+1][i]=s[i][j+1];d[j+1][i+1]=s[i+1][j+1];d[j+1][i+2]=s[i+2][j+1];d[j+1][i+3]=s[i+3][j+1];
            d[j+2][i]=s[i][j+2];d[j+2][i+1]=s[i+1][j+2];d[j+2][i+2]=s[i+2][j+2];d[j+2][i+3]=s[i+3][j+2];
            d[j+3][i]=s[i][j+3];d[j+3][i+1]=s[i+1][j+3];d[j+3][i+2]=s[i+2][j+3];d[j+3][i+3]=s[i+3][j+3];
        }
    }

    if(p!=dst)
    {
        mObjectExchange(mat,dst,MMatrix);
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
    else mVectorRedefine(dst,vec1->size,dst->data,vec1->device);
    
    for(i=0;i<vec1->size;i++)
        dst->data[i] = vec1->data[i] + vec2->data[i];
}

void mMatrixAdd(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    int j;
    mException(INVALID_MAT(mat1)||INVALID_MAT(mat2),EXIT,"invalid input");
    mException((mat1->row!=mat2->row)||(mat1->col!=mat2->col),EXIT,"invalid input");
    if(INVALID_POINTER(dst)) dst = mat1;
    else mMatrixRedefine(dst,mat1->row,mat1->col,dst->data,DFLT);
    // #pragma omp parallel for
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
    else mMatrixRedefine(dst,mat1->row,mat1->col,dst->data,DFLT);
    // #pragma omp parallel for
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
    else mVectorRedefine(dst,vec1->size,dst->data,vec1->device);
    
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
    
    MVector *p= dst;
    
    mException((INVALID_MAT(mat)||INVALID_VEC(vec)),EXIT,"invalid input");
    
    num_in = mat->col;
    mException((vec->size != num_in),EXIT,"invalid input");
    
    num_out = mat->row;
    if(INVALID_POINTER(dst)||(dst == vec)) dst = mVectorCreate(num_out,NULL,vec->device);
    else mVectorRedefine(dst,num_out,dst->data,vec->device);
       
    
    for(i=0;i<num_out;i++)
    {
        dst->data[i] = 0.0f;
        for(j=0;j<num_in;j++)
            dst->data[i] = dst->data[i] + vec->data[j]*mat->data[i][j];
    }
    
    if(p!=dst)
    {
        mObjectExchange(dst,vec,MVector);
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
    if(INVALID_POINTER(dst)||(dst == vec)) dst = mVectorCreate(num_out,NULL,vec->device);
    else mVectorRedefine(dst,num_out,dst->data,vec->device);
    
    for(i=0;i<num_out;i++)
    {
        dst->data[i] = 0.0f;
        for(j=0;j<num_in;j++)
            dst->data[i] = dst->data[i] + vec->data[j]*mat->data[j][i];
    }
    
    if(p!=dst)
    {
        mObjectExchange(vec,dst,MVector);
        mVectorRelease(dst);
    }
}

void mMatrixScalar(MMatrix *src,MMatrix *dst,float k,float b)
{
    mException((src==NULL),EXIT,"invalid input");
    if(dst==NULL) dst=src;
    for(int j=0;j<src->row;j++)for(int i=0;i<src->col;i++)
        dst->data[j][i] = src->data[j][i]*k+b;
}

void mMatrixScalarMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    int i,j;
    
    mException((INVALID_MAT(mat1)||INVALID_MAT(mat2)),EXIT,"invalid input");
    mException((mat1->row != mat2->row)||(mat1->col != mat2->col),EXIT,"invalid input");

    if(INVALID_POINTER(dst)) dst = mat1;
    else mMatrixRedefine(dst,mat1->row,mat1->col,dst->data,DFLT);
    
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

void m_MatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
{
    mException((INVALID_MAT(mat1))||(INVALID_MAT(mat2)),EXIT,"invalid input");
    mException((mat1->col != mat2->row),EXIT,"invalid operate");
    int num = mat1->col;
    int dst_col = mat2->col;
    int dst_row = mat1->row;

    MMatrix *p = dst;
    if((INVALID_POINTER(dst))||(dst==mat1)||(dst==mat2)) dst = mMatrixCreate(dst_row,dst_col);
    else mMatrixRedefine(dst,dst_row,dst_col,dst->data);

    int flag = num&0x03; //if(flag==0) flag = 4;
    
    int i,j,k;
    float data1,data2,data3,data4;
    float *psrc1,*psrc2,*psrc3,*psrc4;
    float *pdst;

    // #pragma omp parallel for
    for(j=0;j<dst_row;j++)
    {
        pdst = dst->data[j];
        data1 = mat1->data[j][0]; psrc1 = mat2->data[0];
        for(k=0;k<dst_col;k++) pdst[k] = data1*psrc1[k];
        for(i=1;i<flag;i++)
        {
            data1 = mat1->data[j][i]; psrc1 = mat2->data[i];
            for(k=0;k<dst_col;k++) pdst[k]+= data1*psrc1[k];
        }
        
        for(i=flag;i<num;i=i+4)
        {
            data1 = mat1->data[j][i  ]; psrc1 = mat2->data[i  ];
            data2 = mat1->data[j][i+1]; psrc2 = mat2->data[i+1];
            data3 = mat1->data[j][i+2]; psrc3 = mat2->data[i+2];
            data4 = mat1->data[j][i+3]; psrc4 = mat2->data[i+3];
            for(k=0;k<dst_col;k++)
                pdst[k] += data1*psrc1[k]+data2*psrc2[k]+data3*psrc3[k]+data4*psrc4[k];
        }
    }

    if(p != dst)
    {
        if(p == mat2) {mObjectExchange(mat2,dst,MMatrix); mMatrixRelease(dst);}
        else          {mObjectExchange(mat1,dst,MMatrix); mMatrixRelease(dst);}
    }
}

float mMatrixDetValue(MMatrix *mat)
{
    int i,j,k;
    mException((INVALID_MAT(mat)),EXIT,"invalid input");
    int num = mat->row;mException((mat->col != num),EXIT,"invalid operate");
    
    if(num == 1) return mat->data[0][0];
    if(num == 2) return (mat->data[0][0]* mat->data[1][1]-mat->data[1][0]*mat->data[0][1]);
    if(num == 3) return (mat->data[0][0]*(mat->data[1][1]*mat->data[2][2]-mat->data[1][2]*mat->data[2][1])
                       + mat->data[0][1]*(mat->data[1][2]*mat->data[2][0]-mat->data[1][0]*mat->data[2][2])
                       + mat->data[0][2]*(mat->data[1][0]*mat->data[2][1]-mat->data[1][1]*mat->data[2][0]));
    
    double **data = (double **)mMalloc(num*sizeof(double*));
    double *pdata = (double  *)mMalloc(num*(num+3)*sizeof(double));
    for(j=0;j<num;j++)
    {
        data[j]=pdata+(num+3)*j;
        for(i=0;i<num;i++) data[j][i] = (double)(mat->data[j][i]);
    }
    
    double value = 1.0;
    for(k=0;k<num;k++)
    {
        if(data[k][k]==0)
        {
            for(j=k+1;j<num;j++)
                if(data[j][k]!=0) {double *buff = data[k]; data[k] = data[j]; data[j] = buff; value=0-value; break;}
            if(j==num){mFree(pdata);mFree(data);return 0.0f;}
        }
        value = value * data[k][k];
        
        for(j=k+1;j<num;j++)
        {
            double w = data[j][k]/data[k][k];
            for(i=k+1;i<num;i+=4)
            {
                data[j][i  ] -= w*data[k][i  ];
                data[j][i+1] -= w*data[k][i+1];
                data[j][i+2] -= w*data[k][i+2];
                data[j][i+3] -= w*data[k][i+3];
            }
        }
    }

    mFree(pdata);mFree(data);
    return (float)value;
}

int mMatrixInverse(MMatrix *mat,MMatrix *inv)
{
    int i,j,k;
    mException((INVALID_MAT(mat)),EXIT,"invalid input");
    int num = mat->row;mException((mat->col != num),EXIT,"invalid operate");
    
    double **data = (double **)mMalloc( num *sizeof(double *));
    double *pdata = (double * )mMalloc((num+num+3)*num*sizeof(double));
    for(j=0;j<num;j++)
    {
        data[j]=pdata+(num+num+3)*j;
        for(i=0;i<num;i++) data[j][i] = (double)(mat->data[j][i]);
        memset(data[j]+num,0,(num+3)*sizeof(double));data[j][num+j]=-1.0;
    }
    
    for(k=0;k<num;k++)
    {
        if(data[k][k]==0)
        {
            for(j=k+1;j<num;j++){if(data[j][k]!=0){double *buff = data[k]; data[k]=data[j]; data[j]=buff; break;}}
            if(j==num) {mFree(data);mFree(pdata);return 0;}
        }
        
        for(j=k+1;j<num;j++)
        {
            if(data[j][k]==0) continue;
            double w = data[j][k]/data[k][k];
            for(i=k+1;i<num+num;i+=4)
            {
                data[j][i  ]-=w*data[k][i  ];
                data[j][i+1]-=w*data[k][i+1];
                data[j][i+2]-=w*data[k][i+2];
                data[j][i+3]-=w*data[k][i+3];
            }
        }
    }

    MMatrix *p = inv;
    if((INVALID_POINTER(inv))||(inv == mat)) inv = mMatrixCreate(num,num+3,NULL);
    else mMatrixRedefine(inv,num,num+3,inv->data);
    
    for(j=num-1;j>=0;j--)for(k=num;k<num+num;k+=4)
    {
        data[j][k  ]=0-data[j][k  ];
        data[j][k+1]=0-data[j][k+1];
        data[j][k+2]=0-data[j][k+2];
        data[j][k+3]=0-data[j][k+3];
        for(i=j+1;i<num;i++)
        {
            data[j][k  ]-=data[j][i]*data[i][k  ];
            data[j][k+1]-=data[j][i]*data[i][k+1];
            data[j][k+2]-=data[j][i]*data[i][k+2];
            data[j][k+3]-=data[j][i]*data[i][k+3];
        }
        data[j][k  ] /= data[j][j];inv->data[j][k  -num] = (float)data[j][k  ];
        data[j][k+1] /= data[j][j];inv->data[j][k+1-num] = (float)data[j][k+1];
        data[j][k+2] /= data[j][j];inv->data[j][k+2-num] = (float)data[j][k+2];
        data[j][k+3] /= data[j][j];inv->data[j][k+3-num] = (float)data[j][k+3];
    }
    
    mFree(pdata);mFree(data);
    if(p!=inv){mObjectExchange(inv,mat,MMatrix);mMatrixRelease(inv);}
    inv->col=num;
    return 1;
}

int mLinearEquation(MMatrix *mat,float *answer)
{
    int i,j,k;
    mException((INVALID_MAT(mat))||(INVALID_POINTER(answer)),EXIT,"invalid input");
    int num = mat->row;mException((mat->col - num!=1),EXIT,"invalid operate");
    
    if(num == 1)
    {
        if(mat->data[0][0]==0.0f) return 0;
        *answer = (0.0f-mat->data[0][1])/mat->data[0][0];
        return 1;
    }

    double **data=(double **)mMalloc(num*sizeof(double *));
    double *pdata= (double *)mMalloc(num*(num+4)*sizeof(double));
    for(j=0;j<num;j++)
    {
        data[j]=pdata+j*(num+4);
        for(int i=0;i<num+1;i++) data[j][i]=(double)(mat->data[j][i]);
    }
    
    for(k=0;k<num;k++)
    {
        if(data[k][k]==0)
        {
            for(j=k+1;j<num;j++) if(data[j][k]!=0) {double *buff = data[k];data[k]=data[j];data[j]=buff; break;}
            if(j==num) {mFree(pdata);mFree(data);return 0;}
        }
        
        for(j=k+1;j<num;j++)
        {
            if(data[j][k]==0) continue;
            double w = data[j][k]/data[k][k];
            for(i=k+1;i<num+1;i+=4)
            {
                data[j][i  ]-=w*data[k][i  ];
                data[j][i+1]-=w*data[k][i+1];
                data[j][i+2]-=w*data[k][i+2];
                data[j][i+3]-=w*data[k][i+3];
            }
        }
    }

    double *a=mMalloc((num+3)*sizeof(double));a[num]=0;a[num+1]=0;a[num+2]=0;
    for(j=num-1;j>=0;j--)
    {
        a[j] = 0-data[j][num];
        for(i=j+1;i<num;i+=4)
            a[j]-= data[j][i]*a[i]+data[j][i+1]*a[i+1]+data[j][i+2]*a[i+2]+data[j][i+3]*a[i+3];
        a[j] = a[j]/data[j][j];
        answer[j]=a[j];
    }

    mFree(data);mFree(pdata);mFree(a);
    return 1;
}
