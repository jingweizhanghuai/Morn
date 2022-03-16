#include "morn_image.h"

float curve_interpolation(float x,float *para)
{
    float x2=x*x;float x3=x2*x;
    return (para[0]+para[1]*x+para[2]*x2+para[3]*x3);
}

void CurveInterpolation3(MImagePoint *pt1,MImagePoint *pt2,MImagePoint *pt3,int type,MMatrix *mat,MImageCurve *curve)
{
    float x1=0,y1=0,x2=0,y2=0,x3=0,y3=0;
    curve->type=type;
    if(type==MORN_CURVE_X) {x1=pt1->x;y1=pt1->y;x2=pt2->x;y2=pt2->y;x3=pt3->x;y3=pt3->y;}
    else                   {x1=pt1->y;y1=pt1->x;x2=pt2->y;y2=pt2->x;x3=pt3->y;y3=pt3->x;}
    curve->v1=*pt1;curve->v2=*pt2;
    curve->curve = curve_interpolation;

    mMatrixRedefine(mat,3,4);float **data=mat->data;
    data[0][0]=1;data[0][1]=x1;data[0][2]=x1*x1;data[0][3]=0-y1;
    data[1][0]=1;data[1][1]=x2;data[1][2]=x2*x2;data[1][3]=0-y2;
    data[2][0]=1;data[2][1]=x3;data[2][2]=x3*x3;data[2][3]=0-y3;
    mLinearEquation(mat,curve->para);
    curve->para[3]=0;
}

void CurveInterpolation4(MImagePoint *pt1,MImagePoint *pt2,int type,float d11,float d12,MMatrix *mat,MImageCurve *curve)
{
    float x1,y1,x2,y2;
    curve->type=type;
    if(type==MORN_CURVE_X) {x1=pt1->x;y1=pt1->y;x2=pt2->x;y2=pt2->y;}
    else                   {x1=pt1->y;y1=pt1->x;x2=pt2->y;y2=pt2->x;}
    curve->v1=*pt1;curve->v2=*pt2;
    curve->curve = curve_interpolation;
    
    mMatrixRedefine(mat,4,5);float **data=mat->data;
    data[0][0]=1;data[0][1]=x1;data[0][2]=x1*x1;data[0][3]=x1*x1*x1;data[0][4]=0-y1;
    data[1][0]=1;data[1][1]=x2;data[1][2]=x2*x2;data[1][3]=x2*x2*x2;data[1][4]=0-y2;
    data[2][0]=0;data[2][1]= 1;data[2][2]= 2*x1;data[2][3]= 3*x1*x1;data[2][4]=0-d11;
    data[3][0]=0;data[3][1]= 0;data[3][2]= 2   ;data[3][3]= 6*x1   ;data[3][4]=0-d12;
    mLinearEquation(mat,curve->para);
}

void mCurveInterpolation(MList *point,MList *curve_list)
{
    int n = point->num;MImagePoint **pt=(MImagePoint **)(point->data);
    mException(n<3,EXIT,"invalid input");
    int type=0;int flag;
    int i;{flag = (pt[0]->x>pt[1]->x); for(i=1;i<n-1;i++) {if((pt[i]->x>pt[i+1]->x)!=flag) break;}} if(i==n-1) type=MORN_CURVE_X;
    else  {flag = (pt[0]->y>pt[1]->y); for(i=1;i<n-1;i++) {if((pt[i]->y>pt[i+1]->y)!=flag) break;}} if(i==n-1) type=MORN_CURVE_Y;
    else mException(1,EXIT,"invalid input");

    MMatrix *mat = mMatrixCreate(4,5);
    MImageCurve curve;

    mListClear(curve_list);
    
    CurveInterpolation3(pt[0],pt[1],pt[2],type,mat,&curve);
    mListWrite(curve_list,DFLT,&curve,sizeof(MImageCurve));
    float *para=curve.para;float x = (type==MORN_CURVE_X)?pt[2]->x:pt[2]->y;
    float d11=para[1]+2*para[2]*x+3*para[3]*x*x;
    float d12=        2*para[2]  +6*para[3]*x  ;

    for(i=3;i<n;i++)
    {
        CurveInterpolation4(pt[i-1],pt[i],type,d11,d12,mat,&curve);
        mListWrite(curve_list,DFLT,&curve,sizeof(MImageCurve));
        para=curve.para; x = (type==MORN_CURVE_X)?pt[i]->x:pt[i]->y;
        d11=para[1]+2*para[2]*x+3*para[3]*x*x;
        d12=        2*para[2]  +6*para[3]*x  ;
    }

    mMatrixRelease(mat);
}


void PointInterpolation(MList *curve_list,MList *list,float k1,float k2)
{
    int n=3*list->num-2;
    printf("n=%d\n",n);
    MMatrix *matrix=mMatrixCreate(n,n+1);
    float **mat=matrix->data;for(int j=0;j<n;j++) memset(mat[j],9,(n+1)*sizeof(float));
    MImagePoint **pt=(MImagePoint **)(list->data);
    float x11,x12,x13,y1,x21,x22,x23,y2;

    int j;
    x11=pt[0]->x;x12=x11*x11;x13=x12*x11;y1=pt[0]->y;pt++;
    printf("x11=%f,y1=%f\n",x11,y1);
    x21=pt[0]->x;x22=x21*x21;x23=x22*x21;y2=pt[0]->y;pt++;
    printf("x21=%f,y2=%f\n",x21,y2);
    mat[0][0]=3*x12;mat[0][1]=2*x11;mat[0][2]=  1;            mat[0][n]=0-k1;
    mat[1][0]=  x13;mat[1][1]=  x12;mat[1][2]=x11;mat[1][3]=1;mat[1][n]=0-y1;
    mat[2][0]=  x23;mat[2][1]=  x22;mat[2][2]=x21;mat[2][3]=1;mat[2][n]=0-y2;
    mat[3][0]=3*x22;mat[3][1]=2*x21;mat[3][2]=  1;            mat[3][4]=0-2*x21;mat[3][5]=-1;
    int m=4;
    for(j=4;j<n-3;j++)
    {
        x11=x21;x12=x22;y1=y2;
        x21=pt[0]->x;x22=x21*x21;y2=pt[0]->y;pt++;
        printf("j=%d\n",j);
        printf("x21=%f,y2=%f\n",x21,y2);
            mat[j][m]=x12;mat[j][m+1]=x11;mat[j][m+2]=1;mat[j][n]=0-y1;
        j++;mat[j][m]=x22;mat[j][m+1]=x21;mat[j][m+2]=1;mat[j][n]=0-y2;
        j++;mat[j][m]=2*x21;mat[j][m+1]=1;mat[j][m+3]=0-2*x21;mat[j][m+4]=-1;
        m=m+3;
    }
    x11=x21;x12=x22;y1=y2;
    x21=pt[0]->x;x22=x21*x21;y2=pt[0]->y;pt++;
    printf("j=%d\n",j);
        printf("x21=%f,y2=%f\n",x21,y2);
        mat[j][m]=x12;mat[j][m+1]=x11;mat[j][m+2]=1;mat[j][n]=0-y1;
    j++;mat[j][m]=x22;mat[j][m+1]=x21;mat[j][m+2]=1;mat[j][n]=0-y2;
    j++;mat[j][m]=2*x21;mat[j][m+1]=1;mat[j][n]=0-k2;

    PrintMat(matrix);

    float *a=mMalloc(n*sizeof(float));
    mLinearEquation(matrix,a);
    printf("y=%fx3+%fx2+%fx1+%f\n",a[0],a[1],a[2],a[3]);
    printf("y=%fx2+%fx1+%f\n",a[4],a[5],a[6]);

    mListClear(curve_list);
    MImageCurve curve;
    pt=(MImagePoint **)(list->data);
    float para[4];para[3]=a[0];para[2]=a[1];para[1]=a[2];para[0]=a[3];
    mCurve(&curve,pt[0]->x,pt[1]->x,DFLT,curve_interpolation,para);
    mListWrite(curve_list,DFLT,&curve,sizeof(MImageCurve));
    m=4;para[3]=0;
    for(int i=1;i<list->num-1;i++)
    {
        para[2]=a[m];para[1]=a[m+1];para[0]=a[m+2];m+=3;
        mCurve(&curve,pt[i]->x,pt[i+1]->x,DFLT,curve_interpolation,para);
        mListWrite(curve_list,DFLT,&curve,sizeof(MImageCurve));
    }

    mFree(a);
    mMatrixRelease(matrix);
}



