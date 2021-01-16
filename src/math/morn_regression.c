#include "morn_math.h"

void mLinearRegression(float **x,float *y,int n,int m,float *A)
{
    mException((n<3),EXIT,"sample number (which <3) is not enough.");
    mException((INVALID_POINTER(x)||(INVALID_POINTER(y))),EXIT,"no input data.");
    mException((INVALID_POINTER(A)),EXIT,"answer out is NULL.");

    MTable *tab = mTableCreate(m+1,m+2,sizeof(double));double **data = tab->dataD64;
    for(int j=0;j<m+1;j++) memset(data[j],0,(m+2)*sizeof(double));
    
    double num = (float)n;
    for(int k=0;k<n;k++)
    {
        for(int j=0;j<m;j++)
        {
            for(int i=j;i<m;i++) data[j][i]+=x[k][j]*x[k][i]/num;
            data[j][m  ]+=x[k][j]/num;
            data[j][m+1]-=x[k][j]*y[k]/num;
        }
        for(int i=0;i<m;i++) data[m][i]+=x[k][i]/num;
        data[m][m  ]+=1.0/num;
        data[m][m+1]-=y[k]/num;
    }
    
    MMatrix *mat = mMatrixCreate(m+1,m+2);
    for(int j=0;j<m;j++)
    {
        for(int i=0;i<m;i++) mat->data[j][i]=(i<j)?data[i][j]:data[j][i];
        mat->data[j][m  ] = data[j][m];
        mat->data[j][m+1] = data[j][m+1];
    }
    for(int i=0;i<m+2;i++) mat->data[m][i]=data[m][i];
    
    mTableRelease(tab);
    
    mLinearEquation(mat,A);
    
    mMatrixRelease(mat);
}
