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

#include "morn_image.h"

void mLinearFitLSM(float *XIn,float *YIn,int N,float *A)
{
    int i;
    float sumx,sumy,sumxy,sumx2;
    
    mException((N<2),EXIT,"sample number (which <2) is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    sumx = 0.0f;
    sumy = 0.0f;
    sumxy = 0.0f;
    sumx2 = 0.0f;
    for(i=0;i<N;i++)
    {
        sumx = sumx + XIn[i];
        sumy = sumy + YIn[i];
        sumxy = sumxy + XIn[i]*YIn[i];
        sumx2 = sumx2 + XIn[i]*XIn[i];
    }
    
    A[0] = ((((float)N)*sumxy)-(sumx*sumy))/((((float)N)*sumx2)-(sumx*sumx));
    A[1] = ((sumx2*sumy)-(sumx*sumxy))/((((float)N)*sumx2)-(sumx*sumx));
}

void mPiecewiseLinearFit(float *XIn,float *YIn,int N,int piece_num,MList *list)
{
    int i,j;
    float *thresh = mMalloc(piece_num*8*sizeof(int));
    float x_min=XIn[0],x_max=XIn[0],y_min=YIn[0],y_max=YIn[0];
    for(i=1;i<N;i++)
    {
        x_min = MIN(x_min,XIn[i]);
        x_max = MAX(x_max,XIn[i]);
        y_min = MIN(y_min,YIn[i]);
        y_max = MAX(y_max,YIn[i]);
    }
    int   *num  =mMalloc(piece_num*8*sizeof(int)  );memset(  num,0,piece_num*8*sizeof(int)  );
    float *sumx =mMalloc(piece_num*8*sizeof(float));memset( sumx,0,piece_num*8*sizeof(float));
    float *sumy =mMalloc(piece_num*8*sizeof(float));memset( sumy,0,piece_num*8*sizeof(float));
    float *sumxy=mMalloc(piece_num*8*sizeof(float));memset(sumxy,0,piece_num*8*sizeof(float));
    float *sumx2=mMalloc(piece_num*8*sizeof(float));memset(sumx2,0,piece_num*8*sizeof(float));
    
    struct LineInfo
    {
        MImagePoint ps;
        MImagePoint pe;
        float k;
        float b;
    };
    
    struct LineInfo line;
    int flag = (x_max-x_min>y_max-y_min);
    if(flag)
    {
        float step = (x_max-x_min)/(piece_num*8);
        for(i=0;i<N;i++)
        {
            int n = (int)((XIn[i]-x_min)/step+0.5);
            num[n] +=1;
            sumx[n]+=XIn[i];sumy[n]+=YIn[i];
            sumxy[n]+=XIn[i]*YIn[i];sumx2[n]+=XIn[i]*XIn[i];
        }
    }
    else
    {
        float step = (y_max-y_min)/(piece_num*8);
        for(i=0;i<N;i++)
        {
            int n = (int)((YIn[i]-y_min)/step+0.5);
            num[n] +=1;
            sumx[n]+=XIn[i];sumy[n]+=YIn[i];
            sumxy[n]+=XIn[i]*YIn[i];sumx2[n]+=XIn[i]*XIn[i];
        }
    }
    
    int m=0;int sum=0;float xsum=0,ysum=0,xysum=0,x2sum=0;
    for(i=0;i<piece_num*8;i++)
    {
        float a[2];
        sum += num[i];
        xsum += sumx[i];ysum += sumy[i];
        xysum+=sumxy[i];x2sum+=sumx2[i];
        if(sum>=N*(m+1)/piece_num) 
        {
            line.k = ((((float)sum)*xysum)-(xsum*ysum))/((((float)sum)*x2sum)-(xsum*xsum));
            line.b = ((x2sum*ysum)-(xsum*xysum))/((((float)sum)*x2sum)-(xsum*xsum));
            mListWrite(list,m,&line,sizeof(struct LineInfo));
            xsum =0;ysum =0;xysum=0;x2sum=0;sum = 0;
            m=m+1;
        }
    }
    mFree(num);mFree(sumx);mFree(sumy);mFree(sumxy);mFree(sumx2);
    
    struct LineInfo *info1,*info2;
    info1 = list->data[0];
    if(flag) {info1->ps.x = x_min;info1->ps.y = info1->k*x_min +info1->b;}
    else     {info1->ps.y = y_min;info1->ps.x =(y_min-info1->b)/info1->k;}
    for(i=0;i<m-1;m=m+1)
    {
        info1 = list->data[m];
        info2 = list->data[m+1];
        float x = (info2->b-info1->b)/(info1->k-info2->k);
        float y = info1->k*x+info1->b;
        info1->pe.x = x;info1->pe.y=y;
        info2->ps.x = x;info2->ps.y=y;
    }
    info2 = list->data[m-1];
    if(flag) {info2->pe.x = x_max;info2->pe.y = info2->k*x_max +info2->b;}
    else     {info2->pe.y = y_max;info2->pe.x =(y_max-info2->b)/info2->k;}
}



/////////////////////////////////////////////////////////
// 接口功能:
//  用最小二乘法拟合二维空间抛物线
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]X^2+A[1]X+A[2]
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mParabolaFitLSM(float *XIn,float *YIn,int N,float *A)
{
    int i;
    float x,y,x2;
    // float data[12] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
    MMatrix *mat;
    
    mException((N<3),EXIT,"sample number (which <3) is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    mat = mMatrixCreate(3,4,NULL);
    memset(mat->data[0],0,4*sizeof(float));
    memset(mat->data[1],0,4*sizeof(float));
    memset(mat->data[2],0,4*sizeof(float));
    
    for(i=0;i<N;i++)
    {
        x = XIn[i];
        y = YIn[i];
        x2 = XIn[i]*XIn[i];
        
        mat->data[0][0] = mat->data[0][0] + x2*x2;
        mat->data[0][1] = mat->data[0][1] + x2*x;
        mat->data[0][2] = mat->data[0][2] + x2;
        mat->data[0][3] = mat->data[0][3] - x2*y;
        
        mat->data[1][2] = mat->data[1][2] + x;
        mat->data[1][3] = mat->data[1][3] - x*y;
        
        mat->data[2][3] = mat->data[2][3] - y;
    }
    
    mat->data[1][0] = mat->data[0][1];
    mat->data[1][1] = mat->data[0][2];
    mat->data[2][0] = mat->data[0][2];
    mat->data[2][1] = mat->data[1][2];
    mat->data[2][2] = (float)N;
    
    mLinearEquation(mat,A);
    
    mMatrixRelease(mat);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  使用最小二乘法在二维空间上做多项式拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]X^k + A[1]X^(k-1) +...+ A[k-1]*X + A[k]
//  (I)k(NO) - 拟合多项式的次数
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mPolyFitLSM(float *XIn,float *YIn,int N,float *A,int k)
{
    int i,j;
    float *sumx;
    float *sumyx;
    float sumy;
    // float *data;
    float x,y,X;
    MMatrix *mat;
    
    mException((k==0),EXIT,"degree of polynomial is 0");
    mException((N<k+1),EXIT,"sample number is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    if(k == 1)
    {
        mLinearFitLSM(XIn,YIn,N,A);
        return;
    }
    if(k == 2)
    {
        mParabolaFitLSM(XIn,YIn,N,A);
        return;
    }
    
    sumx = (float *)malloc(k*2*sizeof(float));
    sumyx = (float *)malloc(k*sizeof(float));
    // data = (float *)malloc((k+2)*(k+1)*sizeof(float));
    mat = mMatrixCreate(k+1,k+2,NULL);
    
    for(i=0;i<k*2;i++)
        sumx[i] = 0.0f;
    
    for(i=0;i<k;i++)
        sumyx[i] = 0.0f;
    sumy = 0.0;
    
    for(i=0;i<N;i++)
    {
        x = XIn[i];
        y = YIn[i];
        
        X = 1.0f;        
        for(j=0;j<k*2;j++)
        {
            X = X*x;
            sumx[j] = sumx[j] + X;
        }
        
        X = 1.0f;
        for(j=0;j<k;j++)
        {
            X = X*x;
            sumyx[j] = sumyx[j] + X*y;
        }
        
        sumy = sumy + y;
    }
    
    // for(j=0;j<k*2;j++)
        // printf("sumx%d is %f\n",j+1,sumx[j]);
    // printf("\n\n");
    // for(j=0;j<k;j++)
        // printf("sumyx%d is %f\n",j+1,sumyx[j]);
    
    for(i=0;i<k;i++)
        mat->data[0][i] = sumx[k-i-1];
    
    mat->data[0][k] = (float)N;
    mat->data[0][k+1] = 0.0f-sumy;
    
    // #define DATA(x,y) data[y*(k+2)+x]
    
    for(j=1;j<=k;j++)
    {
        for(i=0;i<=k;i++)
            mat->data[j][i] = sumx[k+j-i-1];
        
        mat->data[j][i] = 0.0f-sumyx[j-1];
    }
    
    // for(j=0;j<=k;j++)
    // {
        // printf("%d:\t",j);
        // for(i=0;i<=k+1;i++)
            // printf("%f\t",DATA(i,j));
    
        // printf("\n");
    // }
    
    mLinearEquation(mat,A);
    
    free(sumx);
    free(sumyx);
    mMatrixRelease(mat);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  对有权重的点集拟合二维空间直线
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)WIn(NULL) - 待拟合点的权重
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mLinearFitWeight(float *XIn,float *YIn,float *WIn,int N,float *A)
{
    int i;
    float sum,sumx,sumy,sumxy,sumx2;
    
    mException((N<2),EXIT,"sample number (which <2) is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    if(INVALID_POINTER(WIn))
        mException(DFLT,DFLT,"no input weight.");
    
    sumx = 0.0f;
    sumy = 0.0f;
    sumxy = 0.0f;
    sumx2 = 0.0f;
    sum = 0.0f;
    for(i=0;i<N;i++)
    {
        sum = sum + WIn[i];
        sumx = sumx + WIn[i]*XIn[i];
        sumy = sumy + WIn[i]*YIn[i];
        sumxy = sumxy + WIn[i]*XIn[i]*YIn[i];
        sumx2 = sumx2 + WIn[i]*XIn[i]*XIn[i];
    }
    
    A[0] = ((sum*sumxy)-(sumx*sumy))/((sum*sumx2)-(sumx*sumx));
    A[1] = ((sumx2*sumy)-(sumx*sumxy))/((sum*sumx2)-(sumx*sumx));
}

/////////////////////////////////////////////////////////
// 接口功能:
//  拟合二维空间直线
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mLinearFit(float *XIn,float *YIn,int N,float *A)
{
    int i;
    float *w;
    float error0,error1;
    int n;
    float y_max,y_min;
    
    mException((N<2),EXIT,"sample number (which <2) is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    y_max = YIn[0];
    y_min = YIn[0];
    for(i=0;i<N;i++)
    {
        y_max = (YIn[i]>y_max)?YIn[i]:y_max;
        y_min = (YIn[i]<y_min)?YIn[i]:y_min;
    }
    
    w = (float *)malloc(N*sizeof(float));
    
    mLinearFitLSM(XIn,YIn,N,A);
    
    error0 = (y_max-y_min)*(y_max-y_min);
    error1 = 0.0;
    for(i=0;i<N;i++)
    {
        w[i] = YIn[i]-(A[0]*XIn[i]+A[1]);
        w[i] = w[i]*w[i];
        error1 = error1+w[i];
    }
    error1 = error1/(float)N;
    
    for(i=0;i<N;i++)
        w[i] = (w[i]>error1)?(1.0f/w[i]):1.0f;
    
    // printf("error0 is %f,error1 is %f\n",error0,error1);
    
    n = 0;
    while(ABS(error0-error1)>0.001f*(y_max-y_min))
    {
        n=n+1;
        if(n>=100)
            break;
        
        mLinearFitWeight(XIn,YIn,w,N,A);
        
        error0 = error1;
        error1 = 0.0;
        for(i=0;i<N;i++)
        {
            w[i] = YIn[i]-(A[0]*XIn[i]+A[1]);
            w[i] = w[i]*w[i];
            error1 = error1+w[i];
        }
        error1 = error1/(float)N;
        
        for(i=0;i<N;i++)
            w[i] = (w[i]>error1)?(1.0f/w[i]):1.0f;
        
        // printf("error0 is %f,error1 is %f\n",error0,error1);
    }
    
    free(w);
}
 
/////////////////////////////////////////////////////////
// 接口功能:
//  对有权重的点集拟合二维空间抛物线
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)WIn(NULL) - 待拟合点的权重
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mParabolaFitWeight(float *XIn,float *YIn,float *WIn,int N,float *A)
{
    int i;
    float x,y,x2,w;
    // float data[12] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
    MMatrix *mat;
    
    mException((N<3),EXIT,"sample number (which <3) is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    if(INVALID_POINTER(WIn))
        mException(DFLT,DFLT,"no input weight.");
    
    mat = mMatrixCreate(3,4,NULL);
    memset(mat->data[0],0,4*sizeof(float));
    memset(mat->data[1],0,4*sizeof(float));
    memset(mat->data[2],0,4*sizeof(float));
    
    for(i=0;i<N;i++)
    {
        w = WIn[i];
        x = XIn[i];
        y = YIn[i];
        x2 = XIn[i]*XIn[i];
        
        mat->data[2][2] = mat->data[2][2] + w;
        
        mat->data[0][0] = mat->data[0][0] + w*x2*x2;
        mat->data[0][1] = mat->data[0][1] + w*x2*x;
        mat->data[0][2] = mat->data[0][2] + w*x2;
        mat->data[0][3] = mat->data[0][3] - w*x2*y;
        
        mat->data[1][2] = mat->data[1][2] + w*x;
        mat->data[1][3] = mat->data[1][3] - w*x*y;
        
        mat->data[2][3] = mat->data[2][3] - w*y;
    }
    
    mat->data[1][0] = mat->data[0][1];
    mat->data[1][1] = mat->data[0][2];
    mat->data[2][0] = mat->data[0][2];
    mat->data[2][1] = mat->data[1][2];
    
    mLinearEquation(mat,A);
    mMatrixRelease(mat);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  拟合二维空间抛物线
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mParabolaFit(float *XIn,float *YIn,int N,float *A)
{
    int i;
    float *w;
    float error0,error1;
    int n;
    float y_max,y_min;
    
    mException((N<3),EXIT,"sample number (which <3) is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    y_max = YIn[0];
    y_min = YIn[0];
    for(i=0;i<N;i++)
    {
        y_max = (YIn[i]>y_max)?YIn[i]:y_max;
        y_min = (YIn[i]<y_min)?YIn[i]:y_min;
    }
    
    w = (float *)malloc(N*sizeof(float));
    
    mParabolaFitLSM(XIn,YIn,N,A);
    
    error0 = (y_max-y_min)*(y_max-y_min);
    error1 = 0.0;
    for(i=0;i<N;i++)
    {
        w[i] = YIn[i]-(A[0]*XIn[i]*XIn[i]+A[1]*XIn[i]+A[2]);
        w[i] = w[i]*w[i];
        error1 = error1+w[i];
    }
    error1 = error1/(float)N;
    
    for(i=0;i<N;i++)
        w[i] = (w[i]>error1)?(1.0f/w[i]):1.0f;
    
    // printf("error0 is %f,error1 is %f\n",error0,error1);
    
    n = 0;
    while(ABS(error0-error1)>0.001f*(y_max-y_min))
    {
        n=n+1;
        if(n>=100)
            break;
        
        mParabolaFitWeight(XIn,YIn,w,N,A);
        
        error0 = error1;
        error1 = 0.0f;
        for(i=0;i<N;i++)
        {
            w[i] = YIn[i]-(A[0]*XIn[i]*XIn[i]+A[1]*XIn[i]+A[2]);
            w[i] = w[i]*w[i];
            error1 = error1+w[i];
        }
        error1 = error1/(float)N;
        
        for(i=0;i<N;i++)
            w[i] = (w[i]>error1)?(1.0f/w[i]):1.0f;
        
        // printf("error0 is %f,error1 is %f\n",error0,error1);
    }
    
    free(w);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  对有权重的点集在二维空间上做多项式拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)WIn(NULL) - 待拟合点的权重
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mPolyFitWeight(float *XIn,float *YIn,float *WIn,int N,float *A,int k)
{
    int i,j;
    float *sumx;
    float *sumyx;
    float sumy;
    float sum;
    // float *data;
    MMatrix *mat;
    float x,y,w,X;
    
    
    mException((k==0),EXIT,"degree of polynomial is 0");
    mException((N<k+1),EXIT,"sample number is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    if(INVALID_POINTER(WIn))
        mException(DFLT,DFLT,"no input weight.");
    
    if(k == 1)
    {
        mLinearFitWeight(XIn,YIn,WIn,N,A);
        return;
    }
    if(k == 2)
    {
        mParabolaFitWeight(XIn,YIn,WIn,N,A);
        return;
    }
    
    sumx = (float *)malloc(k*2*sizeof(float));
    sumyx = (float *)malloc(k*sizeof(float));
    // data = (float *)malloc((k+2)*(k+1)*sizeof(float));
    mat = mMatrixCreate(k+1,k+2,NULL);
    
    memset(sumx,0,k*2*sizeof(float));
    memset(sumyx,0,k*sizeof(float));
    sumy = 0.0f;
    sum = 0.0f;
    for(i=0;i<N;i++)
    {
        x = XIn[i];
        y = YIn[i];
        w = WIn[i];
        
        X = 1.0f;        
        for(j=0;j<k*2;j++)
        {
            X = X*x;
            sumx[j] = sumx[j] + w*X;
        }
        
        X = 1.0f;
        for(j=0;j<k;j++)
        {
            X = X*x;
            sumyx[j] = sumyx[j] + w*X*y;
        }
        
        // X = 1.0;        
        // for(j=0;j<k;j++)
        // {
            // X = X*x;
            // sumx[j] = sumx[j] + w*X;
            // sumyx[j] = sumyx[j] + w*X*y;
        // }
        // for(j=k;j<k*2;j++)
        // {
            // X = X*x;
            // sumx[j] = sumx[j] + w*X;
        // }
        
        sumy = sumy + w*y;
        sum = sum + w;
    }
    
    // for(j=0;j<k*2;j++)
        // printf("sumx%d is %f\n",j+1,sumx[j]);
    // printf("\n\n");
    // for(j=0;j<k;j++)
        // printf("sumyx%d is %f\n",j+1,sumyx[j]);
    
    for(i=0;i<k;i++)
        mat->data[0][i] = sumx[k-i-1];
    
    mat->data[0][k] = sum;
    mat->data[0][k+1] = 0.0f-sumy;
    
    // #define DATA(x,y) data[y*(k+2)+x]
    
    for(j=1;j<=k;j++)
    {
        for(i=0;i<=k;i++)
            mat->data[j][i] = sumx[k+j-i-1];
        
        mat->data[j][i] = 0.0f-sumyx[j-1];
    }
    
    // for(j=0;j<=k;j++)
    // {
        // printf("%d:\t",j);
        // for(i=0;i<=k+1;i++)
            // printf("%f\t",DATA(i,j));
    
        // printf("\n");
    // }
    
    mLinearEquation(mat,A);
    
    free(sumx);
    free(sumyx);
    mMatrixRelease(mat);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  在二维空间上的多项式拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mPolyFit(float *XIn,float *YIn,int N,float *A,int k)
{
    int i,j;
    float *w;
    float error0,error1;
    float y;
    float X;
    int n;
    float y_max,y_min;
    
    mException((k==0),EXIT,"degree of polynomial is 0");
    mException((N<k+1),EXIT,"sample number is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    if(k == 1)
    {
        mLinearFit(XIn,YIn,N,A);
        return;
    }
    if(k == 2)
    {
        mParabolaFit(XIn,YIn,N,A);
        return;
    }
    
    y_max = YIn[0];
    y_min = YIn[0];
    for(i=0;i<N;i++)
    {
        y_max = (YIn[i]>y_max)?YIn[i]:y_max;
        y_min = (YIn[i]<y_min)?YIn[i]:y_min;
    }
    
    w = (float *)malloc(N*sizeof(float));
    
    mPolyFitLSM(XIn,YIn,N,A,k);
    
    error0 = (y_max-y_min)*(y_max-y_min);
    error1 = 0.0f;
    for(i=0;i<N;i++)
    {
        X = XIn[i];
        y = A[k-1]*X + A[k];
        for(j=k-2;j>=0;j--)
        {
            X = X*XIn[i];
            y = y+A[j]*X;
        }
    
        w[i] = YIn[i]-y;
        w[i] = w[i]*w[i];
        error1 = error1+w[i];
    }
    error1 = error1/(float)N;
    
    for(i=0;i<N;i++)
        w[i] = (w[i]>error1)?(1.0f/w[i]):1.0f;
    
    // printf("error0 is %f,error1 is %f\n",error0,error1);
    
    n = 0;
    while(ABS(error0-error1)>0.001f*(y_max-y_min))
    {
        n=n+1;
        if(n>=100)
            break;
        
        mPolyFitWeight(XIn,YIn,w,N,A,k);
        
        error0 = error1;
        error1 = 0.0f;
        for(i=0;i<N;i++)
        {
            X = XIn[i];
            y = A[k-1]*X + A[k];
            for(j=k-2;j>=0;j--)
            {
                X = X*XIn[i];
                y = y+A[j]*X;
            }
            
            w[i] = YIn[i]-y;
            w[i] = w[i]*w[i];
            error1 = error1+w[i];
        }
        error1 = error1/(float)N;
        
        for(i=0;i<N;i++)
            w[i] = (w[i]>error1)?(1.0f/w[i]):1.0f;
        
        // printf("error0 is %f,error1 is %f\n",error0,error1);
    }
    
    free(w);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  用最小二乘法作二维空间指数拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]*exp(A[1]*X)
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mExpFitLSM(float *XIn,float *YIn,int N,float *A)
{
    float *lny;
    int i;
    float answer[2];
    
    lny = mMalloc(sizeof(float)*N);
    for(i=0;i<N;i++)
        lny[i] = log(YIn[i]);
    
    mLinearFitLSM(XIn,lny,N,answer);
    
    A[1] = answer[0];
    A[0] = exp(answer[1]);
    
    mFree(lny);
}
/////////////////////////////////////////////////////////
// 接口功能:
//  作二维空间指数拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]*exp(A[1]*X)
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mExpFit(float *XIn,float *YIn,int N,float *A)
{
    float *lny;
    int i;
    float answer[2];
    
    lny = mMalloc(sizeof(float)*N);
    for(i=0;i<N;i++)
        lny[i] = log(YIn[i]);
    
    mLinearFit(XIn,lny,N,answer);
    
    A[1] = answer[0];
    A[0] = exp(answer[1]);
    
    mFree(lny);
}
/////////////////////////////////////////////////////////
// 接口功能:
//  用最小二乘法作二维空间对数拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]*ln(X) + A[1]
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mLnFitLSM(float *XIn,float *YIn,int N,float *A)
{
    float *lnx;
    int i;
    
    lnx = mMalloc(sizeof(float)*N);
    for(i=0;i<N;i++)
        lnx[i] = log(XIn[i]);
    
    mLinearFitLSM(lnx,YIn,N,A);
    
    mFree(lnx);
}
/////////////////////////////////////////////////////////
// 接口功能:
//  作二维空间对数拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]*ln(X) + A[1]
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mLnFit(float *XIn,float *YIn,int N,float *A)
{
    float *lnx;
    int i;
    
    lnx = mMalloc(sizeof(float)*N);
    for(i=0;i<N;i++)
        lnx[i] = log(XIn[i]);
    
    mLinearFit(lnx,YIn,N,A);
    
    mFree(lnx);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  用最小二乘法作二维空间幂拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]*X^A[1]
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mPowerFitLSM(float *XIn,float *YIn,int N,float *A)
{
    float *lnx,*lny;
    int i;
    float answer[2];
    
    lnx = mMalloc(sizeof(float)*N*2);
    lny = lnx+N;
    for(i=0;i<N;i++)
    {
        lnx[i] = log(XIn[i]);
        lny[i] = log(YIn[i]);
    }
    
    mLinearFitLSM(lnx,lny,N,answer);
    
    A[1] = answer[0];
    A[0] = exp(answer[1]);
    
    mFree(lnx);
}
/////////////////////////////////////////////////////////
// 接口功能:
//  作二维空间幂拟合
//
// 参数：
//  (I)XIn(NO) - 待拟合点的横坐标
//  (I)YIn(NO) - 待拟合点的纵坐标
//  (I)N(NO) - 待拟合点的数量
//  (O)A(NO) - 拟合结果：Y=A[0]*X^A[1]
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mPowerFit(float *XIn,float *YIn,int N,float *A)
{
    float *lnx,*lny;
    int i;
    float answer[2];
    
    lnx = mMalloc(sizeof(float)*N*2);
    lny = lnx+N;
    for(i=0;i<N;i++)
    {
        lnx[i] = log(XIn[i]);
        lny[i] = log(YIn[i]);
    }
    
    mLinearFit(lnx,lny,N,answer);
    
    A[1] = answer[0];
    A[0] = exp(answer[1]);
    
    mFree(lnx);
}




/*
struct ExpFitData
{
    float *x;
    float *y;
    float *x_exp;

    int n;
};
float ExpFitError(void *data_in,float *para)
{
    int i;
    float error,sum;
    float k,a;
    struct ExpFitData *data;
    
    data = (struct ExpFitData *)data_in;
    k = para[0];
    a = para[1];
    sum = 0.0;
    for(i=0;i<data->n;i++)
    {
        data->x_exp[i] = exp(data->x[i]*a);
        error = k*(data->x_exp[i]) - (data->y[i]);
        sum = sum + error*error;
    }
    // printf("sum is %f\n",sum);
    return sum;
}
float ExpFitKDiff(void *data_in,float *para)
{
    int i;
    float error,sum;
    float k,a;
    struct ExpFitData *data;
    
    data = (struct ExpFitData *)data_in;
    k = para[0];
    a = para[1];
    sum = 0.0;
    for(i=0;i<data->n;i++)
    {
        error = k*data->x_exp[i] - (data->y[i]);
        sum = sum + error*data->x_exp[i];
    }
    sum = (sum + sum)/((float)i);
    // printf("k_diff is %f\n",sum);
    return sum;
}
float ExpFitADiff(void *data_in,float *para)
{
    int i;
    float error,sum;
    float k,a;
    struct ExpFitData *data;
    
    data = (struct ExpFitData *)data_in;
    k = para[0];
    a = para[1];
    sum = 0.0;
    for(i=0;i<data->n;i++)
    {
        error = k*data->x_exp[i] - (data->y[i]);
        sum = sum + error*k*(data->x_exp[i])*(data->x[i]);
    }
    sum = (sum + sum)/((float)i);
    // printf("a_diff is %f\n",sum);
    return sum;
}
void mSteepestDescend(float (*func)(void *,float *),float (**differential_func)(void *,float *),void *data,float *x,float *y,int n);
void mExpFit(float *XIn,float *YIn,int N,float *para_k,float *para_a)
{
    struct ExpFitData *data;
    float (*diff_func[2])(void *,float *);
    float para[2];
    float error;
    
    data = (struct ExpFitData *)mMalloc(sizeof(struct ExpFitData));
    
    data->x = XIn;
    data->y = YIn;
    data->x_exp = (float *)mMalloc(sizeof(float)*N);
    data->n = N;
    
    diff_func[0] = ExpFitKDiff;
    diff_func[1] = ExpFitADiff;
    
    mSteepestDescend(ExpFitError,diff_func,(void *)data,para,&error,2);
    
    *para_k = para[0];
    *para_a = para[1];
    
    mFree(data->x_exp);
    mFree(data);
}
*/
    
