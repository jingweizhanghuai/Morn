// g++ -O2 -fopenmp test_matrix3.cpp -o test_matrix3.exe -lclapack -lopenblas -lf2c -lmorn
#define EIGEN_DONT_VECTORIZE
#include "Eigen/Dense"
#include "armadillo"
#include "morn_math.h"

void test(int K)
{
    printf("matrix: %d*%d\n",K,K);
    arma::mat A(K,K);
    Eigen::MatrixXf m_a(K,K);
    MMatrix *mat_a = mMatrixCreate(K,K);

    for(int j=0;j<K;j++)for(int i=0;i<K;i++)
    {
        float v=mRand(-1000,1000)/500.0;
        A(j,i)=v;
        m_a(j,i)=v;
        mat_a->data[j][i]=v;
    }

    arma::mat B;
    mTimerBegin("armadillo");
    for(int i=0;i<10000;i++)
        B = A.t();
    mTimerEnd("armadillo");

    Eigen::MatrixXf m_b;
    mTimerBegin("armadillo");
    for(int i=0;i<10000;i++)
        m_b = m_a.transpose();
    mTimerEnd("armadillo");

    MMatrix *mat_b=mMatrixCreate();
    mTimerBegin("armadillo");
    for(int i=0;i<10000;i++)
        mMatrixTranspose(mat_a,mat_b);
    mTimerEnd("armadillo");
    mMatrixRelease(mat_b);
    
    mMatrixRelease(mat_a);
}

void test1(int K)
{
    printf("matrix: %d*%d\n",K,K);
    arma::mat A(K,K);
    Eigen::MatrixXf m_a(K,K);
    MMatrix *mat = mMatrixCreate(K,K);

    for(int j=0;j<K;j++)for(int i=0;i<K;i++)
    {
        float v=mRand(-1000,1000)/500.0;
        A(j,i)=v;
        m_a(j,i)=v;
        mat->data[j][i]=v;
    }

    float det;

    mTimerBegin("armadillo");
    for(int i=0;i<1000;i++)
        det = arma::det(A);
    mTimerEnd("armadillo");
    // printf("det=%f\n",det);

    mTimerBegin("eigen");
    for(int i=0;i<1000;i++)
        det=m_a.determinant();
    mTimerEnd("eigen");
    // printf("det=%f\n",det);

    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        det=mMatrixDetValue(mat);
    mTimerEnd("Morn");
    // printf("det=%f\n",det);

    mMatrixRelease(mat);
}

int main()
{
    // test(10);
    // test(20);
    // test(50);
    // test(100);
    test(200);
    return 0;
}