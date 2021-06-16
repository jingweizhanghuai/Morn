// gcc -O2 -fopenmp test_clblas.c -I C:\ProgramFiles\CPackage\clBLAS\include -I C:\ProgramFiles\NVIDIACUDA\include -I ..\include\ -o test_clblas.exe -L ..\lib\x64_mingw\ -lmorn -L C:\ProgramFiles\CPackage\clBLAS\lib_x64_mingw -lclBLAS -L C:\ProgramFiles\NVIDIACUDA\lib\x64 -l:OpenCL.lib -lstdc++
// #include <sys/types.h>
#include <stdio.h>

#include <clBLAS.h>
#include "morn_util.h"

#define M  4
#define N  3
#define K  5


int ma1in( void )
{
    const float alpha = 1;
    const float beta = 1;
    
    const size_t lda = K;        /* i.e. lda = K */
    const size_t ldb = N;        /* i.e. ldb = N */
    const size_t ldc = N;        /* i.e. ldc = N */

    
    // float *A = malloc(M*K*sizeof(float));for(int i=0;i<M*K;i++) A[i]=(float)mRand(-1000,1000)/1000.0;
    // float A[M*K] = {
    // 11, 12, 13, 14, 15,
    // 21, 22, 23, 24, 25,
    // 31, 32, 33, 34, 35,
    // 41, 42, 43, 44, 45,
    // };
    // float *B = malloc(K*N*sizeof(float));for(int i=0;i<K*N;i++) B[i]=(float)mRand(-1000,1000)/1000.0;
    // float B[K*N] = {
    // 11, 12, 13,
    // 21, 22, 23,
    // 31, 32, 33,
    // 41, 42, 43,
    // 51, 52, 53,
    // };
    
    // float *C = malloc(M*N*sizeof(float));for(int i=0;i<M*N;i++) C[i]=(float)mRand(-1000,1000)/1000.0;
    // float C[M*N] = {
    //     11, 12, 13,
    //     21, 22, 23,
    //     31, 32, 33,
    //     41, 42, 43,
    // };
    
    float *result=malloc(M*N*sizeof(float));
    
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

    // for(int i=0;i<10;i++) printf("%f,",C[i]);
    // printf("\n");

    float *A = malloc(M*K*sizeof(float));for(int i=0;i<M*K;i++) A[i]=(float)mRand(-1000,1000)/1000.0;
    float *B = malloc(K*N*sizeof(float));for(int i=0;i<K*N;i++) B[i]=(float)mRand(-1000,1000)/1000.0;
    float *C = malloc(M*N*sizeof(float));for(int i=0;i<M*N;i++) C[i]=0.0f;//(float)mRand(-1000,1000)/1000.0;

    mTimerBegin("blas1");
    /* Prepare OpenCL memory objects and place matrices inside them. */
    bufA = clCreateBuffer( ctx, CL_MEM_READ_WRITE, M * K * sizeof(float),NULL, &err );
    bufB = clCreateBuffer( ctx, CL_MEM_READ_WRITE, K * N * sizeof(float),NULL, &err );
    bufC = clCreateBuffer( ctx, CL_MEM_READ_WRITE, M * N * sizeof(float),NULL, &err );

    // printf("A=%p,bufA=%p\n",A,bufA);
    // float *A = bufA;
    
    err = clEnqueueWriteBuffer( queue, bufA, CL_TRUE, 0,M * K * sizeof(float), A, 0, NULL, NULL );
    err = clEnqueueWriteBuffer( queue, bufB, CL_TRUE, 0,K * N * sizeof(float), B, 0, NULL, NULL );
    err = clEnqueueWriteBuffer( queue, bufC, CL_TRUE, 0,M * N * sizeof(float), C, 0, NULL, NULL );

    mTimerBegin("blas2");
        /* Call clBLAS extended function. Perform gemm for the lower right sub-matrices */
    err = clblasSgemm( clblasRowMajor, clblasNoTrans, clblasNoTrans,
                        M, N, K,alpha, 
                        bufA, 0, lda,
                        bufB, 0, ldb, beta,
                        bufC, 0, ldc,
                        1, &queue, 0, NULL, &event );
    printf("err=%d,CL_SUCCESS=%d\n",err,CL_SUCCESS);
    
    /* Wait for calculations to be finished. */
    err = clWaitForEvents( 1, &event );
    mTimerEnd("blas2");

    

    /* Fetch results of calculations from GPU memory. */
    err = clEnqueueReadBuffer( queue, bufC, CL_TRUE, 0,M * N * sizeof(*result),result, 0, NULL, NULL );

    /* Finalize work with clBLAS */
    clblasTeardown( );
    mTimerEnd("blas1");

    for(int i=0;i<10;i++) printf("%f,",C[i]);
    printf("\n");

    for(int i=0;i<10;i++) printf("%f,",result[i]);
    printf("\n");

    /* Release OpenCL memory objects. */
    clReleaseMemObject( bufC );
    clReleaseMemObject( bufB );
    clReleaseMemObject( bufA );

    /* Release OpenCL working objects. */
    clReleaseCommandQueue( queue );
    clReleaseContext( ctx );
    free(A);
    free(B);
    free(C);
    free(result);

    printf("eeeeeeeeeeeee\n");
    return ret;
}