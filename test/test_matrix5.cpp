// g++ -O2 -fopenmp test_matrix5.cpp -o test_matrix5.exe -lclapack -lopenblas -lf2c -lmorn
#include "armadillo"
#include "morn_math.h"

int maddin()
{
    arma::mat A={{4,3,11},{2,-1,0},{-1,2,3}};
    arma::mat B = A.t()*A;
    B.print("B:");

    arma::vec eigval;
    arma::mat eigvec;

    arma::eig_sym(eigval, eigvec, B);
    eigval.print("eigval:");
    eigvec.print("eigvec:");
    
    float a[9]={4,3,11,2,-1,0,-1,2,3};
    MMatrix *mat_a=mMatrixCreate(3,3);
    mMatrixData(mat_a,a);

    MMatrix *mat_b=mMatrixCreate(3,3);
    mMatrixTranspose(mat_a,mat_b);
    PrintMat(mat_b);

    mMatrixMul(mat_b,mat_a,mat_a);
    PrintMat(mat_a);

    MList *eigenvalue = mListCreate();
    MList *eigenvector= mListCreate();
    
    mMatrixEigenValue(mat_a,eigenvalue,eigenvector);
    printf("eigenvalue->num=%d\n",eigenvalue->num);
    for(int i=0;i<3;i++)
    {
        printf("eigenvalue=%f\n",*(float *)(eigenvalue->data[i]));
        float *p=(float *)eigenvector->data[i];
        printf("%f,%f,%f\n",p[0],p[1],p[2]);
    }
    return 0;
}

void test(int K)
{
    printf("K=%d\n",K);
    arma::mat A(K,K);
    MMatrix *a=mMatrixCreate(K,K);

    for(int j=0;j<K;j++)for(int i=0;i<K;i++)
    {
        float v=mRand(-1000,1000)/1000.0;
        A(j,i)=v;
        a->data[j][i]=v;
    }

    arma::mat B = A.t()*A;
    MMatrix *b=mMatrixCreate(K,K);
    mMatrixTranspose(a,b);
    mMatrixMul(b,a,b);

    arma::vec eigval;
    arma::mat eigvec;
    mTimerBegin("armadillo");
    for(int i=0;i<1000;i++)
        arma::eig_sym(eigval, eigvec, B);
    mTimerEnd("armadillo");
    // eigval.print("eigval:");
    // eigvec.print("eigvec:");

    MList *eigenvalue =mListCreate();
    MList *eigenvector=mListCreate();
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mMatrixEigenValue(b,eigenvalue,eigenvector);
    mTimerEnd("Morn");
    // for(int i=0;i<eigenvalue->num;i++)
    // {
        // printf("eigenvalue=%f\neigenvector=",*(float *)(eigenvalue->data[i]));
        // float *p=(float *)eigenvector->data[i];
        // for(int j=0;j<K;j++) printf("%f,",p[j]);
        // printf("\n");
    // }
    mListRelease(eigenvalue);
    mListRelease(eigenvector);

    mMatrixRelease(a);
    mMatrixRelease(b);
}

int main()
{
    test(10);
    test(20);
    test(50);
    test(100);
    return 0;
}