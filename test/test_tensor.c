// gcc -O2 -fopenmp  test_tensor.c  -o test_tensor.exe -I ..\include\ -I ../lib/include -L ..\lib\x64_mingw -lmorn -lclblas -lopencl -lstdc++ -lopenblas
#define MORN_USE_CL 1
#include "morn_tensor.h"

#define mString(x) #x

char *cl_source = mString(
__kernel void test1(__global float* A,float B,__global float* C)
{
    size_t idx = get_global_id(0);
    C[idx] = A[idx]+B;
}
);

int main()
{
    MTensor *tns1 = mTensorCreate(1,10,100,100,NULL,MORN_CL_GPU(0));
    MTensor *tns2 = mTensorCreate(1,10,100,100,NULL,MORN_CL_GPU(0));
    MTensor *tns3 = mTensorCreate(1,10,100,100,NULL,MORN_CL_GPU(0));
 
    for(int i=0;i<10*100*100;i++)
    {
        tns1->data[0][i]=mRand(0,1000)/100.0f;
        tns2->data[0][i]=mRand(0,1000)/100.0f;
    }
    printf("\na:");for(int i=0;i<50;i++) printf("%f,",tns1->data[0][i]);
    printf("\nb:");for(int i=0;i<50;i++) printf("%f,",tns2->data[0][i]);

    MMemoryBlock *m1=mMemoryBlock(tns1->data[0]);
    MMemoryBlock *m2=mMemoryBlock(tns2->data[0]);
    MMemoryBlock *m3=mMemoryBlock(tns3->data[0]);

    float bb = 1.0f;
    mCLFunction(cl_source,CLIN(m1),CLPARA(&bb,sizeof(float)),CLOUT(m3));

    mMemoryBlockRead(m3);
    printf("\nc:");for(int i=0;i<50;i++) printf("%f,",tns3->data[0][i]);

    mCLFunction(cl_source,CLIN(m2),CLPARA(&bb,sizeof(float)),CLOUT(m3));

    mMemoryBlockRead(m3);
    printf("\nc:");for(int i=0;i<50;i++) printf("%f,",tns3->data[0][i]);
    
    mTensorRelease(tns1);
    mTensorRelease(tns2);
    mTensorRelease(tns3);
}