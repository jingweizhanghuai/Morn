#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_math.h"

void mMatrixMaxElement(MMatrix *src,float *element,int *x,int *y)
{
    int i,j;
    int col,row;
    
    float data;
    
    mException((INVALID_MAT(src)),EXIT,"invalid input");
    
    data = src->data[0][0];
    col = 0;
    row = 0;
    for(j=0;j<src->row;j++)
        for(i=0;i<src->col;i++)
            if(src->data[j][i] >data)
            {
                data = src->data[j][i];
                col = i;
                row = j;
            }

    if(!INVALID_POINTER(element))
        *element = data;
    if(!INVALID_POINTER(x))
        *x = col;
    if(!INVALID_POINTER(y))
        *y = row;
    
}

#define MAX_ELEMENT(mat,n,x,y) {\
    int k,l;\
    float data;\
    data = 0.0;\
    x = 0;\
    y = 0;\
    for(l=1;l<n;l++)\
        for(k=0;k<l;k++)\
        {\
            if(ABS(mat->data[l][k]+mat->data[k][l]) <= tiny)\
                continue;\
            \
            if(ABS(mat->data[l][k]) >data)\
            {\
                data = ABS(mat->data[l][k]);\
                x = k;\
                y = l;\
            }\
            if(ABS(mat->data[k][l]) >data)\
            {\
                data = ABS(mat->data[k][l]);\
                x = l;\
                y = k;\
            }\
        }\
}\

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

/////////////////////////////////////////////////////////
// 接口功能：
//  计算方阵的（实）特征值和特征向量
//
// 参数：
//  (I)src(NO) - 待计算的方阵
//  (O)eigenvalue(NULL) - 求得的特征值
//  (O)eigenvector(NULL) - 求得的特征向量（与特征值对应）
//  (O)eigennum(NULL) - 特征值的个数
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mMatrixEigenvalue(MMatrix *src,float eigenvalue[],float *eigenvector[],int *eigennum)
{
    int t;
    int i,j;
    int n;
    
    int p,q;
    // int n1,n2;
    
    int flag;
    
    int eigen_num;
    float tiny;
    
    float data0,data1,data2,data3;
    
    double angle;
    float cos1,sin1,cos2,sin2,sin_cos;
    
    MMatrix *mat;
    MMatrix *equ;
   
    mException((INVALID_MAT(src)),EXIT,"invalid input");
    n = src->col;
    mException((n != src->row),EXIT,"invalid input");

    mat = mMatrixCreate(n,n,NULL);
    for(j=0;j<n;j++)
        memcpy(mat->data[j],src->data[j],n*sizeof(float));
    
    equ = mMatrixCreate(n-1,n,NULL);
    for(j=0;j<n-1;j++)
        memcpy(equ->data[j],src->data[j],n*sizeof(float));
    
    for(t=0;t<100;t++)
    {
        // PrintMat(mat);
        
        MAX_EIGENVALUE(mat,n,tiny);
        tiny = tiny*0.00001f;

        MAX_ELEMENT(mat,n,q,p);
        // printf("%d:(%d,%d)\n",t,p,q);
        
        if(p==q)
            break;

        data0 = mat->data[p][p];
        data1 = mat->data[p][q];
        data2 = mat->data[q][p];
        data3 = mat->data[q][q];
        
        // printf("%f,%f,%f,%f\n",data0,data1,data2,data3);
        
        if(ABS(data1) <= tiny)
            break;
        
        angle = atan((double)((data1+data2)/(data0-data3)))/2.0;
        
        sin1 = (float)sin(angle);
        cos1 = (float)cos(angle);
        
        sin_cos = sin1*cos1;
        sin2 = sin1*sin1;
        cos2 = cos1*cos1;
        
        // printf("%f,%f,%f,%f\n",cos1,sin1,cos2,sin2);
        
        mat->data[p][p] = data0*cos2+data3*sin2+(data1+data2)*sin_cos;
        mat->data[q][q] = data3*cos2+data0*sin2-(data1+data2)*sin_cos;
        mat->data[q][p] = data2*cos2-data1*sin2+(data3-data0)*sin_cos;
        // mat->data[p][q] = data1*cos2-data2*sin2+(data3-data0)*sin_cos;
        mat->data[p][q] = 0.0f - mat->data[q][p];
        
        // n1 = (p<q)?p:q;
        // n2 = (p<q)?q:p;
        
        for(i=0;i<n;i++)
        {
            if((i==p)||(i==q))
                continue;
            
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
    
    eigen_num = 0;
    for(i=0;i<n;i++)
    {
        flag = 1;
        
        for(j=0;j<n;j++)
        {
            if(i==j)
                continue;
            
            if((ABS(mat->data[i][j]) > tiny)||(ABS(mat->data[j][i]) > tiny))
               flag = 0; 
        }
        
        if(flag == 1)
        {
            if(!INVALID_POINTER(eigenvalue))
                eigenvalue[eigen_num] = mat->data[i][i];
            
            if(!INVALID_POINTER(eigenvector))
            {
                for(j=0;j<n-1;j++)
                    equ->data[j][j] = src->data[j][j]-mat->data[i][i];
                
                mLinearEquation(equ,eigenvector[eigen_num]);
                eigenvector[eigen_num][n-1] = 1.0f;
            }
                
            eigen_num = eigen_num +1;
        }
    }
    mMatrixRelease(mat);
    mMatrixRelease(equ);
    
    if(!INVALID_POINTER(eigennum))
        *eigennum = eigen_num;
}

