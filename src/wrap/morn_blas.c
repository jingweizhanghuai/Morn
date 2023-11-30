/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"
#include "morn_math.h"

#define MORN_NO_TRANS 0
#define MORN_TRANS    1

#if defined MORN_USE_CL
#include <CL/cl.h>
#include <clBLAS.h>
cl_context mDeviceContext(int device);
cl_command_queue mDeviceQueue(int device);

struct HandleCLBlas
{
    int clblas_setup;
};
void endCLBlas(struct HandleCLBlas *handle)
{
    if(handle->clblas_setup) clblasTeardown();
}
#define HASH_CLBlas 0x3b4b7c3a
static struct HandleCLBlas *morn_clblas_handle=NULL;
void SetupCLBlas()
{
    MHandle *hdl=mHandle(mMornObject(NULL,DFLT),CLBlas);
    morn_clblas_handle=(struct HandleCLBlas *)(hdl->handle);
    if(hdl->valid==0)
    {
        mException(clblasSetup()!=CL_SUCCESS,EXIT,"CLBlas not work");
        morn_clblas_handle->clblas_setup=1;
        hdl->valid=1;
    }
}

void mSgemm(int a_trans,int b_trans,int m,int n,int k,float alpha,MMemoryBlock *a,int sa,MMemoryBlock *b,int sb,float beta,MMemoryBlock *c,int sc)
{
    a_trans=(a_trans==MORN_TRANS)?clblasTrans:clblasNoTrans;
    b_trans=(b_trans==MORN_TRANS)?clblasTrans:clblasNoTrans;
    
    if(morn_clblas_handle==NULL) SetupCLBlas();
    
    int device = c->device;
    mMemoryBlockCopy(a,device);
    mMemoryBlockCopy(b,device);
    cl_event a_event=(cl_event)(a->cl_evt);
    cl_event b_event=(cl_event)(b->cl_evt);
    cl_event c_event=(cl_event)(c->cl_evt);

    cl_event event_list[2] = {a_event,b_event};
    cl_command_queue queue=mDeviceQueue(device);
    int ret=clblasSgemm(clblasRowMajor,a_trans,b_trans,m,n,k,alpha,a->cl_data,0,sa,b->cl_data,0,sb,beta,c->cl_data,0,sc,1,&queue,2,event_list,&c_event);
    mException(ret!=CL_SUCCESS,EXIT,"clblas error");
    
    c->flag = MORN_DEVICE;
}

char *saxpby=mString(
__kernel void saxpby(__global const float* a,__global float* b,const float alpha,const float beta)
{
    const int idx = get_global_id(0);
    b[idx]=a[idx]*alpha+b[idx]*beta;
});

void mSaxpby(int n,float alpha,MMemoryBlock *a,float beta,MMemoryBlock *b)
{
    mCLFunction(saxpby,CLSIZE(n),CLIN(a),CLINOUT(b),CLPARA(&alpha,sizeof(float)),CLPARA(&beta,sizeof(float)));
}

#elif defined MORN_USE_CBLAS
#include <cblas.h>
void mSgemm(int a_trans,int b_trans,int m,int n,int k,float alpha,MMemoryBlock *a,int sa,MMemoryBlock *b,int sb,float beta,MMemoryBlock *c,int sc)
{
    a_trans=(a_trans==MORN_TRANS)?CblasTrans:CblasNoTrans;
    b_trans=(b_trans==MORN_TRANS)?CblasTrans:CblasNoTrans;
    cblas_sgemm(CblasRowMajor,a_trans,b_trans,m,n,k,alpha,a->data,sa,b->data,sb,beta,c->data,sc);
}
void mSaxpby(int n,float alpha,MMemoryBlock *a,float beta,MMemoryBlock *b)
{
    cblas_saxpby(n,alpha,a->data,1,beta,b->data,1);
}

#else

struct HandleBlas
{
    MMatrix *src1;
    MMatrix *src2;
    MMatrix *dst;
    
    MMatrix *buff1;
    MMatrix *buff2;
    MMatrix *buff3;
};
struct HandleBlas *morn_blas_handle=NULL;
#define HASH_Blas 0x50b386ed
void endBlas(struct HandleBlas *handle)
{
    if(handle->src1 !=NULL) mMatrixRelease(handle->src1 );
    if(handle->src2 !=NULL) mMatrixRelease(handle->src2 );
    if(handle->dst  !=NULL) mMatrixRelease(handle->dst  );
    
    if(handle->buff1!=NULL) mMatrixRelease(handle->buff1);
    if(handle->buff2!=NULL) mMatrixRelease(handle->buff2);
    if(handle->buff3!=NULL) mMatrixRelease(handle->buff3);

    morn_blas_handle=NULL;
}

struct HandleBlas *BlasInit()
{
    if(morn_blas_handle!=NULL) return morn_blas_handle;
    MHandle *hdl=mHandle("Morn",Blas);
    struct HandleBlas *handle=hdl->handle;
    handle->src1=mMatrixCreate();
    handle->src2=mMatrixCreate();
    handle->dst =mMatrixCreate();
    
    handle->buff1=mMatrixCreate(2000,2000);
    handle->buff2=mMatrixCreate(2000,2000);
    handle->buff3=mMatrixCreate(2000,2000);
    
    hdl->valid=1;
    morn_blas_handle =handle;
    return handle;
}

void mSgemm(int a_trans,int b_trans,int m,int n,int k,float alpha,MMemoryBlock *a,int sa,MMemoryBlock *b,int sb,float beta,MMemoryBlock *c,int sc)
{
    struct HandleBlas *handle=BlasInit();
    
    float **pa=mMalloc(m*sizeof(float *));sa=sa*sizeof(float);
    float **pc=mMalloc(m*sizeof(float *));sc=sc*sizeof(float);
    for(int i=0;i<m;i++) {pa[i]=a->data+i*sa;pc[i]=c->data+i*sc;}
    
    float **pb=mMalloc(k*sizeof(float *));sb=sb*sizeof(float);
    for(int i=0;i<k;i++) pb[i]=b->data+i*sb;
    
    mMatrixRedefine(handle->src1,m,k,pa);
    mMatrixRedefine(handle->src2,k,n,pb);
    mMatrixRedefine(handle->dst ,m,n,pc);
    
    MMatrix *src1,*src2,*dst;
    
    if(a_trans) {mMatrixTranspose(handle->src1,handle->buff1);src1=handle->buff1;}else src1=handle->src1;
    if(b_trans) {mMatrixTranspose(handle->src2,handle->buff2);src2=handle->buff2;}else src2=handle->src2;
    if(beta!=0) {                                             dst =handle->buff3;}else dst =handle->dst ;                                             
    
    mMatrixMul(src1,src2,dst);
    
    if(alpha!=1) m_MatrixScale(dst,dst,alpha,0);
    if(beta !=0)
    {
        m_MatrixScale(handle->dst,handle->dst,beta,0);
        mMatrixAdd(dst,handle->dst,handle->dst);
    }
}
void mSaxpby(int n,float alpha,MMemoryBlock *a,float beta,MMemoryBlock *b)
{
    float *pa=a->data;
    float *pb=b->data;
    int i;for(i=0;i<n-3;i++)
    {
        pb[i  ]=pa[i  ]*alpha+pb[i  ]*beta;
        pb[i+1]=pa[i+1]*alpha+pb[i+1]*beta;
        pb[i+2]=pa[i+2]*alpha+pb[i+2]*beta;
        pb[i+3]=pa[i+3]*alpha+pb[i+3]*beta;
    }
    for(;i<n;i++) pb[i]=pa[i]*alpha+pb[i]*beta;
}
#endif


