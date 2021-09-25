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



