/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_math.h"

float m_MatrixMaxElement(MMatrix *src,int *x,int *y)
{
    mException((INVALID_MAT(src)),EXIT,"invalid input");
    
    float data = src->data[0][0];
    int col = 0,row = 0;
    for(int j=0;j<src->row;j++)for(int i=0;i<src->col;i++)
        if(src->data[j][i] >data) {data = src->data[j][i];col = i;row = j;}

    if(!INVALID_POINTER(x)) *x = col;
    if(!INVALID_POINTER(y)) *y = row;
    return data;
}

float m_MatrixMinElement(MMatrix *src,int *x,int *y)
{
    mException((INVALID_MAT(src)),EXIT,"invalid input");
    
    float data = src->data[0][0];
    int col = 0,row = 0;
    for(int j=0;j<src->row;j++)for(int i=0;i<src->col;i++)
        if(src->data[j][i] <data) {data = src->data[j][i];col = i;row = j;}

    if(!INVALID_POINTER(x)) *x = col;
    if(!INVALID_POINTER(y)) *y = row;
    return data;
}

#define MAX_ELEMENT(mat,n,x,y) {\
    float data = 0.0;\
    x = 0;y = 0;\
    for(int l=1;l<n;l++)for(int k=0;k<l;k++)\
    {\
        if(ABS(mat->data[l][k]) >data) {data = ABS(mat->data[l][k]);x = k;y = l;}\
        if(ABS(mat->data[k][l]) >data) {data = ABS(mat->data[k][l]);x = l;y = k;}\
    }\
}\

/*
#define MAX_EIGENVALUE(mat,n,value)\
{\
    int l;\
    value = ABS(mat->data[0][0]);\
    for(l=1;l<n;l++)\
    {\
        if(ABS(mat->data[l][l])>value)\
            value = ABS(mat->data[l][l]);\
    }\
}
*/

// void mMatrixEigenvalue(MMatrix *src,float eigenvalue[],float *eigenvector[],int *eigennum)
// {
//     mException((INVALID_MAT(src)),EXIT,"invalid input");
//     int n = src->col;mException((n != src->row),EXIT,"invalid input");

//     MMatrix *mat = mMatrixCreate(n,n);
//     for(int j=0;j<n;j++) memcpy(mat->data[j],src->data[j],n*sizeof(float));
    
//     MMatrix *equ = mMatrixCreate(n-1,n);
//     for(int j=0;j<n-1;j++) memcpy(equ->data[j],src->data[j],n*sizeof(float));

//     float tiny;
//     for(int t=0;t<100;t++)
//     {
//         // PrintMat(mat);
//         MAX_EIGENVALUE(mat,n,tiny);
//         tiny = tiny*0.00001f;

//         int p,q;
//         MAX_ELEMENT(mat,n,q,p);
//         // printf("%d:(%d,%d)\n",t,p,q);
//         if(p==q) break;

//         float data0 = mat->data[p][p];
//         float data1 = mat->data[p][q];
//         float data2 = mat->data[q][p];
//         float data3 = mat->data[q][q];
//         // printf("%f,%f,%f,%f\n",data0,data1,data2,data3);
//         if(ABS(data1) <= tiny) break;
        
//         double angle = atan((double)((data1+data2)/(data0-data3)))/2.0;
//         float sin1 = (float)sin(angle);
//         float cos1 = (float)cos(angle);
//         float sin_cos = sin1*cos1;
//         float sin2 = sin1*sin1;
//         float cos2 = cos1*cos1;
//         // printf("%f,%f,%f,%f\n",cos1,sin1,cos2,sin2);
        
//         mat->data[p][p] = data0*cos2+data3*sin2+(data1+data2)*sin_cos;
//         mat->data[q][q] = data3*cos2+data0*sin2-(data1+data2)*sin_cos;
//         mat->data[q][p] = data2*cos2-data1*sin2+(data3-data0)*sin_cos;
//         mat->data[p][q] = 0.0f - mat->data[q][p];
        
//         for(int i=0;i<n;i++)
//         {
//             if((i==p)||(i==q)) continue;
            
//             data0 = mat->data[i][p];
//             data1 = mat->data[i][q];
//             mat->data[i][p] = cos1*data0+sin1*data1;
//             mat->data[i][q] = cos1*data1-sin1*data0;
            
//             data0 = mat->data[p][i];
//             data1 = mat->data[q][i];
//             mat->data[p][i] = cos1*data0+sin1*data1;
//             mat->data[q][i] = cos1*data1-sin1*data0;
//         }
//         // PrintMat(mat);
//     }
    
//     int eigen_num = 0;
//     for(int i=0;i<n;i++)
//     {
//         int flag = 1;
//         for(int j=0;j<n;j++)
//         {
//             if(i==j) continue;
//             if((ABS(mat->data[i][j])>tiny)||(ABS(mat->data[j][i])>tiny)) {flag = 0;break;}
//         }
//         if(flag == 0) continue;
        
//         if(!INVALID_POINTER(eigenvalue)) eigenvalue[eigen_num] = mat->data[i][i];
//         if(!INVALID_POINTER(eigenvector))
//         {
//             for(int j=0;j<n-1;j++) equ->data[j][j] = src->data[j][j]-mat->data[i][i];
//             mLinearEquation(equ,eigenvector[eigen_num]);
//             eigenvector[eigen_num][n-1] = 1.0f;
//         }
//         eigen_num = eigen_num +1;
//     }
//     mMatrixRelease(mat);
//     mMatrixRelease(equ);
    
//     if(!INVALID_POINTER(eigennum)) *eigennum = eigen_num;
// }

void mMatrixEigenValue(MMatrix *src,MList *eigenvalue,MList *eigenvector)
{
    mException((INVALID_MAT(src)),EXIT,"invalid input");
    int n = src->col;mException((n != src->row),EXIT,"invalid input");

    MMatrix *mat = mMatrixCreate(n,n);
    for(int j=0;j<n;j++) memcpy(mat->data[j],src->data[j],n*sizeof(float));
    
    for(int t=0;t<100;t++)
    {
        int p,q;
        MAX_ELEMENT(mat,n,q,p);
        if(p==q) break;

        float data0 = mat->data[p][p];
        float data1 = mat->data[p][q];
        float data2 = mat->data[q][p];
        float data3 = mat->data[q][q];
        
        double angle = atan((double)((data1+data2)/(data0-data3)))/2.0;
        float sin1 = (float)sin(angle),cos1 = (float)cos(angle);
        float sin_cos = sin1*cos1,sin2 = sin1*sin1,cos2 = cos1*cos1;
        
        mat->data[p][p] = data0*cos2+data3*sin2+(data1+data2)*sin_cos;
        mat->data[q][q] = data3*cos2+data0*sin2-(data1+data2)*sin_cos;
        mat->data[q][p] = data2*cos2-data1*sin2+(data3-data0)*sin_cos;
        mat->data[p][q] = 0.0f - mat->data[q][p];
        
        for(int i=0;i<n;i++)
        {
            if((i==p)||(i==q)) continue;
            
            data0 = mat->data[i][p];
            data1 = mat->data[i][q];
            mat->data[i][p] = cos1*data0+sin1*data1;
            mat->data[i][q] = cos1*data1-sin1*data0;
            
            data0 = mat->data[p][i];
            data1 = mat->data[q][i];
            mat->data[p][i] = cos1*data0+sin1*data1;
            mat->data[q][i] = cos1*data1-sin1*data0;
        }
    }
    
    MMatrix *equ = NULL;
    float *answer=NULL;
    mListClear(eigenvalue);
    if(!INVALID_POINTER(eigenvector))
    {
        mListClear(eigenvector);
        equ = mMatrixCreate(n-1,n);
        for(int j=0;j<n-1;j++) memcpy(equ->data[j],src->data[j],n*sizeof(float));
        answer = (float *)mMalloc(n*sizeof(float));
        answer[n-1] = 1.0f;
    }
    for(int i=0;i<n;i++)
    {
        mListWrite(eigenvalue,DFLT,&(mat->data[i][i]),sizeof(float));
        if(!INVALID_POINTER(eigenvector))
        {
            for(int j=0;j<n-1;j++) equ->data[j][j] = src->data[j][j]-mat->data[i][i];
            mLinearEquation(equ,answer);
            mListWrite(eigenvector,DFLT,answer,n*sizeof(float));
        }
    }
    if(equ) mMatrixRelease(equ);
    if(answer) mFree(answer);
    mMatrixRelease(mat);
}



/*
struct HandleMatrixEigen
{
    MMatrix *mat;
    MMatrix *equ;
    MArray *array;
    MMatrix *eigenvector;
    MVector *vector;
};
void endMatrixEigen(struct HandleMatrixEigen *handle)
{
    if(handle->mat!=NULL) mMatrixRelease(handle->mat);
    if(handle->equ!=NULL) mMatrixRelease(handle->equ);
    if(handle->array!=NULL) mArrayRelease(handle->array);
    if(handle->eigenvector!=NULL) mMatrixRelease(handle->eigenvector);
    if(handle->vector!=NULL) mVectorRelease(handle->vector);
}
#define HASH_MatrixEigen 0x5dc79c5a
MArray *mMatrixEigenValue(MMatrix *src)//,float eigenvalue[],float *eigenvector[],int *eigennum)
{
    mException((INVALID_MAT(src)),EXIT,"invalid input");
    int n = src->col;mException((n != src->row),EXIT,"invalid input");

    MHandle *hdl = mHandle(src,MatrixEigen);
    struct HandleMatrixEigen *handle = (struct HandleMatrixEigen *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->mat==NULL) handle->mat = mMatrixCreate(n,n);
        else                  mMatrixRedefine(handle->mat,n,n);
        if(handle->equ==NULL) handle->equ = mMatrixCreate(n-1,n);
        else                  mMatrixRedefine(handle->equ,n-1,n);
        for(int j=0;j<n  ;j++) memcpy(handle->mat->data[j],src->data[j],n*sizeof(float));
        for(int j=0;j<n-1;j++) memcpy(handle->equ->data[j],src->data[j],n*sizeof(float));
        
        
    }
    MMatrix *mat = handle->mat;
    MMatrix *equ = handle->equ;
    

    float tiny;
    for(int t=0;t<100;t++)
    {
        // PrintMat(mat);
        MAX_EIGENVALUE(mat,n,tiny);
        tiny = tiny*0.00001f;

        int p,q;
        MAX_ELEMENT(mat,n,q,p);
        // printf("%d:(%d,%d)\n",t,p,q);
        if(p==q) break;

        float data0 = mat->data[p][p];
        float data1 = mat->data[p][q];
        float data2 = mat->data[q][p];
        float data3 = mat->data[q][q];
        // printf("%f,%f,%f,%f\n",data0,data1,data2,data3);
        if(ABS(data1) <= tiny) break;
        
        double angle = atan((double)((data1+data2)/(data0-data3)))/2.0;
        float sin1 = (float)sin(angle);
        float cos1 = (float)cos(angle);
        float sin_cos = sin1*cos1;
        float sin2 = sin1*sin1;
        float cos2 = cos1*cos1;
        // printf("%f,%f,%f,%f\n",cos1,sin1,cos2,sin2);
        
        mat->data[p][p] = data0*cos2+data3*sin2+(data1+data2)*sin_cos;
        mat->data[q][q] = data3*cos2+data0*sin2-(data1+data2)*sin_cos;
        mat->data[q][p] = data2*cos2-data1*sin2+(data3-data0)*sin_cos;
        mat->data[p][q] = 0.0f - mat->data[q][p];
        
        for(int i=0;i<n;i++)
        {
            if((i==p)||(i==q)) continue;
            
            data0 = mat->data[i][p];
            data1 = mat->data[i][q];
            mat->data[i][p] = cos1*data0+sin1*data1;
            mat->data[i][q] = cos1*data1-sin1*data0;
            
            data0 = mat->data[p][i];
            data1 = mat->data[q][i];
            mat->data[p][i] = cos1*data0+sin1*data1;
            mat->data[q][i] = cos1*data1-sin1*data0;
        }
        // PrintMat(mat);
    }
    
    int eigen_num = 0;
    for(int i=0;i<n;i++)
    {
        int flag = 1;
        for(int j=0;j<n;j++)
        {
            if(i==j) continue;
            if((ABS(mat->data[i][j])>tiny)||(ABS(mat->data[j][i])>tiny)) {flag = 0;break;}
        }
        if(flag == 0) continue;
        
        if(!INVALID_POINTER(eigenvalue)) eigenvalue[eigen_num] = mat->data[i][i];
        if(!INVALID_POINTER(eigenvector))
        {
            for(int j=0;j<n-1;j++) equ->data[j][j] = src->data[j][j]-mat->data[i][i];
            mLinearEquation(equ,eigenvector[eigen_num]);
            eigenvector[eigen_num][n-1] = 1.0f;
        }
        eigen_num = eigen_num +1;
    }
    mMatrixRelease(mat);
    mMatrixRelease(equ);
    
    if(!INVALID_POINTER(eigennum)) *eigennum = eigen_num;
}
*/

