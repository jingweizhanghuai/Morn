/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

#include <cblas.h>
#ifdef MORN_USE_CL
#include <CL/cl.h>
#include <clBLAS.h>
cl_context mDeviceContext(int device);
cl_command_queue mDeviceQueue(int device);
#endif

#define MORN_NO_TRANS 0
#define MORN_TRANS    1

struct HandleCLBlas
{
    int clblas_setup;
};
void endCLBlas(struct HandleCLBlas *handle)
{
    NULL;
    #ifdef MORN_USE_CL
    if(handle->clblas_setup) clblasTeardown();
    #endif
}
#define HASH_CLBlas 0x3b4b7c3a
static struct HandleCLBlas *morn_clblas_handle=NULL;
void SetupCLBlas()
{
    MHandle *hdl=mHandle(mMornObject(NULL,DFLT),CLBlas);
    morn_clblas_handle=(struct HandleCLBlas *)(hdl->handle);
    if(hdl->valid==0)
    {
        #ifdef MORN_USE_CL
        mException(clblasSetup()!=CL_SUCCESS,EXIT,"CLBlas not work");
        morn_clblas_handle->clblas_setup=1;
        #endif
        hdl->valid=1;
    }
}

void mSgemm(int device,int a_trans,int b_trans,int m,int n,int k,float alpha,MMemoryBlock *a,int sa,MMemoryBlock *b,int sb,float beta,MMemoryBlock *c,int sc)
{
    #ifdef MORN_USE_CL
    if(device!=MORN_HOST)
    {
        a_trans=(a_trans==MORN_TRANS)?clblasTrans:clblasNoTrans;
        b_trans=(b_trans==MORN_TRANS)?clblasTrans:clblasNoTrans;
        
        if(morn_clblas_handle==NULL) SetupCLBlas();
        
        device = c->device;
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
        return;
    }
    #endif
    a_trans=(a_trans==MORN_TRANS)?CblasTrans:CblasNoTrans;
    b_trans=(b_trans==MORN_TRANS)?CblasTrans:CblasNoTrans;
    cblas_sgemm(CblasRowMajor,a_trans,b_trans,m,n,k,alpha,a->data,sa,b->data,sb,beta,c->data,sc);
}

char *saxpby=mString(
__kernel void saxpby(__global const float* a,__global float* b,const float alpha,const float beta,const int sa,const int sb)
{
    const int idx = get_global_id(0);
    const int ia = idx*sa;
    const int ib = idx*sb;
    b[ib]=a[ia]*alpha+b[ib]*beta;
});

void mSaxpby(int device,int n,float alpha,MMemoryBlock *a,int sa,float beta,MMemoryBlock *b,int sb)
{
    #ifdef MORN_USE_CL
    if(device!=MORN_HOST)
    {
        mCLFunction(saxpby,CLSIZE(n),CLIN(a),CLINOUT(b),CLPARA(&alpha,sizeof(float)),CLPARA(&beta,sizeof(float)),CLPARA(&sa,sizeof(int)),CLPARA(&sb,sizeof(int)));
        return;
    }
    #endif
    cblas_saxpby(n,alpha,a->data,sa,beta,b->data,sb);
}