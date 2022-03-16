// g++ -O2 -fopenmp test_linear_equation.cpp -o test_linear_equation.exe -lclapack -lf2c -lopenblas -lmorn
// g++ -O2 -fopenmp test_linear_equation.cpp -o test_linear_equation.exe -I ../include/ -L ../lib/x64_gnu/ -llapack -lopenblas -larmadillo -lmorn
#define EIGEN_DONT_VECTORIZE
#include "Eigen/Dense"
#include "armadillo"
#include "morn_math.h"

extern "C" int sgesv_(long *n, long *nrhs, float *a, long *lda,long *ipiv, float *b, long *ldb, long *info);

void test(int K)
{
    printf("num_x=%d\n",K);
    float *a=(float *)malloc(K*K*sizeof(float));
    float *a_buff=(float *)malloc(K*K*sizeof(float));
    float *b=(float *)malloc(K*sizeof(float));
    float *b_buff=(float *)malloc(K*sizeof(float));

    arma::mat A(K,K);
    arma::vec B(K);

    Eigen::MatrixXf m_a(K,K);
    Eigen::VectorXf v_b(K);

    MMatrix *mat = mMatrixCreate(K,K+1);

    float v;
    for(int i=0;i<K;i++)
    {
        for(int j=0;j<K;j++)
        {
            v=mRand(-1000,1000)/1000.0;
            a_buff[j*K+i]=v;
            m_a(i,j)=v;
            A(i,j)=v;
            mat->data[i][j]=v;
        }
        v=mRand(-1000,1000);
        b_buff[i]=v;
        v_b(i)=v;
        B(i)=v;
        mat->data[i][K]=0-v;
    }

    #define T 1000

    long M=K,N=1;
    long info,ipiv[K];
    mTimerBegin("lapack");
    for(int i=0;i<T;i++)
    {
        memcpy(a,a_buff,K*K*sizeof(float));
        memcpy(b,b_buff,K*sizeof(float));
        sgesv_(&M,&N,a,&M,ipiv,b,&M,&info);
    }
    mTimerEnd("lapack");
    // for(int i=0;i<K;i++) printf("%f,",b[i]);printf("\n");

    arma::vec x;
    mTimerBegin("armadillo");
    for(int i=0;i<T;i++)
        x = arma::solve(A, B);
    mTimerEnd("armadillo");
    // for(int i=0;i<K;i++) printf("%f,",x(i));printf("\n");

    Eigen::VectorXf v_x;
    mTimerBegin("Eigen QR");
    for(int i=0;i<T;i++)
        v_x = m_a.colPivHouseholderQr().solve(v_b);
    mTimerEnd("Eigen QR");
    // for(int i=0;i<K;i++) printf("%f,",v_x(i));printf("\n");

    mTimerBegin("Eigen LU");
    for(int i=0;i<T;i++)
        v_x = m_a.lu().solve(v_b);
    mTimerEnd("Eigen LU");
    // for(int i=0;i<K;i++) printf("%f,",v_x(i));printf("\n");

    mTimerBegin("Morn");
    for(int i=0;i<T;i++)
        mLinearEquation(mat,b);
    mTimerEnd("Morn");
    // for(int i=0;i<K;i++) printf("%f,",b[i]);printf("\n");
    
    free(a);
    free(a_buff);
    free(b);
    free(b_buff);
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