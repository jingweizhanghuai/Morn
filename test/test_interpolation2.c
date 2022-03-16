#include "morn_image.h"

void PointInterpolation(MList *curve,MList *list,float k1,float k2);
int main()
{
    MList *curve = mListCreate();
    MList *list = mListCreate();
    MImagePoint pt;
    pt.x=1,pt.y=0;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=2,pt.y=5;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=3,pt.y=3;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=4,pt.y=1;mListWrite(list,DFLT,&pt,sizeof(MImagePoint));

    PointInterpolation(curve,list,0,0);
    for(int i=0;i<curve->num;i++)
    {
        MImageCurve *c = curve->data[i];
        for(float x=c->v1.x;x<c->v2.x;x+=0.1)
            printf("(%f,%f)\n",x,mCurvePoint(c,x));
    }

    mListRelease(list);
    mListRelease(curve);
}