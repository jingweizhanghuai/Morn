/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_image.h"

// #define PointLineCheck(px,py,lx1,ly1,lx2,ly2) ((lx1 - lx2)*(py-ly2)-(ly1 - ly2)*(px-lx2))
int mLinePointCheck(MList *line,MImagePoint *point)
{
    MImagePoint **line_point = (MImagePoint **)(line->data);
    mException((line->num !=2),EXIT,"invalid line");
    return (PointLineCheck(point->x,point->y,line_point[0]->x,line_point[0]->y,line_point[1]->x,line_point[1]->y)>0.0f);
}

int LineCrossCheck(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2)
{
    double flag1,flag2;
    
    flag1 = PointLineCheck(l2x1,l2y1,l1x1,l1y1,l1x2,l1y2);
    flag2 = PointLineCheck(l2x2,l2y2,l1x1,l1y1,l1x2,l1y2);
    if((flag1>0.0f)==(flag2>0.0f))
        return 0;
    else if((flag1==0.0f)||(flag2==0.0f))
        return 1;
    
    flag1 = PointLineCheck(l1x1,l1y1,l2x1,l2y1,l2x2,l2y2);
    flag2 = PointLineCheck(l1x2,l1y2,l2x1,l2y1,l2x2,l2y2);
    if((flag1>0.0f)==(flag2>0.0f))
        return 0;
    else if((flag1==0.0f)||(flag2==0.0f))
        return 1;
    
    return 1;
}



int LineCrossPoint(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2,float *px,float *py)
{
    // printf("l1 is %25.20f,%25.20f   %25.20f,%25.20f\n",l1x1,l1y1,l1x2,l1y2);
    // printf("l2 is %25.20f,%25.20f   %25.20f,%25.20f\n",l2x1,l2y1,l2x2,l2y2);
    double k1,k2,b1,b2;
    if((l1x1==l1x2)&&(l2x1==l2x2))
    {
        return 0;
    }
    k1 = (l1y1-l1y2)/(l1x1-l1x2);b1 = l1y1-k1*l1x1;
    k2 = (l2y1-l2y2)/(l2x1-l2x2);b2 = l2y1-k2*l2x1;
    // printf("k1 is %f,k2 is %f\n",k1,k2);
    if(((isinf(k1))&&(isinf(k2)))||(k1==k2))
    {
        // if(l1x1==l2x1) {*px=l1x1;*py=l1y1; return 1;}
        *px=mNan();*py=mNan();
        return 0;
    }
    
    double x,y;
         if(isinf(k1)) {x=l1x1;y=k2*x+b2;}
    else if(isinf(k2)) {x=l2x1;y=k1*x+b1;}
    else {x = (b1-b2)/(k2-k1);y = ((k1+k2)*x+b1+b2)/2.0f;}
    
    // printf("x is %25.20f,y is %25.20f\n",x,y);
    *px = x; *py = y;
    if((((l1x1>x)==(x>=l1x2))&&((l1y1>y)==(y>=l1y2)))&&(((l2x1>x)==(x>=l2x2))&&((l2y1>y)==(y>=l2y2))))
        return 1;
    return 0;
}

int mLineCross(MImagePoint *l1s,MImagePoint *l1e,MImagePoint *l2s,MImagePoint *l2e,MImagePoint *point)
{
    if(point == NULL) return LineCrossCheck(l1s->x,l1s->y,l1e->x,l1e->y,l2s->x,l2s->y,l2e->x,l2e->y);
    return LineCrossPoint(l1s->x,l1s->y,l1e->x,l1e->y,l2s->x,l2s->y,l2e->x,l2e->y,&(point->x),&(point->y));
}

int mLineRectCrossCheck(MImagePoint *ls,MImagePoint *le,MImageRect *rect)
{
    if((ls->x<rect->x1)&&(le->x<rect->x1)) return 0;
    if((ls->x>rect->x2)&&(le->x>rect->x2)) return 0;
    if((ls->y<rect->y1)&&(le->y<rect->y1)) return 0;
    if((ls->y>rect->y2)&&(le->y>rect->y2)) return 0;
    if(LineCrossCheck(ls->x,ls->y,le->x,le->y,rect->x1,rect->y1,rect->x2,rect->y1)) return 1;
    if(LineCrossCheck(ls->x,ls->y,le->x,le->y,rect->x1,rect->y1,rect->x1,rect->y2)) return 1;
    if(LineCrossCheck(ls->x,ls->y,le->x,le->y,rect->x2,rect->y1,rect->x2,rect->y2)) return 1;
    if(LineCrossCheck(ls->x,ls->y,le->x,le->y,rect->x1,rect->y2,rect->x2,rect->y2)) return 1;
    return 0;
}

float mLineAngle(MImagePoint *l1s,MImagePoint *l1e,MImagePoint *l2s,MImagePoint *l2e)
{
    MImagePoint pt;pt.x=0;pt.y=0;
    if(l1s==NULL){l1s=&pt;}if(l1e==NULL){l1e=&pt;}if(l2s==NULL){l2s=&pt;}if(l2e==NULL){l2e=&pt;}
    float a1;
    if(l1s==l1e) {a1=0;} 
    else if(l1s->x==l1e->x) a1=(l1s->y>l1e->y)?(0-MORN_PI/2):MORN_PI/2;
    else {a1=atan((l1s->y-l1e->y)/(l1s->x-l1e->x));if(l1s->x>l1e->x) {if(l1s->y>l1e->y) {a1-=MORN_PI;} else {a1+=MORN_PI;}}}

    float a2;if(l2s==l2e) {a2=0;}
    else if(l2s->x==l2e->x) a2=(l2s->y>l2e->y)?(0-MORN_PI/2):MORN_PI/2;
    else {a2=atan((l2s->y-l2e->y)/(l2s->x-l2e->x));if(l2s->x>l2e->x) {if(l2s->y>l2e->y) {a2-=MORN_PI;} else {a2+=MORN_PI;}}}

    return ((a1-a2)*180.0/MORN_PI);
}

#define INTEGRAL_AREA(P1x,P1y,P2x,P2y) (((P1x)-(P2x))*((P1y)+(P2y)))

float mPolygonArea(MList *polygon)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input");
    mException((polygon->num<3),EXIT,"invalid input");
    MImagePoint **point = (MImagePoint **)(polygon->data);
    
    float area = INTEGRAL_AREA(point[polygon->num-1]->x,point[polygon->num-1]->y,point[0]->x,point[0]->y);
    for(int i=1;i<polygon->num;i++)
        area = area + INTEGRAL_AREA(point[i-1]->x,point[i-1]->y,point[i]->x,point[i]->y);
    
    return (ABS(area))/2.0f;
}

float TriangleArea(float x1,float y1,float x2,float y2,float x3,float y3)
{
    return ABS(INTEGRAL_AREA(x1,y1,x2,y2)+INTEGRAL_AREA(x2,y2,x3,y3)+INTEGRAL_AREA(x3,y3,x1,y1))/2.0f;
}

float mTriangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3)
{
    return TriangleArea(p1->x,p1->y,p2->x,p2->y,p3->x,p3->y);
}

float QuadrangleArea(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4)
{
    return ABS(INTEGRAL_AREA(x1,y1,x2,y2)+INTEGRAL_AREA(x2,y2,x3,y3)
              +INTEGRAL_AREA(x3,y3,x4,y4)+INTEGRAL_AREA(x4,y4,x1,y1))/2.0f;
}

float mQuadrangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3,MImagePoint *p4)
{
    return QuadrangleArea(p1->x,p1->y,p2->x,p2->y,p3->x,p3->y,p4->x,p4->y);
}

double PointVerticalDistance(double px,double py,double lx1,double ly1,double lx2,double ly2,float *vx,float *vy)
{
    double v_x,v_y,l;
    double a=ly2-ly1;
    double b=lx2-lx1;
         if(a==0) {v_x=px; v_y=ly1;l=ABS(py-v_y);}
    else if(b==0) {v_x=lx1;v_y=py; l=ABS(px-v_x);}
    else
    {
        double c=a*lx2-b*ly2;
        double d=a*py+b*px;
        
        double buff = (a*a+b*b);
        v_x = (c*a+b*d)/buff;
        v_y = (a*v_x-c)/b;
        l=sqrt((px-v_x)*(px-v_x)+(py-v_y)*(py-v_y));
    }
    
    if(vx!=NULL) *vx = v_x;
    if(vy!=NULL) *vy = v_y;
    
    return l;
}

float mPointVerticalDistance(MImagePoint *point,MImagePoint *p1,MImagePoint *p2,MImagePoint *pedal)
{
    if(pedal == NULL)
        return PointVerticalDistance(point->x,point->y,p1->x,p1->y,p2->x,p2->y,NULL,NULL);
    else
        return PointVerticalDistance(point->x,point->y,p1->x,p1->y,p2->x,p2->y,&(pedal->x),&(pedal->y));
}

float LineAngle(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2)
{
    float px,py;
    LineCrossPoint(l1x1,l1y1,l1x2,l1y2,l2x1,l2y1,l2x2,l2y2,&px,&py);
    if(mIsNan(px)) return 0.0f;
    
    float l,d;
    d=PointVerticalDistance(l1x2,l1y2,l2x1,l2y1,l2x2,l2y2,NULL,NULL);
    if(d<=0.0f)
    {
        d=PointVerticalDistance(l1x1,l1y1,l2x1,l2y1,l2x2,l2y2,NULL,NULL);
        l=sqrt((px-l1x1)*(px-l1x1)+(py-l1y1)*(py-l1y1));
    }
    else
        l=sqrt((px-l1x2)*(px-l1x2)+(py-l1y2)*(py-l1y2));
        
    return asin(d/l)*180/MORN_PI;
}
// #define mPointInRect(Point,Rect) (((Point)->x>(Rect)->x1)&&((Point)->x<(Rect)->x2)&&((Point)->y>(Rect)->y1)&&((Point)->y<(Rect)->y2))

int PointInPolygon(double x,double y,MList *polygon)
{
    MImagePoint **vertex = (MImagePoint **)(polygon->data);
    int n=0;
    int num = polygon->num-1;
    if((vertex[0]->x>x)==(x>=vertex[num]->x))
    {
        double k = (vertex[0]->y - vertex[num]->y)/(vertex[0]->x - vertex[num]->x);
        double b = vertex[0]->y - (k*vertex[0]->x);
        double ly = k*x+b;if(ly==y) return 1;
        n = n+(ly<y);
        // printf("aaaak=%f,b=%f,ly=%f,y=%f,n=%d\n",k,b,ly,y,n);
    }
    for(int i=0;i<num;i++)
    {
        if((vertex[i]->x>x)==(x>=vertex[i+1]->x))
        {
            double k = (vertex[i+1]->y - vertex[i]->y)/(vertex[i+1]->x - vertex[i]->x);
            double b = vertex[i]->y - (k*vertex[i]->x);
            double ly = k*x+b;if(ly==y) return 1;
            n = n+(ly<y);
            // printf("k=%f,b=%f,ly=%f,y=%f,n=%d\n",k,b,ly,y,n);
        }
    }
    return ((n%2)==1);
}

int mPointInPolygon(MImagePoint *point,MList *polygon)
{
    return PointInPolygon(point->x,point->y,polygon);
}

void mCircumCircle(MImagePoint *p0,MImagePoint *p1,MImagePoint *p2,MImageCircle *circle)
{
    double x1=(p0->x+p1->x)/2.0;double y1=(p0->y+p1->y)/2.0;double k1=(p0->x+p1->x)/(p1->y-p0->y);double b1=y1-k1*x1;
    double x2=(p2->x+p1->x)/2.0;double y2=(p2->y+p1->y)/2.0;double k2=(p2->x+p1->x)/(p1->y-p2->y);double b2=y2-k2*x2;
    double x0=(b2-b1)/(k1-k2);double y0=k1*x0+b1;
    circle->center.x=x0; circle->center.y=y0;
    circle->r=mPointDistance(&(circle->center),p1);
}

float mRectIntersetArea(MImageRect *rect1,MImageRect *rect2)
{
    int x1=MAX(rect1->x1,rect2->x1);int x2=MIN(rect1->x2,rect2->x2);
    if(x1>=x2)return 0;
    int y1=MAX(rect1->y1,rect2->y1);int y2=MIN(rect1->y2,rect2->y2);
    if(y1>=y2)return 0;
    return ((y2-y1)*(x2-x1));
}
float mRectUnionsetArea(MImageRect *rect1,MImageRect *rect2)
{
    int area1 = (rect1->x2-rect1->x1)*(rect1->y2-rect1->y1);
    int area2 = (rect2->x2-rect2->x1)*(rect2->y2-rect2->y1);
    return (area1+area2-mRectIntersetArea(rect1,rect2));
}

#define CLOSS_POINT(I1,I2) LineCrossPoint(v1[I1]->x,v1[I1]->y,v1[I1+1]->x,v1[I1+1]->y,v2[I2]->x,v2[I2]->y,v2[I2+1]->x,v2[I2+1]->y,&(point.x),&(point.y));


float mCircleIntersetArea(MImageCircle *circle1,MImageCircle *circle2)
{
    float c1_area = mCircleArea(circle1);
    float c2_area = mCircleArea(circle2);
    float u=mPointDistance(&(circle1->center),&(circle2->center));
    if(u>=circle1->r+circle2->r) return 0.0f;
    if(u<=circle1->r-circle2->r) return c2_area;
    if(u<=circle2->r-circle1->r) return c1_area;
    
    float d=((circle1->r*circle1->r)-(circle2->r*circle2->r))/u;
    float r1=(u+d)/2.0f; float r2=(u-d)/2.0f;
    
    float a1=acos(r1/circle1->r);
    float a2=acos(r2/circle2->r);
    
    float l=sqrt(circle1->r*circle1->r-r1*r1);
    float area1 = c1_area*a1/(MORN_PI)-(l*r1);
    float area2 = c2_area*a2/(MORN_PI)-(l*r2);
    
    return (area1+area2);
}



float mPolygonIntersetArea(MList *polygon1,MList *polygon2)
{
    int i,j;
    float area;MImagePoint point;
    int valid;
    MImagePoint **v1 = (MImagePoint **)(polygon1->data);
    MImagePoint **v2 = (MImagePoint **)(polygon2->data);
    int n1=polygon1->num;int n2=polygon2->num;int n=n1+n2;
    MList *polygon;
    
    for(i=0;i<n1;i++)for(j=0;j<n2;j++)
    {
        if((v1[i]->x==v2[j]->x)&&(v1[i]->y==v2[j]->y))
        {
            v1[i]->x += (float)mRand(-8,7)/10000.0f;
            v1[i]->y += (float)mRand(-8,7)/10000.0f;
            j=-1;
        }
    }
    
    MMatrix *x_mat = mMatrixCreate(n,n,NULL,DFLT);
    MMatrix *y_mat = mMatrixCreate(n,n,NULL,DFLT);
    for(int i=0;i<n;i++)
    {
        memset(x_mat->data[i],0,n*sizeof(float));
        memset(y_mat->data[i],0,n*sizeof(float));
    }
    int *num1 = (int *)mMalloc(n1*sizeof(int));memset(num1,0,n1*sizeof(int));
    int *num2 = (int *)mMalloc(n2*sizeof(int));memset(num2,0,n2*sizeof(int));
    int lx,ly;
    
    mListAppend(polygon1,DFLT);polygon1->data[n1]=polygon1->data[0];polygon1->num=n1;
    mListAppend(polygon2,DFLT);polygon2->data[n2]=polygon2->data[0];polygon2->num=n2;
    
    int count = 0;
    for(i=0;i<n1;i++)
        for(j=0;j<n2;j++)
        {
            valid = CLOSS_POINT(i,j);
            
            if(valid==1) 
            {
                // printf("aa i is %d,j is %d,point is %f,%f\n",i,n1+j,point.x,point.y);
                x_mat->data[i][n1+j]=point.x;x_mat->data[n1+j][i]=point.x;
                y_mat->data[i][n1+j]=point.y;y_mat->data[n1+j][i]=point.y;
                num1[i]+=1;num2[j]+=1;
                ly=i;lx=n1+j; 
                count +=1;
            }
        }
    if(count == 0) {area = 0.0f;goto PolygonIntersetArea_end;}
    
    valid = mPointInPolygon(v1[0],polygon2);
    for(i=0;i<n1;i++)
    {
        if(valid==1)
        {
            j=i-1;if(j<0)j=n1-1;
            // printf("bb i is %d,j is %d,point is %f,%f\n",i,j,v1[i]->x,v1[i]->y);
            x_mat->data[i][j]=v1[i]->x;x_mat->data[j][i]=v1[i]->x;
            y_mat->data[i][j]=v1[i]->y;y_mat->data[j][i]=v1[i]->y;
        }
        if(num1[i]==1) valid=1-valid;
        mException((num1[i]>2),EXIT,"invalid input polygon");
    }
    
    valid = mPointInPolygon(v2[0],polygon1);
    for(j=0;j<n2;j++)
    {
        if(valid==1)
        {
            i=j-1;if(i<0)i=n2-1;
            // printf("bb i is %d,j is %d,point is %f,%f\n",n1+i,n1+j,v2[j]->x,v2[j]->y);
            x_mat->data[n1+i][n1+j]=v2[j]->x;x_mat->data[n1+j][n1+i]=v2[j]->x;
            y_mat->data[n1+i][n1+j]=v2[j]->y;y_mat->data[n1+j][n1+i]=v2[j]->y;
        }
        if(num2[j]==1) valid=1-valid;
        mException((num2[j]>2),EXIT,"invalid input polygon");
    }
    
    polygon = mListCreate(DFLT,NULL);
    
    PolygonIntersetArea_next:
    point.x = x_mat->data[ly][lx];x_mat->data[ly][lx]=0.0f;x_mat->data[lx][ly]=0.0f;
    point.y = y_mat->data[ly][lx];y_mat->data[ly][lx]=0.0f;y_mat->data[lx][ly]=0.0f;
    // printf("ly is %d,lx is %d,x is %f,y is %f\n",ly,lx,point.x,point.y);
    mListWrite(polygon,DFLT,&point,sizeof(MImagePoint));
    
    for(i=lx+1;;i++)
    {
        if(i==n) i=0;
        if(i==lx)break;
        if((x_mat->data[ly][i]!=0.0f)||(y_mat->data[ly][i]!=0.0f))
        {
            lx = i;goto PolygonIntersetArea_next;
        }
    }
    for(j=ly+1;;j++)
    {
        if(j==n) j=0;
        if(j==ly)break;
        if((x_mat->data[j][lx]!=0.0f)||(y_mat->data[j][lx]!=0.0f))
        {
            ly = j;goto PolygonIntersetArea_next;
        }
    }
    
    area = mPolygonArea(polygon);
    mListRelease(polygon);
    
    PolygonIntersetArea_end:
    mMatrixRelease(x_mat);
    mMatrixRelease(y_mat);
    mFree(num1);
    mFree(num2);
    return area;
}


int PolygonCross(MList *polygon1,MList *polygon2)
{
    int n1,n2;
    MImagePoint **v1,**v2;
    MImagePoint *p,*p1,*p2;
    float result,flag;
    
    int i,j;
    
    n1 = polygon1->num;
    n2 = polygon2->num;
    v1 = (MImagePoint **)(polygon1->data);
    v2 = (MImagePoint **)(polygon2->data);
    
    p = v1[n1-2];
    p1 = v1[n1-1];
    p2 = v1[0];
    result = PointLineCheck(p->x,p->y,p1->x,p1->y,p2->x,p2->y);
    if(result==0.0f)
        return 1;
    for(i=0;i<n2;i++)
    {
        flag = PointLineCheck(v2[i]->x,v2[i]->y,p1->x,p1->y,p2->x,p2->y);
        if((flag>0)==(result>0))
            break;
    }
    if(i==n2)
        return 0;
    
    for(j=1;j<n1;j++)
    {
        p=p1;p1=p2;p2=v1[j];
        result = PointLineCheck(p->x,p->y,p1->x,p1->y,p2->x,p2->y);
        if(result==0.0f)
            return 1;
        for(i=0;i<n2;i++)
        {
            flag = PointLineCheck(v2[i]->x,v2[i]->y,p1->x,p1->y,p2->x,p2->y);
            if((flag>0)==(result>0))
                break;
        }
        if(i==n2)
            return 0;
    }
    return 1;
}

int mPolygonCross(MList *polygon1,MList *polygon2)
{
    if(PolygonCross(polygon1,polygon2)==0)
        return 0;
    if(PolygonCross(polygon2,polygon1)==0)
        return 0;
    
    return 1;
}


int mPolygonConcaveCheck(MList *polygon)
{
    int i;
    MImagePoint point;
    MImagePoint **vertex;
    
    vertex = (MImagePoint **)(polygon->data);
    point.x=0;
    point.y=0;
    for(i=0;i<polygon->num;i++)
    {
        point.x += vertex[i]->x;
        point.y += vertex[i]->y;
    }
    point.x = point.x/(float)(polygon->num);
    point.y = point.y/(float)(polygon->num);
    
    return mPointInPolygon(&point,polygon);
}

void mShapeBounding(MList *shape,MList *bounding)
{
    mException(INVALID_POINTER(shape),EXIT,"invalid input shape");
    
    MImagePoint **point = (MImagePoint **)(shape->data);
    float min_area = mSup();//10000.0f*10000.0f;
    float a = 0.0f;
    int point_min1=0;int point_max1=0;
    int point_min2=0;int point_max2=0;
    float k1,k2,b1,b2,sn1,sn2,cs1,cs2;
    
    for(int i=0;i<90;i++)
    {
        float sn1 = mSin((float)i);
        float cs1 = mCos((float)i);
        float sn2 = cs1;
        float cs2 = 0.0-sn1;
        
        float min1 = (point[0]->y)*cs1-(point[0]->x)*sn1; float max1=min1;
        float min2 = (point[0]->y)*cs2-(point[0]->x)*sn2; float max2=min2;
        int max1_index = 0; int min1_index = 0;
        int max2_index = 0; int min2_index = 0;
        
        for(int j=0;j<shape->num;j++)
        {
            float l1 = (point[j]->y)*cs1-(point[j]->x)*sn1;
            float l2 = (point[j]->y)*cs2-(point[j]->x)*sn2;
            if(l1<min1) {min1 = l1; min1_index = j;}else if(l1>max1) {max1 = l1; max1_index = j;}
            if(l2<min2) {min2 = l2; min2_index = j;}else if(l2>max2) {max2 = l2; max2_index = j;}
        }
        
        float area = (max1-min1)*(max2-min2);
        if(area<min_area)
        {
            min_area = area;
            a = (float)i;
            point_min1 = min1_index;
            point_max1 = max1_index;
            point_min2 = min2_index;
            point_max2 = max2_index;
        }
    }
    
    // printf("point_max2 is %d\n",point_max2);
    // printf("point[point_min1] is %f,%f\n",point[point_min1]->x,point[point_min1]->y);
    // printf("point[point_max1] is %f,%f\n",point[point_max1]->x,point[point_max1]->y);
    // printf("point[point_min2] is %f,%f\n",point[point_min2]->x,point[point_min2]->y);
    // printf("point[point_max2] is %f,%f\n",point[point_max2]->x,point[point_max2]->y);
    
    MImagePoint rst[4];
    if(bounding == NULL) bounding = shape;
    
    if(a==0.0f)
    {
        rst[0].x = point[point_min2]->x;rst[0].y = point[point_min1]->y;
        rst[1].x = point[point_min2]->x;rst[1].y = point[point_max1]->y;
        rst[2].x = point[point_max2]->x;rst[2].y = point[point_max1]->y;
        rst[3].x = point[point_max2]->x;rst[3].y = point[point_min1]->y;
        goto ShapeBoundingNext;
    }
    
    sn1 = mSin(a);cs1 = mCos(a);sn2 = cs1;cs2 = 0.0-sn1;

    k1 = sn1/cs1;k2 = sn2/cs2;
    // printf("a is %f\n",a);
    // printf("sn1 is %f,cs1 is %f\n",sn1,cs1);
    // printf("k1 is %f,k2 is %f\n",k1,k2);
    b1 = point[point_min1]->y - k1*point[point_min1]->x;
    b2 = point[point_min2]->y - k2*point[point_min2]->x;
    
    // printf("b1 is %f,b2 is %f\n",b1,b2);
    
    rst[0].x = (b1-b2)/(k2-k1);
    rst[0].y = ((k1+k2)*rst[0].x +b1 +b2)/2.0;
    // printf("rst[0] is %f,%f\n",rst[0].x,rst[0].y);
    // printf("aaaa point[point_max2] is %f,%f\n",point[point_max2]->x,point[point_max2]->y);
    
    
    // printf("aaaaaaaaa point[point_max2] is %f,%f\n",point[point_max2]->x,point[point_max2]->y);
    b2 = point[point_max2]->y - k2*point[point_max2]->x;
    // printf("b1 is %f,b2 is %f\n",b1,b2);

    rst[1].x = (b1-b2)/(k2-k1);
    rst[1].y = ((k1+k2)*rst[1].x +b1 +b2)/2.0;
    // printf("rst[1] is %f,%f\n",rst[1].x,rst[1].y);
    
    
    b1 = point[point_max1]->y - k1*point[point_max1]->x;

    rst[2].x = (b1-b2)/(k2-k1);
    rst[2].y = ((k1+k2)*rst[2].x +b1 +b2)/2.0;
    // printf("rst[2] is %f,%f\n",rst[2].x,rst[2].y);
    

    b2 = point[point_min2]->y - k2*point[point_min2]->x;
    
    rst[3].x = (b1-b2)/(k2-k1);
    rst[3].y = ((k1+k2)*rst[3].x +b1 +b2)/2.0;
    // printf("rst[3] is %f,%f\n",rst[3].x,rst[3].y);
    
    ShapeBoundingNext:
    mListWrite(bounding,0,&rst[0],sizeof(MImagePoint));
    mListWrite(bounding,1,&rst[1],sizeof(MImagePoint));
    mListWrite(bounding,2,&rst[2],sizeof(MImagePoint));
    mListWrite(bounding,3,&rst[3],sizeof(MImagePoint));
    
    bounding->num = 4;
}


void ConvexHull(MList *point,MChain *chain,MChainNode *node)
{
    MImagePoint *p1 = (MImagePoint *)(node->data);
    MImagePoint *p2 = (MImagePoint *)(node->next->data);
    MImagePoint **p = (MImagePoint **)(point->data);
    
    float ky = p1->x - p2->x; float kx= p1->y -p2->y; float b=kx*p2->x - ky*p2->y;
    
    MImagePoint *buff;
    MImagePoint value;
    
    int n = 0;
    
    float min = 0;
    for(int i=0;i<point->num;i++)
    {
        float v = kx*p[i]->x -ky*p[i]->y -b;
        if(v<0)
        {
            if(v<min) {min = v;value.x = p[i]->x;value.y = p[i]->y;}
            buff = p[n];p[n] = p[i]; p[i] = buff; n=n+1;
        }
    }
    
    if(min<-1.0f)
    {
        MChainNode *next = mChainNode(chain,&value,sizeof(MImagePoint));
        mChainNodeInsert(node,next,NULL);
        
        if(n>1) {point->num = n; ConvexHull(point,chain,node);}
        if(n>1) {point->num = n; ConvexHull(point,chain,next);}
    }
}

void mConvexHull(MList *point,MList *polygon)
{
    mException(INVALID_POINTER(point),EXIT,"invalid input");
    mException((point->num<3),EXIT,"invalid input");
    
    MList *pout = polygon;
    if((polygon == NULL)||(polygon == point))
        polygon = mListCreate(DFLT,NULL);
    polygon->num =0;
    
    MImagePoint **p = (MImagePoint **)(point->data);
    
    int i;
    float x_max = p[0]->x;float x_min = x_max;
    float y_max = p[0]->y;float y_min = y_max;
    int i1=0;int i2=0;int i3=0;int i4=0;
    for(i=1;i<point->num;i++)
    {
        if(p[i]->x > x_max) {x_max = p[i]->x;i1=i;}else if(p[i]->x < x_min) {x_min = p[i]->x;i3=i;}
        if(p[i]->y > y_max) {y_max = p[i]->y;i2=i;}else if(p[i]->y < y_min) {y_min = p[i]->y;i4=i;}
    }

    // printf("i1 is %d,p[i1]->x is %f,p[i1]->y is %f\n",i1,p[i1]->x,p[i1]->y);
    // printf("i2 is %d,p[i2]->x is %f,p[i2]->y is %f\n",i2,p[i2]->x,p[i2]->y);
    // printf("i3 is %d,p[i3]->x is %f,p[i3]->y is %f\n",i3,p[i3]->x,p[i3]->y);
    // printf("i4 is %d,p[i4]->x is %f,p[i4]->y is %f\n",i4,p[i4]->x,p[i4]->y);
    double kx1=0,ky1=0,b1=0; if(i1 != i2) {ky1=p[i1]->x - p[i2]->x;kx1=p[i1]->y - p[i2]->y; b1=kx1*p[i1]->x - ky1*p[i1]->y;}
    double kx2=0,ky2=0,b2=0; if(i2 != i3) {ky2=p[i2]->x - p[i3]->x;kx2=p[i2]->y - p[i3]->y; b2=kx2*p[i2]->x - ky2*p[i2]->y;}
    double kx3=0,ky3=0,b3=0; if(i3 != i4) {ky3=p[i3]->x - p[i4]->x;kx3=p[i3]->y - p[i4]->y; b3=kx3*p[i3]->x - ky3*p[i3]->y;}
    double kx4=0,ky4=0,b4=0; if(i4 != i1) {ky4=p[i4]->x - p[i1]->x;kx4=p[i4]->y - p[i1]->y; b4=kx4*p[i4]->x - ky4*p[i4]->y;}
   
    MList *list1 = mListCreate(point->num); int n1=0;
    MList *list2 = mListCreate(point->num); int n2=0;
    MList *list3 = mListCreate(point->num); int n3=0;
    MList *list4 = mListCreate(point->num); int n4=0;
    for(i=0;i<point->num;i++)
    {
        if(i1 != i2) if(kx1*p[i]->x - ky1*p[i]->y < b1) {list1->data[n1] = p[i]; n1=n1+1; continue;}
        if(i2 != i3) if(kx2*p[i]->x - ky2*p[i]->y < b2) {list2->data[n2] = p[i]; n2=n2+1; continue;}
        if(i3 != i4) if(kx3*p[i]->x - ky3*p[i]->y < b3) {list3->data[n3] = p[i]; n3=n3+1; continue;}
        if(i4 != i1) if(kx4*p[i]->x - ky4*p[i]->y < b4) {list4->data[n4] = p[i]; n4=n4+1; continue;}
    }
    list1->num = n1; list2->num = n2; list3->num = n3; list4->num = n4;
    // printf("n is %d,%d,%d,%d\n",list1->num,list2->num,list3->num,list4->num);
    
    MChain *chain = mChainCreate();
    MChainNode *node1=NULL; if(i1!=i2) {node1 = mChainNode(chain,NULL,DFLT);node1->data = p[i1];                         chain->object = (void *)node1;}
    MChainNode *node2=NULL; if(i2!=i3) {node2 = mChainNode(chain,NULL,DFLT);node2->data = p[i2]; if(chain->object==NULL) chain->object = (void *)node2; else mChainNodeInsert(NULL,node2,(MChainNode *)(chain->object));}
    MChainNode *node3=NULL; if(i3!=i4) {node3 = mChainNode(chain,NULL,DFLT);node3->data = p[i3]; if(chain->object==NULL) chain->object = (void *)node3; else mChainNodeInsert(NULL,node3,(MChainNode *)(chain->object));}
    MChainNode *node4=NULL; if(i4!=i1) {node4 = mChainNode(chain,NULL,DFLT);node4->data = p[i4]; if(chain->object==NULL) chain->object = (void *)node4; else mChainNodeInsert(NULL,node4,(MChainNode *)(chain->object));}
    
    if(n1>0) ConvexHull(list1,chain,node1);
    if(n2>0) ConvexHull(list2,chain,node2);
    if(n3>0) ConvexHull(list3,chain,node3);
    if(n4>0) ConvexHull(list4,chain,node4);
    
    MChainNode *node = (MChainNode *)(chain->object);
    polygon->num = 0;
    while(1)
    {
        mListWrite(polygon,DFLT,node->data,sizeof(MImagePoint));
        node = node->next;
        if(node == chain->object)
            break;
    }

    mListRelease(list1);
    mListRelease(list2);
    mListRelease(list3);
    mListRelease(list4);
    mChainRelease(chain);

    if(polygon->num)
    {
        mListWrite(polygon,DFLT,polygon->data[0],sizeof(MImagePoint));
        polygon->num=polygon->num-1;
    }
    
    if(pout != polygon) {mObjectExchange(polygon,point,MList);mListRelease(polygon);}
}

void _EdgeBoundary(MList *edge,MChain *chain,MChainNode *node,float thresh)
{
    MImagePoint **p=(MImagePoint **)(edge->data);
    int b=*(int *)(node->data);int e=*(int *)(node->next->data);
    MImagePoint *pb= p[b];MImagePoint *pe= p[(e==edge->num)?0:e];
    
    float max=0;int idx;
    for(int i=b+1;i<e;i++)
    {
        float l=mPointVerticalDistance(p[i],pb,pe,NULL);
        if(l>max) {max=l;idx=i;}
    }
    // double k,b;
    // double k = (double)(p[e]->y-p[b]->y)/(double)(p[e]->x-p[b]->x);
    
    // printf("max is %f\n",max);
    if(max<thresh) return;
    // printf("b is %d,e=%d,idx=%d\n",b,e,idx);
    MChainNode *node0 = mChainNode(chain,&idx,sizeof(int));
    mChainNodeInsert(node,node0,NULL);
    _EdgeBoundary(edge,chain,node ,thresh);
    _EdgeBoundary(edge,chain,node0,thresh);
}

void mEdgeBoundary(MList *edge,MList *polygon,int thresh)
{
    mException(edge==NULL,EXIT,"invalid input");
    if(thresh<=0) thresh = 1;
    
    mListClear(polygon);
    int num = edge->num;if(num==0) return;

    MImagePoint **p=(MImagePoint **)(edge->data);
    // printf("num=%d,p[0] is %f,%f\n",num,p[0]->x,p[0]->y);
    
    MChain *chain = mChainCreate();

    MChainNode *node1 = mChainNode(chain,NULL,sizeof(int)); chain->chainnode=node1;
    MChainNode *node2 = mChainNode(chain,NULL,sizeof(int)); mChainNodeInsert(node1,node2,NULL);
    *(int *)(node2->data) = num/2;

    *(int *)(node1->data) = num;
    _EdgeBoundary(edge,chain,node2,1);
    
    *(int *)(node1->data) = 0;  
    _EdgeBoundary(edge,chain,node1,1);

    node1 = chain->chainnode;
    node2 = node1->next->next;
    while(node1!=node2)
    {
        int i1 = *(int *)(node1->data);
        if(i1>=0)
        {
            int i2 = *(int *)(node2->data);if(i2<0) {node2 = node2->next;continue;}
            MChainNode *node;
            for(node=node1->next;node!=node2;node=node->next)
            {
                int i = *(int *)(node->data); 
                int i0=(i<0)?(-1-i):i;
                float l=mPointVerticalDistance(p[i0],p[i1],p[i2],NULL);
                if(l>thresh) break;
                else if(i>0) {*(int *)(node->data)=-1-i;}
            }
            if(node==node2) {node2=node2->next; continue;}
        }
        node1 = node1->next;if(node1==chain->chainnode) break;
        node2 = node1->next->next;
    }
    node1 = chain->chainnode;
    while(1)
    {
        int i = *(int *)(node1->data);
        if(i>=0) mListWrite(polygon,DFLT,p[i],sizeof(MImagePoint));
        node1 = node1->next;if(node1==chain->chainnode) break;
    }
    
    if(polygon->num)
    {
        mListWrite(polygon,DFLT,polygon->data[0],sizeof(MImagePoint));
        polygon->num=polygon->num-1;
    }

    mChainRelease(chain);
}

int m_PolygonSimilar(MList *polygon1,MList *polygon2,int flag)
{
    mException((polygon1==NULL)||(polygon2==NULL),EXIT,"invalid input");
    int n =polygon1->num; if(n!=polygon2->num) return 0;
    float *a1 = malloc(n*4*sizeof(float));float *a2=a1+n+n;

    MImagePoint **p1 = (MImagePoint **)polygon1->data;
    MImagePoint **p2 = (MImagePoint **)polygon2->data;

    a1[0]=atan((p1[n-1]->y-p1[0]->y)/(p1[n-1]->x-p1[0]->x));a1[n]=a1[0];
    a2[0]=atan((p2[n-1]->y-p2[0]->y)/(p2[n-1]->x-p2[0]->x));a2[n]=a2[0];
    for(int i=1;i<n;n++)
    {
        a1[i]=atan((p1[0]->y-p1[1]->y)/(p1[0]->x-p1[1]->x));a1[i-1]=a1[i]-a1[i-1];
        a2[i]=atan((p2[0]->y-p2[1]->y)/(p2[0]->x-p2[1]->x));a2[i-1]=a2[i]-a2[i-1];
    }
    a1[n-1]=a1[n]-a1[n-1];
    a2[n-1]=a2[n]-a2[n-1];
    memcpy(a2+n,a2,n*sizeof(float));
    
    int j,i1,i2;
    for(j=0;j<n;j++)
    {
        for(i1=0,i2=j;i1<n;i1++,i2++)
        {
            if(ABS(a1[i1]-a2[i2])>ABS(a1[i1]+a2[i2])*0.01) break;
        }
        if(i1==n) break;
    }
    if(j==n) return 0;
    
    if(flag==1)
    {
        float d1 = (p1[0]->y-p1[n-1]->y)*(p1[0]->y-p1[n-1]->y)+(p1[0]->x-p1[n-1]->x)*(p1[0]->x-p1[n-1]->x);
        float d2 = (p1[j]->y-p1[j-1]->y)*(p1[j]->y-p1[j-1]->y)+(p1[j]->x-p1[j-1]->x)*(p1[j]->x-p1[j-1]->x);
        if(ABS(d1-d2)>(d1+d2)*0.01) return 0;
    }
    
    return 1;
}
/*
DelaunayGrow(M

void Delaunay(MList *point,MList *rst_line)
{
    MImagePoint **pt = (MImagePoint **)(point->data);
    float xmin=(pt[0]->x);float xmax=xmin;
    float ymin=(pt[0]->y);float ymax=ymin;
    for(int i=1;i<point->num;i++)
    {
        xmin=MIN(pt[i].x,xmin);xmax=MAX(pt[i].xmax);
        ymin=MIN(pt[i].y,ymin);ymax=MAX(pt[i].ymax);
    }

    int d=(int)sqrt((double)(point->num/2));
    float sx=(xmax-xmin)/d;float sy=(ymax-ymin)/d;
    float dmax = xmax+ymax-xmin-ymin;
    
    struct PointInfo
    {
        MImagePoint pt;
        MImagePoint *nb;
        float d;
        int count;
    };
    MSheet *sheet_point = mSheetCreate(d*d,NULL);
    struct PointInfo info;
    for(int i=0;i<point->num;i++)
    {
        int lx=(int)(pt[i]->x/sx +0.5);
        int ly=(int)(pt[i]->y/sy +0.5);
        info.pt=*(pt[i]);
        info.nb=NULL;
        info.d=dmax;
        mSheetWrite(sheet,ly*d+lx,DFLT,&info,sizeof(struct PointInfo));
    }
    for(int j=0;j<d-1;j++)for(int i=0;i<d-1;i++)
    {
        int i0=j*d+i;int i1=i0+1;int i2=i0+d;int i3=i2+1;
        
        for(int k=0;k<sheet->col[i0*i1];k++)
        mSheetLine
    }
    
        
    mSheetRelease(sheet);
}
*/