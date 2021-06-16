#include "morn_image.h"

void mCurveInterpolation(MList *point,MList *curve_list);
int main()
{
    MList *point_list = mListCreate();
    MList *curve_list = mListCreate();
    MImagePoint pt;
    pt.x=10,pt.y=20;mListWrite(point_list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=22,pt.y=43;mListWrite(point_list,DFLT,&pt,sizeof(MImagePoint));
    pt.x=31,pt.y=36;mListWrite(point_list,DFLT,&pt,sizeof(MImagePoint));

    mCurveInterpolation(point_list,curve_list);
    for(int i=0;i<curve_list->num;i++)
    {
        
    }

    mListRelease(point_list);
    mListRelease(curve_list);
}

    
    