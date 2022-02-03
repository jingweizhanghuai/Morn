/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_image.h"

void m_Mask(MImageMask *mask,int num,float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float x5,float y5,float x6,float y6,float x7,float y7)
{
    mException((num<3),EXIT,"invalid input");
    mException(INVALID_POINTER(mask),EXIT,"invalid input");

    float x[9]={x0,x1,x2,x3,x4,x5,x6,x7,x0};x[num]=x0;
    float y[9]={y0,y1,y2,y3,y4,y5,y6,y7,x0};y[num]=y0;

    float ymin = MIN(MIN(y0,y1),y2);float ymax = MAX(MAX(y0,y1),y2);
    for(int n=3;n<num;n++){ymin = MIN(ymin,y[n]);ymax = MAX(ymax,y[n]);}
    float step=(ymax-ymin)/99.0;
    memset(mask->x1,0,100*sizeof(float));
    memset(mask->x2,0,100*sizeof(float));
    mask->y1=ymin;mask->y2=ymax;mask->step=step;
    
    for(int n=0;n<num;n++)
    {
        int ly1=(int)((y[n  ]-ymin)/step + 0.5f);
        int ly2=(int)((y[n+1]-ymin)/step + 0.5f);
        float k=(x[n+1]-x[n])/(y[n+1]-y[n]);
        float x_locate = (ly1*step+ymin-y[n])*k+x[n];
        if(ly1==ly2)
        {
            float x_locate2=(ly2*step+ymin-y[n+1])*k+x[n+1];
            if(x_locate<x_locate2) {mask->x1[ly1]=x_locate ;mask->x2[ly1]=x_locate2;}
            else                   {mask->x1[ly1]=x_locate2;mask->x2[ly1]=x_locate ;}
            continue;
        }
        float x_step=k*step;if(ly2<ly1) x_step=0-x_step;
        for(int ly=ly1;ly!=ly2;ly+=((ly2>ly1)?1:-1))
        {
                 if(mask->x1[ly]==0.0f)                               mask->x1[ly]=x_locate;
            else if(x_locate<mask->x1[ly]) {mask->x2[ly]=mask->x1[ly];mask->x1[ly]=x_locate;}
            else if(x_locate>mask->x2[ly])                            mask->x2[ly]=x_locate;
            x_locate = x_locate + x_step;
        }
    }
    mask->x2[ 0]=mask->x1[ 0];
    mask->x2[99]=mask->x1[99];
}

void mPolygonMask(MImageMask *mask,MList *polygon)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input polygon");
    mException((polygon->num<3),EXIT,"invalid input polygon");
    mException(INVALID_POINTER(mask),EXIT,"invalid input");

    MImagePoint **point = (MImagePoint **)(polygon->data);
    float ymin = point[0]->y;float ymax = point[0]->y;
    for(int n=1;n<polygon->num;n++)
        {ymin = MIN(ymin,point[n]->y);ymax= MAX(ymax,point[n]->y);}
    float step=(ymax-ymin)/99.0;
    memset(mask->x1,0,100*sizeof(float));
    memset(mask->x2,0,100*sizeof(float));
    mask->y1=ymin;mask->y2=ymax;mask->step=step;
    
    for(int n=0;n<polygon->num;n++)
    {
        MImagePoint *p1,*p2;

        p1 = point[n];
        if(n+1<polygon->num) p2 = point[n+1];
        else p2 = point[0];
        
        int ly1=(int)((p1->y-ymin)/step + 0.5f);
        int ly2=(int)((p2->y-ymin)/step + 0.5f);
        float k=(p2->x-p1->x)/(p2->y-p1->y);
        float x_locate = (ly1*step+ymin-p1->y)*k+p1->x;
        if(ly1==ly2)
        {
            float x_locate2=(ly2*step+ymin-p2->y)*k+p2->x;
            if(x_locate<x_locate2) {mask->x1[ly1]=x_locate ;mask->x2[ly1]=x_locate2;}
            else                   {mask->x1[ly1]=x_locate2;mask->x2[ly1]=x_locate ;}
            continue;
        }
        float x_step=k*step;if(ly2<ly1) x_step=0-x_step;
        for(int ly=ly1;ly!=ly2;ly+=((ly2>ly1)?1:-1))
        {
                 if(mask->x1[ly]==0)                                  mask->x1[ly]=x_locate;
            else if(x_locate<mask->x1[ly]) {mask->x2[ly]=mask->x1[ly];mask->x1[ly]=x_locate;}
            else if(x_locate>mask->x2[ly])                            mask->x2[ly]=x_locate;
            x_locate = x_locate + x_step;
        }
    }
    mask->x2[ 0]=mask->x1[ 0];
    mask->x2[99]=mask->x1[99];
}

int mMaskData(MImageMask *mask,float x,float y)
{
    if((y<mask->y1)||(y>mask->y2)) return 0;
    int ly=(int)((y-mask->y1)/mask->step+0.5);
    if((x<mask->x1[ly])||(x>mask->x2[ly])) return 0;
    return 1;
}




