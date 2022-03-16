// g++ -O2 -fopenmp test_matrix4.cpp -o test_matrix4.exe -lclapack -lopenblas -lf2c -lmorn
// g++ -O2 -fopenmp test_matrix4.cpp -o test_matrix4.exe -I ../include/ -L ../lib/x64_gnu/ -llapack -lopenblas -larmadillo -lmorn
// #define EIGEN_DONT_VECTORIZE
#include "Eigen/Dense"
#include "morn_math.h"

extern "C" int sgetrf_(long *m,long *n,float *a,long *lda,long *ipiv,long *info);
extern "C" int sgetri_(long *n,float *a,long *lda,long *ipiv,float *work,long *lwork,long *info);
void test(int K)
{
    printf("matrix: %d*%d\n",K,K);
    float *a=(float *)malloc(K*K*sizeof(float));
    float *a_buff=(float *)malloc(K*K*sizeof(float));

    Eigen::MatrixXf m_a(K,K);

    MMatrix *mat = mMatrixCreate(K,K);

    for(int j=0;j<K;j++)for(int i=0;i<K;i++)
    {
        float v=mRand(-1000,1000)/1000.0;
        a_buff[j*K+i]=v;
        m_a(j,i)=v;
        mat->data[j][i]=v;
    }

    long n=K,lwork=K*K,info;
    long *ipiv = (long *)malloc(K*sizeof(long));
    float *work=(float *)malloc(K*K*sizeof(float));
    mTimerBegin("lapack");
    for(int i=0;i<1000;i++)
    {
        memcpy(a,a_buff,K*K*sizeof(float));
        sgetrf_(&n,&n,a,&n,ipiv,&info);
        sgetri_(&n,a,&n,ipiv,work,&lwork,&info);
    }
    mTimerEnd("lapack");
    // for(int i=0;i<K;i++) printf("%f,",a[i]); printf("\n");
    free(ipiv);
    free(work);

    Eigen::MatrixXf m_inv;
    mTimerBegin("eigen");
    for(int i=0;i<1000;i++)
        m_inv=m_a.inverse();
    mTimerEnd("eigen");
    // for(int i=0;i<K;i++) printf("%f,",m_inv(2,i)); printf("\n");

    MMatrix *inv = mMatrixCreate(K,K);
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mMatrixInverse(mat,inv);
    mTimerEnd("Morn");
    // for(int i=0;i<K;i++) printf("%f,",inv->data[2][i]);printf("\n");
    mMatrixRelease(inv);

    free(a);
    free(a_buff);
    mMatrixRelease(mat);
}

int main()
{
    test(10);
    test(20);
    test(50);
    test(100);
    test(200);
    return 0;
}
