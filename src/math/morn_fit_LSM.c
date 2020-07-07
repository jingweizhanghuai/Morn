/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
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

void mLinearLSMFit(MList *list,MImagePoint *p1,MImagePoint *p2)
{
    mException((list==NULL)||(p1==NULL)||(p2==NULL),EXIT,"invalid input list");
    mException((list->num<2),EXIT,"sample number (which <2) is not enough.");

    float **p = (float **)list->data;
    double sumx = 0.0f;double sumy = 0.0f;
    double sumxy= 0.0f;double sumx2= 0.0f;
    float min=p[0][0];float max=p[0][0];
    for(int i=0;i<list->num;i++)
    {
        sumx = sumx + p[i][0];sumx2 = sumx2 + p[i][0]*p[i][0];
        sumy = sumy + p[i][1];sumxy = sumxy + p[i][0]*p[i][1];
        min=MIN(p[i][0],min);max=MAX(p[i][0],max);
    }
    double N = (double)(list->num);
    double k = ((N*sumxy)-(sumx*sumy))/((N*sumx2)-(sumx*sumx));
    double b = ((sumx2*sumy)-(sumx*sumxy))/((N*sumx2)-(sumx*sumx));
    p1->x=min;p1->y=k*min+b;
    p2->x=max;p2->y=k*max+b;
}



void mPiecewiseLinearFit(float *XIn,float *YIn,int N,int piece_num,MList *list)
{
    int i;
    // float *thresh = mMalloc(piece_num*8*sizeof(int));
    float x_min=XIn[0],x_max=XIn[0],y_min=YIn[0],y_max=YIn[0];
    for(i=1;i<N;i++)
    {
        x_min = MIN(x_min,XIn[i]);
        x_max = MAX(x_max,XIn[i]);
        y_min = MIN(y_min,YIn[i]);
        y_max = MAX(y_max,YIn[i]);
    }
    int   *num  =(int   *)mMalloc(piece_num*8*sizeof(int)  );memset(  num,0,piece_num*8*sizeof(int)  );
    float *sumx =(float *)mMalloc(piece_num*8*sizeof(float));memset( sumx,0,piece_num*8*sizeof(float));
    float *sumy =(float *)mMalloc(piece_num*8*sizeof(float));memset( sumy,0,piece_num*8*sizeof(float));
    float *sumxy=(float *)mMalloc(piece_num*8*sizeof(float));memset(sumxy,0,piece_num*8*sizeof(float));
    float *sumx2=(float *)mMalloc(piece_num*8*sizeof(float));memset(sumx2,0,piece_num*8*sizeof(float));
    
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
        // float a[2];
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
    info1 = (struct LineInfo *)(list->data[0]);
    if(flag) {info1->ps.x = x_min;info1->ps.y = info1->k*x_min +info1->b;}
    else     {info1->ps.y = y_min;info1->ps.x =(y_min-info1->b)/info1->k;}
    for(i=0;i<m-1;m=m+1)
    {
        info1 = (struct LineInfo *)(list->data[m  ]);
        info2 = (struct LineInfo *)(list->data[m+1]);
        float x = (info2->b-info1->b)/(info1->k-info2->k);
        float y = info1->k*x+info1->b;
        info1->pe.x = x;info1->pe.y=y;
        info2->ps.x = x;info2->ps.y=y;
    }
    info2 = (struct LineInfo *)(list->data[m-1]);
    if(flag) {info2->pe.x = x_max;info2->pe.y = info2->k*x_max +info2->b;}
    else     {info2->pe.y = y_max;info2->pe.x =(y_max-info2->b)/info2->k;}
}

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















void mPolyFitLSM(float *XIn,float *YIn,int N,float *A,int k)
{
    mException((k==0),EXIT,"degree of polynomial is 0");
    mException((N<k+1),EXIT,"sample number is not enough.");
    mException((INVALID_POINTER(XIn)||(INVALID_POINTER(YIn))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");
    
    double *sumx = (double *)malloc(k*2*sizeof(double));memset( sumx,0,k*2*sizeof(double));
    double *sumyx= (double *)malloc(k  *sizeof(double));memset(sumyx,0,k  *sizeof(double));
    double  sumy = 0;
    MMatrix *mat = mMatrixCreate(k+1,k+2,NULL);
    for(int i=0;i<N;i++)
    {
        double x = XIn[i];double y = YIn[i];double data;
        data = 1.0f;for(int j=0;j<k*2;j++){data = data*x; sumx[j] = sumx[j] + data;}
        data = y;   for(int j=0;j<k  ;j++){data = data*x;sumyx[j] =sumyx[j] + data;}
        sumy = sumy + y;
    }
    // for(j=0;j<k*2;j++)printf("sumx%d is %f\n",j+1,sumx[j]);
    // for(j=0;j<k;j++)printf("sumyx%d is %f\n",j+1,sumyx[j]);

    mat->data[0][0] = (float)N;
    for(int i=0;i<k;i++) mat->data[0][i+1] = sumx[i];
    mat->data[0][k+1] = 0.0-sumy;
    for(int j=0;j<k;j++)
    {
        for(int i=0;i<=k;i++) mat->data[j+1][i] = sumx[j+i];
        mat->data[j+1][k+1] = 0.0-sumyx[j];
    }
    
    mLinearEquation(mat,A);
    
    free(sumx);
    free(sumyx);
    mMatrixRelease(mat);
}

float _polycurve(float x,float *para)
{
    int k=para[0];para=para+1;
    float data=1;float y=para[0];
    for(int i=1;i<=k;i++) {data=data*x;y+=para[i]*data;}
    return y;
}
struct HandlePolyLSMFit
{
    double *sumx;
    double *sumyx;
    MMatrix *mat;
};
void endPolyLSMFit(void *info)
{
    struct HandlePolyLSMFit *handle = (struct HandlePolyLSMFit *)info;
    if(handle->sumx !=NULL) mFree(handle->sumx );
    if(handle->sumyx!=NULL) mFree(handle->sumyx);
    if(handle->mat  !=NULL) mMatrixRelease(handle->mat);
};
#define HASH_PolyLSMFit 0xa258594a
void mPolyLSMFit(MList *list,MImageCurve *curve,int k)
{
    mException((list==NULL),EXIT,"invalid list input");
    mException((k<=0)||(k>15)||(list->num<=k),EXIT,"invalid k,which is %d\n",k);
    MHandle *hdl=mHandle(list,PolyLSMFit);
    struct HandlePolyLSMFit *handle = (struct HandlePolyLSMFit *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->sumx !=NULL) mFree(handle->sumx ); handle->sumx = (double *)mMalloc(30*sizeof(double));
        if(handle->sumyx!=NULL) mFree(handle->sumyx); handle->sumyx= (double *)mMalloc(15*sizeof(double));
        if(handle->mat==NULL) handle->mat = mMatrixCreate(16,17,NULL);
    }
    double *sumx = handle->sumx ; memset( sumx,0,k*2*sizeof(double));
    double *sumyx= handle->sumyx; memset(sumyx,0,k  *sizeof(double));
    double sumy=0;
    
    int type = curve->type;

    float *p = (float *)(list->data[0]);
    float min,max;
    if(type<=0) {min=p[0];max=p[0];} else {min=p[1];max=p[1];}
    for(int i=0;i<list->num;i++)
    {
        p = (float *)(list->data[i]);
        double x,y,data;
        if(type<=0) {x=p[0];y=p[1];} else {x=p[1];y=p[0];}
        min=MIN(min,x);max=MAX(max,x);
        
        data = 1.0f;for(int j=0;j<k*2;j++){data = data*x; sumx[j] = sumx[j] + data;}
        data = y;   for(int j=0;j<k  ;j++){data = data*x;sumyx[j] =sumyx[j] + data;}
        sumy = sumy + y;
    }

    mMatrixRedefine(handle->mat,k+1,k+2,handle->mat->data);
    MMatrix *mat = handle->mat;
    mat->data[0][0] = (float)((double)(list->num)/sumx[1]);
    for(int i=0;i<k;i++) mat->data[0][i+1] = (float)(sumx[i]/sumx[1]);
    mat->data[0][k+1] = 0.0f-(float)(sumy/sumx[1]);
    for(int j=0;j<k;j++)
    {
        for(int i=0;i<=k;i++) mat->data[j+1][i] = (float)(sumx[j+i]/sumx[j+2]);
        mat->data[j+1][k+1] = 0.0f-(float)(sumyx[j]/sumx[j+2]);
    }
    
    curve->para[0]=k;
    mLinearEquation(mat,curve->para+1);
    curve->curve = _polycurve;
    if(type<=0) {curve->v1.x=min;curve->v1.y=_polycurve(min,curve->para);curve->v2.x=max;curve->v2.y=_polycurve(max,curve->para);}
    else        {curve->v1.y=min;curve->v1.x=_polycurve(min,curve->para);curve->v2.y=max;curve->v2.x=_polycurve(max,curve->para);}
}








































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

void mExpFitLSM(float *XIn,float *YIn,int N,float *A)
{
    float *lny = (float *)mMalloc(sizeof(float)*N);
    for(int i=0;i<N;i++) lny[i] = log(YIn[i]);
    
    float answer[2]; mLinearFitLSM(XIn,lny,N,answer);
    A[1] = answer[0]; A[0] = exp(answer[1]);
    
    mFree(lny);
}

void mExpFit(float *XIn,float *YIn,int N,float *A)
{
    float *lny = (float *)mMalloc(sizeof(float)*N);
    for(int i=0;i<N;i++) lny[i] = log(YIn[i]);
    
    float answer[2];mLinearFit(XIn,lny,N,answer);
    
    A[1] = answer[0]; A[0] = exp(answer[1]);
    
    mFree(lny);
}

void mLnFitLSM(float *XIn,float *YIn,int N,float *A)
{
    float *lnx = (float *)mMalloc(sizeof(float)*N);
    for(int i=0;i<N;i++) lnx[i] = log(XIn[i]);
    
    mLinearFitLSM(lnx,YIn,N,A);
    
    mFree(lnx);
}

void mLnFit(float *XIn,float *YIn,int N,float *A)
{
    float *lnx = (float *)mMalloc(sizeof(float)*N);
    for(int i=0;i<N;i++) lnx[i] = log(XIn[i]);
    
    mLinearFit(lnx,YIn,N,A);
    
    mFree(lnx);
}

void mPowerFitLSM(float *XIn,float *YIn,int N,float *A)
{
    float *lnx = (float *)mMalloc(sizeof(float)*N*2);
    float *lny = lnx+N;
    for(int i=0;i<N;i++){lnx[i] = log(XIn[i]);lny[i] = log(YIn[i]);}
    
    float answer[2];mLinearFitLSM(lnx,lny,N,answer);
    A[1] = answer[0]; A[0] = exp(answer[1]);
    
    mFree(lnx);
}

void mPowerFit(float *XIn,float *YIn,int N,float *A)
{
    float *lnx = (float *)mMalloc(sizeof(float)*N*2);
    float *lny = lnx+N;
    for(int i=0;i<N;i++){lnx[i] = log(XIn[i]); lny[i] = log(YIn[i]);}
    
    float answer[2];mLinearFit(lnx,lny,N,answer);
    A[1] = answer[0]; A[0] = exp(answer[1]);
    
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