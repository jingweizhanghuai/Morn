#include "morn_image.h"




void CurvePoint(MList *point,MList *curve)
{
    MImageCurve **c = (MImageCurve **)(curve->data);
    MImagePoint **pt= (MImagePoint **)(point->data);
    int type = c[0]->type;
    int n=0;
    if(type==MORN_CURVE_X)
    {
        for(int i=0;i<point->num;i++)
        {
            for(;n<curve->num;n++)
            {
                if((pt[i]->x>=c[n]->v1.x)&&(pt[i]->x<=c[n]->v2.x))
                {
                    pt[i]->y=mCurvePoint(c[n],pt[i]->x);break;
                }
            }
        }
    }
    else
    {
        for(int i=0;i<point->num;i++)
        {
            for(;n<curve->num;n++)
            {
                if((pt[i]->y>=c[n]->v1.y)&&(pt[i]->y<=c[n]->v2.y))
                {
                    pt[i]->x=mCurvePoint(c[n],pt[i]->y);break;
                }
            }
        }
    }
}

int main1()
{
    float k=2.0;
    MList *point = mListCreate();
    for(int x=10;x<=40;x++)
    {
        MImagePoint pt={.x=x,.y=k*x};
        mListWrite(point,DFLT,&pt,sizeof(MImagePoint));
    }
    
    MList *curve = mListCreate();
    MList *list = mListCreate();
    MImagePoint pt;
    pt.x=10,pt.y=20;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=20,pt.y=50;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=30,pt.y=68;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=40,pt.y=80;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));

    mPointInterpolation(curve,list,k,k);
    CurvePoint(point,curve);
    for(int i=0;i<point->num;i++)
    {
        MImagePoint *p = point->data[i];
        printf("(%f,%f)\n",p->x,p->y);
    }

    mListRelease(list);
    mListRelease(curve);
    mListRelease(point);
}


int main()
{
    float k=0;
    MList *point = mListCreate();
    for(int x=10;x<=40;x++)
    {
        MImagePoint pt={.x=x,.y=k*x};
        mListWrite(point,DFLT,&pt,sizeof(MImagePoint));
    }
    
    MList *curve = mListCreate();
    MList *list = mListCreate();
    MImagePoint pt;
    pt.x=10,pt.y=90;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    // pt.x=20,pt.y=50;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    // pt.x=30,pt.y=68;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=40,pt.y=0;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));

    mPointInterpolation(curve,list,-10,0);
    CurvePoint(point,curve);
    for(int i=0;i<point->num;i++)
    {
        MImagePoint *p = point->data[i];
        printf("(%f,%f)\n",p->x,p->y);
    }

    mListRelease(list);
    mListRelease(curve);
    mListRelease(point);
}