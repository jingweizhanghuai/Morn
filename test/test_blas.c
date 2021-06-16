// gcc -O2 -fopenmp  test_blas.c  -o test_blas.exe -I ..\include\ -L ..\lib\x64_mingw -lmorn -lopenblas -lclblas -lopencl -lstdc++
#include "morn_math.h"

#define MORN_NO_TRANS 0
#define MORN_TRANS    1
void mSgemm(int device,int a_trans,int b_trans,int m,int n,int k,float alpha,MMemoryBlock *a,int sa,MMemoryBlock *b,int sb,float beta,MMemoryBlock *c,int sc);

MMemoryBlock *mMatrixMemory(MMatrix *mat);

void main1()
{
    MMatrix *a = mMatrixCreate(2000,3000,NULL,MORN_CL_GPU(0));
    MMatrix *b = mMatrixCreate(3000,4000,NULL,MORN_CL_GPU(0));
    MMatrix *c = mMatrixCreate(2000,4000,NULL,MORN_CL_GPU(0));
    MMatrix *d = mMatrixCreate(2000,4000,NULL,MORN_CL_GPU(0));
    MMatrix *e = mMatrixCreate(2000,4000,NULL,MORN_CL_GPU(0));
    for(int j=0;j<a->row;j++)for(int i=0;i<a->col;i++) a->data[j][i]=mRand(-1000,1000)/1000.0;
    for(int j=0;j<b->row;j++)for(int i=0;i<b->col;i++) b->data[j][i]=mRand(-1000,1000)/1000.0;

    mTimerBegin("matrixmul");
    mMatrixMul(a,b,c);
    mTimerEnd("matrixmul");
    for(int i=0;i<50;i++) printf("%f,",c->data[0][i]);
    printf("\n\n");
    
    mTimerBegin("device");
    mSgemm(MORN_DEVICE,MORN_NO_TRANS,MORN_NO_TRANS,2000,4000,3000,1,mMatrixMemory(a),3000,mMatrixMemory(b),4000,0,mMatrixMemory(d),4000);
    mTimerEnd("device");
    mMemoryBlockRead(mMatrixMemory(d));
    for(int i=0;i<50;i++) printf("%f,",d->data[0][i]);
    printf("\n\n");
    
    mTimerBegin("host");
    mSgemm(  MORN_HOST,MORN_NO_TRANS,MORN_NO_TRANS,2000,4000,3000,1,mMatrixMemory(a),3000,mMatrixMemory(b),4000,0,mMatrixMemory(e),4000);
    mTimerEnd("host");
    for(int i=0;i<50;i++) printf("%f,",e->data[0][i]);
    printf("\n\n");
    
    mMatrixRelease(a);
    mMatrixRelease(b);
    mMatrixRelease(c);
    mMatrixRelease(d);
    mMatrixRelease(e);
}

char *saxpby0=mString(
__kernel void saxpby0(__global const float* a,__global float* b,const float alpha,const float beta,const int sa,const int sb)
{
    const int idx = get_global_id(0);
    const int ia = idx*sa;
    const int ib = idx*sb;
    b[ib]=a[ia]*alpha+b[ib]*beta;
});

void main()
{
    MMemoryBlock *a = mMemoryBlockCreate(1000*1000*sizeof(float),MORN_CL_GPU(0)); float *pa=a->data;
    MMemoryBlock *b = mMemoryBlockCreate(1000*1000*sizeof(float),MORN_CL_GPU(0)); float *pb=b->data;
    MMemoryBlock *c = mMemoryBlockCreate(1000*1000*sizeof(float),MORN_CL_GPU(0)); float *pc=c->data;
    for(int i=0;i<1000*1000;i++)
    {
        pa[i]=mRand(-1000,1000)/1000.0;
        pb[i]=mRand(-1000,1000)/1000.0;
        pc[i]=pb[i];
    }

    float alpha=2.0f;float beta=-2.0f;
    // int sa=2;int sb=2;
    mTimerBegin();
    // for(int i=0;i<100;i++)
    {
        mSaxpby(a->device,1000*1000/2,alpha,a,2,beta,b,2);
        // mCLFunction(saxpby0,CLSIZE(1000*1000/2),CLIN(a),CLINOUT(b),CLPARA(alpha),CLPARA(beta),CLPARA(sa),CLPARA(sb));
    }
    mTimerEnd();
    mMemoryBlockRead(b);
    for(int i=0;i<100;i++)
    {
        printf("a=%f,b=%f,out=%f,%f\n",pa[i],pc[i],pb[i],(pa[i]-pc[i])*2);
    }

    mMemoryBlockRelease(a);
    mMemoryBlockRelease(b);
    mMemoryBlockRelease(c);
    
}