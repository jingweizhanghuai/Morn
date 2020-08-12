#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cuda_runtime.h"
#include "cublas_v2.h"
#include "cblas.h"
#include "morn_math.h"

// gcc -O2 -fopenmp test_matrix.c -I ..\include\ -I ..\lib\include\ -I C:\ProgramFiles\NVIDIACUDA\include\ -L ..\lib\x64\mingw\ -L C:\ProgramFiles\NVIDIACUDA\lib\x64\ -lmorn -lopenblas -l:cublas.lib -o test_matrix.exe

int main1()
{
    float a[ 6] = {1,2,3,4,5,6};
    float b[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    float c[8];
    float d[8];

    MMatrix *mat1=mMatrixCreate(2,3,NULL); mMatrixData(mat1,a);
    MMatrix *mat2=mMatrixCreate(3,4,NULL); mMatrixData(mat2,b);
    MMatrix *mat3=mMatrixCreate(DFLT,DFLT,NULL);

    mMatrixMul(mat1,mat2,mat3);
    PrintMat(mat1);
    PrintMat(mat2);
    PrintMat(mat3);

    // cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
    //             2,4,3,
    //             1.0,
    //             a,3,
    //             b,4,
    //             0.0,
    //             c,4);
    // for(int i=0;i<8;i++) printf("%f,",c[i]);printf("\n");

    float *d_A,*d_B,*d_C;
    cudaMalloc((void**)&d_A,sizeof(float)*6);
    cudaMalloc((void**)&d_B,sizeof(float)*12);
    cudaMalloc((void**)&d_C,sizeof(float)*8);

    cublasHandle_t handle;
    cublasCreate(&handle);
    cudaMemcpy(d_A,a,sizeof(float)*6,cudaMemcpyHostToDevice);
    cudaMemcpy(d_B,b,sizeof(float)*12,cudaMemcpyHostToDevice);
    
    // cudaMemcpy(c,d_A,sizeof(float)*8,cudaMemcpyDeviceToHost);
    // for(int i=0;i<8;i++) printf("%f,",c[i]);printf("\n");
    
    float alpha = 1, beta = 0;

    cublasSgemm_v2(
          handle,
          CUBLAS_OP_N,
          CUBLAS_OP_N,
                4,2,3,
                &alpha,
                d_B,4,
                d_A,3,
                &beta,
                d_C,4);
    cudaMemcpy(c,d_C,sizeof(float)*8,cudaMemcpyDeviceToHost);
    for(int i=0;i<8;i++) printf("%f,",c[i]);printf("\n");

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cublasDestroy(handle);
    
    mMatrixRelease(mat1);
    mMatrixRelease(mat2);
    mMatrixRelease(mat3);

    cl_int err;
    cl_platform_id platform = 0;
    cl_device_id device = 0;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_context ctx = 0;
    cl_command_queue queue = 0;
    cl_mem bufA, bufB, bufC;
    cl_event event = NULL;
    int ret = 0;

    /* Setup OpenCL environment. */
    err = clGetPlatformIDs( 1, &platform, NULL );
    err = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );

    props[1] = (cl_context_properties)platform;
    ctx = clCreateContext( props, 1, &device, NULL, NULL, &err );
    queue = clCreateCommandQueue( ctx, device, 0, &err );

    /* Setup clBLAS */
    err = clblasSetup( );

    /* Prepare OpenCL memory objects and place matrices inside them. */
    bufA = clCreateBuffer( ctx, CL_MEM_READ_ONLY, 6* sizeof(float),NULL, &err );
    bufB = clCreateBuffer( ctx, CL_MEM_READ_ONLY, 12* sizeof(float),NULL, &err );
    bufC = clCreateBuffer( ctx, CL_MEM_READ_WRITE, 8* sizeof(float),NULL, &err );
                          

    err = clEnqueueWriteBuffer( queue, bufA, CL_TRUE, 0,6 * sizeof(float), a, 0, NULL, NULL );
    err = clEnqueueWriteBuffer( queue, bufB, CL_TRUE, 0,12* sizeof(float), b, 0, NULL, NULL );
    // err = clEnqueueWriteBuffer( queue, bufC, CL_TRUE, 0,8 * sizeof(float), c, 0, NULL, NULL );

        /* Call clBLAS extended function. Perform gemm for the lower right sub-matrices */
    err = clblasSgemm( clblasRowMajor, clblasNoTrans, clblasNoTrans,
                                2,4,3,alpha, bufA, 0, lda,
                                bufB, 0, ldb, beta,
                                bufC, 0, ldc,
                                1, &queue, 0, NULL, &event );

    /* Wait for calculations to be finished. */
    err = clWaitForEvents( 1, &event );

    /* Fetch results of calculations from GPU memory. */
    err = clEnqueueReadBuffer( queue, bufC, CL_TRUE, 0,8* sizeof(float),c, 0, NULL, NULL );

    /* Release OpenCL memory objects. */
    clReleaseMemObject( bufC );
    clReleaseMemObject( bufB );
    clReleaseMemObject( bufA );

    /* Finalize work with clBLAS */
    clblasTeardown( );

    /* Release OpenCL working objects. */
    clReleaseCommandQueue( queue );
    clReleaseContext( ctx );
    
}

int main()
{
    float *a=mMalloc(1000*2000*sizeof(float));for(int i=0;i<1000*2000;i++) a[i]=(float)(mRand(-10000,10000))/10000.0f;
    float *b=mMalloc(2000*3000*sizeof(float));for(int i=0;i<2000*3000;i++) b[i]=(float)(mRand(-10000,10000))/10000.0f;
    float *c=mMalloc(1000*3000*sizeof(float));

    MMatrix *mat1=mMatrixCreate(1000,2000,NULL); mMatrixData(mat1,a);
    MMatrix *mat2=mMatrixCreate(2000,3000,NULL); mMatrixData(mat2,b);
    MMatrix *mat3=mMatrixCreate(DFLT,DFLT,NULL);

    mTimerBegin();
    mMatrixMul(mat1,mat2,mat3);
    mTimerEnd();
    for(int i=0;i<10;i++) printf("%f,",mat3->data[0][i]);printf("\n");

    
    mTimerBegin();
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                1000,3000,2000,
                1.0,
                a,2000,
                b,3000,
                0.0,
                c,3000);
    mTimerEnd();
    for(int i=0;i<10;i++) printf("%f,",c[i]);printf("\n");

    mTimerBegin();
    char *d_A=NULL;cudaMalloc((void**)&d_A,sizeof(float)*1000*2000);
    char *d_B=NULL;cudaMalloc((void**)&d_B,sizeof(float)*2000*3000);
    char *d_C=NULL;cudaMalloc((void**)&d_C,sizeof(float)*1000*3000);
    cublasHandle_t handle;cublasCreate(&handle);
    mTimerEnd();

    d_C[0]=d_A[0];
    
    float alpha = 1, beta = 0;
    
    mTimerBegin();
    cudaMemcpy(d_A,a,sizeof(float)*1000*2000,cudaMemcpyHostToDevice);
    cudaMemcpy(d_B,b,sizeof(float)*2000*3000,cudaMemcpyHostToDevice);
    mTimerEnd();
    mTimerBegin();
    // for(int i=0;i<100;i++)
    cublasSgemm_v2(
          handle,
          CUBLAS_OP_N,
          CUBLAS_OP_N,
                3000,1000,2000,
                &alpha,
                (float *)d_B,3000,
                (float *)d_A,2000,
                &beta,
                (float *)d_C,3000);
    mTimerEnd();
    mTimerBegin();
    cudaMemcpy(c,d_C,sizeof(float)*1000*3000,cudaMemcpyDeviceToHost);
    mTimerEnd();

    mTimerBegin();
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cublasDestroy(handle);
    mTimerEnd();

    for(int i=0;i<10;i++) printf("%f,",c[i]);printf("\n");

    mMatrixRelease(mat1);
    mMatrixRelease(mat2);
    mMatrixRelease(mat3);
}


int main3()
{
    // float *a=_aligned_malloc(1000*2000*sizeof(float),32768);
    // printf("aa is %p\n",a);
    
    // float *b=mMalloc(2000*3000*sizeof(float));
    // float *c=mMalloc(1000*3000*sizeof(float));

    float *a; cudaHostAlloc((void **)&a,1000*2000*sizeof(float),cudaHostAllocWriteCombined | cudaHostAllocMapped);
    printf("a is %p\n",a);
    float *b; cudaHostAlloc((void **)&b,2000*3000*sizeof(float),cudaHostAllocWriteCombined | cudaHostAllocMapped);
    float *c; cudaHostAlloc((void **)&c,1000*3000*sizeof(float),cudaHostAllocWriteCombined | cudaHostAllocMapped);
    for(int i=0;i<1000*2000;i++) a[i]=(float)(mRand(-10000,10000))/10000.0f;
    for(int i=0;i<2000*3000;i++) b[i]=(float)(mRand(-10000,10000))/10000.0f;

    MMatrix *mat1=mMatrixCreate(1000,2000,NULL); mMatrixData(mat1,a);
    MMatrix *mat2=mMatrixCreate(2000,3000,NULL); mMatrixData(mat2,b);
    MMatrix *mat3=mMatrixCreate(DFLT,DFLT,NULL);

    mTimerBegin();
    mMatrixMul(mat1,mat2,mat3);
    mTimerEnd();
    for(int i=0;i<10;i++) printf("%f,",mat3->data[0][i]);printf("\n");

    printf("CblasNoTrans=%d\n",CblasNoTrans);
    printf("CUBLAS_OP_N=%d\n",CUBLAS_OP_N);
    /*
    mTimerBegin();
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                1000,3000,2000,
                1.0,
                a,2000,
                b,3000,
                0.0,
                c,3000);
    mTimerEnd();
    for(int i=0;i<10;i++) printf("%f,",c[i]);printf("\n");

    mTimerBegin();
    char *d_A=NULL;cudaMalloc((void**)&d_A,sizeof(float)*1000*2000);
    char *d_B=NULL;cudaMalloc((void**)&d_B,sizeof(float)*2000*3000);
    char *d_C=NULL;cudaMalloc((void**)&d_C,sizeof(float)*1000*3000);
    
    mTimerEnd();
    */

    float *dd_A;int ret=cudaHostGetDevicePointer((void **)(&dd_A),a,0);printf("ret=%d\n",ret);
    float *d_A = dd_A;
    float *d_B;cudaHostGetDevicePointer((void **)(&d_B),b,0);
    float *d_C;cudaHostGetDevicePointer((void **)(&d_C),c,0);
    cublasHandle_t handle;cublasCreate(&handle);
    
    float alpha = 1, beta = 0;
    
  
    mTimerBegin();
    // for(int i=0;i<100;i++)
    cublasSgemm(
          handle,
          CUBLAS_OP_N,
          CUBLAS_OP_N,
                3000,1000,2000,
                &alpha,
                (float *)d_B,3000,
                (float *)d_A,2000,
                &beta,
                (float *)d_C,3000);
    mTimerEnd();

    mTimerBegin();
    cublasDestroy(handle);
    mTimerEnd();

    for(int i=0;i<10;i++) printf("%f,",c[i]);printf("\n");
    
    mMatrixRelease(mat1);
    mMatrixRelease(mat2);
    mMatrixRelease(mat3);
    // _aligned_free(a);
    cudaFreeHost(a);
    cudaFreeHost(b);
    cudaFreeHost(c);
}

