/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "morn_image.h"

void ImagePolygonBorder(MArray *border,int height,int width,MList *polygon)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input polygon");
    mException((polygon->num<3),EXIT,"invalid input polygon");
    mException(INVALID_POINTER(border),EXIT,"invalid input");
    mArrayRedefine(border,height*2,S16,border->dataS16);
    
    MImagePoint **point = (MImagePoint **)(polygon->data);

    char *str;
    str=&(border->info.name[0][0]);if(strlen(str)>0) mException((strcmp(str,"y1"    )!=0),EXIT,"invalid border");else strcpy(str,"y1");
    str=&(border->info.name[1][0]);if(strlen(str)>0) mException((strcmp(str,"y2"    )!=0),EXIT,"invalid border");else strcpy(str,"y2");
    str=&(border->info.name[2][0]);if(strlen(str)>0) mException((strcmp(str,"height")!=0),EXIT,"invalid border");else strcpy(str,"height");
    str=&(border->info.name[3][0]);if(strlen(str)>0) mException((strcmp(str,"width" )!=0),EXIT,"invalid border");else strcpy(str,"width");

    int y1 = (int)(point[0]->y+0.5f);int y2 = (int)(point[0]->y+0.5f);
    for(int n=1;n<polygon->num;n++)
        {y1 = MIN(y1,(int)(point[n]->y+0.5f));y2 = MAX(y2,(int)(point[n]->y+0.5f));}
    y1 = MAX(y1,0);y2 = MIN(y2+1,height);
    printf("y1 is %d,y2 is %d\n",y1,y2);
    border->info.value[0]=y1;border->info.value[1]=y2;border->info.value[2]=height;border->info.value[3]=width;

    memset(border->dataS16,0,border->num*sizeof(short));
    
    for(int n=0;n<polygon->num;n++)
    {
        MImagePoint *p1,*p2;
        int lx,ly;

        p1 = point[n];
        if(n+1<polygon->num)
            p2 = point[n+1];
        else
            p2 = point[0];

        if(p1->y==p2->y)
        {
            ly=(short)(p1->y + 0.5f);
            if((ly>=0)&&(ly<height))
            {
                if(p1->x<p2->x)
                {
                    border->dataS16[ly+ly  ] = p1->x;
                    border->dataS16[ly+ly+1] = p2->x;
                }
                else
                {
                    border->dataS16[ly+ly  ] = p2->x;
                    border->dataS16[ly+ly+1] = p1->x;
                }
            }
            continue;
        }

        float x_locate = (p1->x);
        float step = (p2->x - p1->x)/(p2->y - p1->y);
        step = (p1->y>p2->y)?(0.0f-step):step;
        
        for(ly=(int)(p1->y+0.5f);;ly=((p1->y>p2->y)?(ly-1):(ly+1)))
        {
            lx = (float)(x_locate+0.5);
            
            if(lx<0) lx=0; else if(lx>width) lx=width;
            
            if((ly>=0)&&(ly<height))
            {
                if( border->dataS16[ly+ly] == 0)
                    border->dataS16[ly+ly] = lx;
                else if(lx<border->dataS16[ly+ly])
                {
                    border->dataS16[ly+ly+1] = border->dataS16[ly+ly];
                    border->dataS16[ly+ly  ] = lx;
                }
                else if(lx>border->dataS16[ly+ly+1])
                    border->dataS16[ly+ly+1] = lx;
            }
            
            if(ly==(int)(p2->y+0.5f))
                break;
                
            x_locate = x_locate + step;
        }
    }
}

void mImagePolygonBorder(MArray *border,int height,int width,int num,...)
{
    mException(INVALID_POINTER(border),EXIT,"invalid input");
    mException((num<3),EXIT,"invalid input");
    
    MList *polygon = mListCreate(DFLT,NULL);
    mListPlace(polygon,NULL,num,sizeof(MImagePoint));

    MImagePoint **point = (MImagePoint **)(polygon->data);
    
    va_list para;va_start(para,num);
    for(int i=0;i<num;i++)
    {
        point[i]->x = (float)va_arg(para,int);
        point[i]->y = (float)va_arg(para,int);
    }
    va_end(para);

    // printf("point[0]->x is %f,point[0]->y is %f\n",point[0]->x,point[0]->y);
    // printf("point[1]->x is %f,point[1]->y is %f\n",point[1]->x,point[1]->y);
    // printf("point[2]->x is %f,point[2]->y is %f\n",point[2]->x,point[2]->y);
    // printf("point[3]->x is %f,point[3]->y is %f\n",point[3]->x,point[3]->y);

    ImagePolygonBorder(border,height,width,polygon);
    mListRelease(polygon);
}

void mImageRectBorder(MArray *border,int height,int width,int x1,int x2,int y1,int y2)
{
    mException(INVALID_POINTER(border),EXIT,"invalid input");
    mArrayRedefine(border,height*2,S16,border->dataS16);
    
    int buff;
    if(x1>x2) {buff=x1;x1=x2;x2=buff;} x1 = MAX(0,x1); x2 = MIN(width ,x2);
    if(y1>y2) {buff=y1;y1=y2;y2=buff;} y1 = MAX(0,y1); y2 = MIN(height,y2);

    char *str;
    str=&(border->info.name[0][0]);if(strlen(str)>0) mException((strcmp(str,"y1"    )!=0),EXIT,"invalid border");else strcpy(str,"y1");
    str=&(border->info.name[1][0]);if(strlen(str)>0) mException((strcmp(str,"y2"    )!=0),EXIT,"invalid border");else strcpy(str,"y2");
    str=&(border->info.name[2][0]);if(strlen(str)>0) mException((strcmp(str,"height")!=0),EXIT,"invalid border");else strcpy(str,"height");
    str=&(border->info.name[3][0]);if(strlen(str)>0) mException((strcmp(str,"width" )!=0),EXIT,"invalid border");else strcpy(str,"width");
    border->info.value[0]=y1;border->info.value[0]=y2;border->info.value[0]=height;border->info.value[0]=width;

    memset(border->dataS16,0,(y1+y1)*sizeof(short));
    for(int i=y1;i<y2;i++)
    {
        border->dataS16[i+i  ] = x1;
        border->dataS16[i+i+1] = x2;
    }
    memset(border->dataS16+y2+y2,0,(height-y2)*2*sizeof(short));
}

void mLine(MList *line,float x1,float y1,float x2,float y2)
{
    mException(INVALID_POINTER(line),EXIT,"invalid input");
    
    mListPlace(line,NULL,2,sizeof(MImagePoint));
    
    MImagePoint **point = (MImagePoint **)(line->data);
    point[0]->x = x1;
    point[0]->y = y1;
    point[1]->x = x2;
    point[1]->y = y2;
}

void mTriangle(MList *triangle,float x1,float y1,float x2,float y2,float x3,float y3)
{
    mException(INVALID_POINTER(triangle),EXIT,"invalid input");
    
    mListPlace(triangle,NULL,3,sizeof(MImagePoint));
    
    MImagePoint **point = (MImagePoint **)(triangle->data);
    point[0]->x = x1;
    point[0]->y = y1;
    point[1]->x = x2;
    point[1]->y = y2;
    point[2]->x = x3;
    point[2]->y = y3;
}

void mQuadrangle(MList *quadrangle,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4)
{
    mException(INVALID_POINTER(quadrangle),EXIT,"invalid input");
    
    mListPlace(quadrangle,NULL,4,sizeof(MImagePoint));
    
    MImagePoint **point = (MImagePoint **)(quadrangle->data);
    point[0]->x = x1;
    point[0]->y = y1;
    point[1]->x = x2;
    point[1]->y = y2;
    point[2]->x = x3;
    point[2]->y = y3;
    point[3]->x = x4;
    point[3]->y = y4;
}

void mLineTravel(MImagePoint *p1,MImagePoint *p2,int stride,void (*func)(MImagePoint *,void *),void *para)
{
    int i;float step;int num;
    float x_min,x_max,y_min,y_max;
    // printf("p1 is %f,%f,p2 is %f,%f\n",p1->x,p1->y,p2->x,p2->y);
    if(ABS(p1->x-p2->x)>ABS(p1->y-p2->y))
    {
        if(p1->x==p2->x) return;
        if(p1->x<p2->x){x_min=p1->x;x_max=p2->x;y_min=p1->y;y_max=p2->y;}
        else           {x_min=p2->x;x_max=p1->x;y_min=p2->y;y_max=p1->y;}
        step = (p1->y-p2->y)/(p1->x-p2->x);step = step*stride;
        num = (int)((x_max-x_min)/stride+0.5);num=MAX(num,1);
        // #pragma omp parallel for
        for(i=0;i<num;i++)
        {
            MImagePoint point; 
            point.x = x_min + i*stride;
            point.y = y_min + i*step;
            func(&point,para);
        }
    }
    else
    {
        if(p1->y==p2->y) return;
        if(p1->y<p2->y){x_min=p1->x;x_max=p2->x;y_min=p1->y;y_max=p2->y;}
        else           {x_min=p2->x;x_max=p1->x;y_min=p2->y;y_max=p1->y;}
        step = (p1->x-p2->x)/(p1->y-p2->y);step = step*stride;
        num = (int)((y_max-y_min)/stride+0.5);num=MAX(num,1);
        // #pragma omp parallel for
        for(i=0;i<num;i++)
        {
            MImagePoint point; 
            point.x = x_min + i*step;
            point.y = y_min + i*stride;
            func(&point,para);
        }
    }
}
void mPolygonSideTravel(MList *polygon,int stride,void (*func)(MImagePoint *,void *),void *para)
{
    int i;
    for(i=0;i<polygon->num-1;i++)
        mLineTravel((MImagePoint *)(polygon->data[i]),(MImagePoint *)(polygon->data[i+1]),stride,func,para);
    mLineTravel((MImagePoint *)(polygon->data[i]),(MImagePoint *)(polygon->data[0]),stride,func,para);
}

void mCurveTravel(MImageCurve *curve,int stride,void (*func)(MImagePoint *,void *),void *para)
{
    int min,max;
    if(curve->type<=0)
    {
        if(curve->v1.x<curve->v2.x) {min=curve->v1.x;max=curve->v2.x;}
        else                        {min=curve->v2.x;max=curve->v1.x;}
        for(int i=min;i<max;i+=stride)
        {
            MImagePoint p1;p1.x=i  ;p1.y=curve->curve(p1.x,curve->para);
            MImagePoint p2;p2.x=i+1;p2.y=curve->curve(p2.x,curve->para);
            mLineTravel(&p1,&p2,stride,func,para);
        }
    }
    else
    {
        if(curve->v1.y<curve->v2.y) {min=curve->v1.y;max=curve->v2.y;}
        else                        {min=curve->v2.y;max=curve->v1.y;}
        for(int i=min;i<max;i+=stride)
        {
            MImagePoint p1;p1.y=i  ;p1.x=curve->curve(p1.y,curve->para);
            MImagePoint p2;p2.y=i+1;p2.x=curve->curve(p2.y,curve->para);
            mLineTravel(&p1,&p2,stride,func,para);
        }
    }
}

// void mCurveToPolyLine(MImageCurve *curve,MList *line)
// {
//     MImage pt;
//     if(curve->type<=0) {for(int i=curve->v1.x;i<=curve->v2.x;i++) {pt.x=i;pt.y=curve->curve(pt.x,curve->para);mListWrite(line,
//     for(int
// }


/*
void mPolygon(MList *polygon,int num,...)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input");
    
    if(num<0)
        num = polygon->num;
    mException((num<1),EXIT,"invalid input");
    
    mListPlace(polygon,NULL,num,sizeof(MImagePoint));

    MImagePoint **point = (MImagePoint **)(polygon->data);

    va_list para;
    va_start(para,num);
    for(int i=0;i<num;i++)
    {
        point[i]->x = (float)va_arg(para,int);
        point[i]->y = (float)va_arg(para,int);
    }
    va_end(para);
}*/

void mPolygon(MList *polygon,int num,...)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input");
    
    if(num<0)
        num = polygon->num;
    mException((num<1),EXIT,"invalid input");
    
    MImagePoint point;int data;
    mListClear(polygon);
    va_list para1;va_list para2;
    va_start(para1,num);va_start(para2,num);
    for(int i=0;i<num;i++)
    {
        point.x = (float)va_arg(para1,double);
        data = (int)va_arg(para2,int);
        if(point.x==0) point.x = (float)data;
        
        point.y = (float)va_arg(para1,double);
        data = (int)va_arg(para2,int);
        if(point.y==0) point.y = (float)data;
        
        mListWrite(polygon,DFLT,&point,sizeof(MImagePoint));
    }
    va_end(para1);va_end(para2);
    
    mListWrite(polygon,DFLT,polygon->data[0],sizeof(MImagePoint));
    polygon->num=polygon->num-1;
}

void mCurve(MImageCurve *curve,float i1,float i2,int type,float (*func)(float,float *),float *para)
{
    curve->type = type;
    curve->curve= func;
    memcpy(curve->para,para,16*sizeof(float));
    printf("i1 is %f,i2 is %f\n",i1,i2);
    if(i2<i1) {int buff=i1;i1=i2;i2=buff;}
    if(type<=0)
    {
        curve->v1.x=i1;curve->v1.y=func(i1,para);
        curve->v2.x=i2;curve->v2.y=func(i2,para);
    }
    else
    {
        curve->v1.y=i1;curve->v1.x=func(i1,para);
        curve->v2.y=i2;curve->v2.x=func(i2,para);
    }
}

float mCurvePoint(MImageCurve *curve,float x)
{
    return (curve->curve)(x,curve->para);
}
        

void mPoissonDiskPoint(MList *list,float r,float x1,float x2,float y1,float y2)
{
    mException(list==NULL,EXIT,"invalid input");
    float grid_size = 0.707106781*r;
    float r2 = r*r;
    
    int gw = ceil((x2-x1)/grid_size);int gh = ceil((y2-y1)/grid_size);
    MImagePoint **grid = (MImagePoint **)mMalloc(gh*sizeof(MImagePoint *));
    for(int j=0;j<gh;j++) {grid[j]=(MImagePoint *)mMalloc(gw*sizeof(MImagePoint));memset(grid[j],0,gw*sizeof(MImagePoint));}

    //printf("grid_size is %f,gw is %d,gh is %d\n",grid_size,gw,gh);
    mListClear(list);

    MImagePoint point;point.x=(x1+x2)/2;point.y=(y1+y2)/2;
    mListWrite(list,DFLT,&point,sizeof(MImagePoint));
    int x=floor(point.x/grid_size);int y=floor(point.y/grid_size);grid[y][x]=point;

    for(int n=0;n<list->num;n++)
    {
        MImagePoint *p = (MImagePoint *)(list->data[n]);
        int num = 0;
        while(1)
        {
            num+=1;if(num>=64) break;
            float d = (float)mRand(r*10000,r*15000)/10000.0f;
            float a = (float)mRand(0,360000)/1000.0f;
            float dx = d*mSin(a);point.x = p->x+dx; if((point.x>=x2)||(point.x<x1)) continue;
            float dy = d*mCos(a);point.y = p->y+dy; if((point.y>=y2)||(point.y<y1)) continue;
            x=floor(point.x/grid_size); y=floor(point.y/grid_size);
            #define DISTANCE(P1,P2) ((P1.x-P2.x)*(P1.x-P2.x)+(P1.y-P2.y)*(P1.y-P2.y))
                                   if((grid[y  ][x  ].x!=0)||(grid[y  ][x  ].y!=0)) {                                      continue;}
            if((y-2>=0)          ) if((grid[y-2][x  ].x!=0)||(grid[y-2][x  ].y!=0)) {if(DISTANCE(grid[y-2][x  ],point)<r2) continue;}
            if((y-1>=0)&&(x-1>=0)) if((grid[y-1][x-1].x!=0)||(grid[y-1][x-1].y!=0)) {if(DISTANCE(grid[y-1][x-1],point)<r2) continue;}
            if((y-1>=0)          ) if((grid[y-1][x  ].x!=0)||(grid[y-1][x  ].y!=0)) {if(DISTANCE(grid[y-1][x  ],point)<r2) continue;}
            if((y-1>=0)&&(x+1<gw)) if((grid[y-1][x+1].x!=0)||(grid[y-1][x+1].y!=0)) {if(DISTANCE(grid[y-1][x+1],point)<r2) continue;}
            if(          (x-2>=0)) if((grid[y  ][x-2].x!=0)||(grid[y  ][x-2].y!=0)) {if(DISTANCE(grid[y  ][x-2],point)<r2) continue;}
            if(          (x-1>=0)) if((grid[y  ][x-1].x!=0)||(grid[y  ][x-1].y!=0)) {if(DISTANCE(grid[y  ][x-1],point)<r2) continue;}
            if(          (x+1<gw)) if((grid[y  ][x+1].x!=0)||(grid[y  ][x+1].y!=0)) {if(DISTANCE(grid[y  ][x+1],point)<r2) continue;}
            if(          (x+2<gw)) if((grid[y  ][x+2].x!=0)||(grid[y  ][x+2].y!=0)) {if(DISTANCE(grid[y  ][x+2],point)<r2) continue;}
            if((y+1<gh)&&(x-1>=0)) if((grid[y+1][x-1].x!=0)||(grid[y+1][x-1].y!=0)) {if(DISTANCE(grid[y+1][x-1],point)<r2) continue;}
            if((y+1<gh)          ) if((grid[y+1][x  ].x!=0)||(grid[y+1][x  ].y!=0)) {if(DISTANCE(grid[y+1][x  ],point)<r2) continue;}
            if((y+1<gh)&&(x+1<gw)) if((grid[y+1][x+1].x!=0)||(grid[y+1][x+1].y!=0)) {if(DISTANCE(grid[y+1][x+1],point)<r2) continue;}
            if((y+2<gh)          ) if((grid[y+2][x  ].x!=0)||(grid[y+2][x  ].y!=0)) {if(DISTANCE(grid[y+2][x  ],point)<r2) continue;}

            num=0;
            mListWrite(list,DFLT,&point,sizeof(MImagePoint)); 
            grid[y][x]=point;
        }
    }
    
    for(int j=0;j<gh;j++) mFree(grid[j]);
    mFree(grid);
}






/*
void mPolygonDeleteVertex(MPolygon *src,MPolygon *dst,int locate)
{
    int i;
    
    mException(INVALID_POINTER(src),"invalid input",EXIT);
    mException((locate >= src->n),"invalid input",EXIT);
    
    if(INVALID_POINTER(dst)||(dst==src))
    {
        src->n = src->n-1;
        
        for(i=locate;i<src->n;i++)
            src->vertex[i] = src->vertex[i+1];
    }
    else
    {
        if(dst->n <src->n-1)
        {
            if(!INVALID_POINTER(dst->vertex))
                mFree(dst->vertex);
            dst->vertex = (MImagePoint *)mMalloc((src->n-1)*sizeof(MImagePoint));
        }
        
        dst->n = src->n -1;
        
        memcpy(dst->vertex,src->vertex,locate*sizeof(MImagePoint));
        memcpy(dst->vertex+locate,src->vertex+locate+1,(dst->n-locate)*sizeof(MImagePoint));
        
    }
    
}
    
void mPolygonAddVertex(MPolygon *src,MPolygon *dst,int locate,int x,int y)
{
    MImagePoint *point;
    
    mException(INVALID_POINTER(src),"invalid input",EXIT);
    mException((locate > src->n),"invalid input",EXIT);
    
    if(dst->n > src->n+1)
        point = dst->vertex;
    else
        point = (MImagePoint *)mMalloc((src->n+1)*sizeof(MImagePoint));
    
    memcpy(point,src->vertex,locate*sizeof(MImagePoint));
    point[locate].x = x;
    point[locate].y = y;
    memcpy(point+locate+1,(src->vertex)+locate,(src->n -locate)*sizeof(MImagePoint));
    
    dst->n = src->n+1;
    if((!INVALID_POINTER(dst->vertex))&&(point != dst->vertex))
        mFree(src->vertex);
    dst->vertex = point;
}
*/