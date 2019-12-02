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

void mImagePolygonBorder(MList *polygon,MImageBorder *border)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input polygon");
    mException((polygon->num<3),EXIT,"invalid input polygon");
    mException(INVALID_POINTER(border),EXIT,"invalid input");
    mException((border->height<=0)||(border->width<=0),EXIT,"invalid border size");
    
    MImagePoint **point = (MImagePoint **)(polygon->data);
    
    border->y1 = (int)(point[0]->y+0.5f);
    border->y2 = (int)(point[0]->y+0.5f);
    for(int n=1;n<polygon->num;n++)
    {
        border->y1 = MIN(border->y1,(int)(point[n]->y+0.5f));
        border->y2 = MAX(border->y2,(int)(point[n]->y+0.5f));
    }
    border->y1 = MAX(border->y1,0);
    border->y2 = MIN(border->y2+1,border->height);
    
    memset(border->x1+border->y1,0,(border->y2-border->y1)*sizeof(short));
    memset(border->x2+border->y1,0,(border->y2-border->y1)*sizeof(short));
    
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
            if((ly>=0)&&(ly<border->height))
            {
                if(p1->x<p2->x)
                {
                    border->x1[ly] = p1->x;
                    border->x2[ly] = p2->x;
                }
                else
                {
                    border->x1[ly] = p2->x;
                    border->x2[ly] = p1->x;
                }
            }
            continue;
        }

        float x_locate = (p1->x);
        float step = (p2->x - p1->x)/(p2->y - p1->y);
        step = (p1->y>p2->y)?(0.0f-step):step;
        
        for(ly=(int)(p1->y+0.5f);;ly=((p1->y>p2->y)?(ly-1):(ly+1)))
        {
            lx = (float)x_locate;
            
            if(lx<0)                  lx=0;
            else if(lx>border->width) lx = border->width;
            
            if((ly>=0)&&(ly<border->height))
            {
                if(border->x1[ly] == 0)
                    border->x1[ly] = lx;
                else if(lx<border->x1[ly])
                {
                    border->x2[ly] = border->x1[ly];
                    border->x1[ly] = lx;
                }
                else
                    border->x2[ly] = lx;
            }
            
            if(ly==(int)(p2->y+0.5f))
                break;
            
            x_locate = x_locate + step;
        }
    }
}

void mImageRectBorder(MImageBorder *border,int x1,int x2,int y1,int y2)
{
    mException(INVALID_POINTER(border),EXIT,"invalid input");
    mException((border->height<=0)||(border->width<=0),EXIT,"invalid border size");
    
    int buff;
    if(x1>x2) {buff=x1;x1=x2;x2=buff;}
    if(y1>y2) {buff=y1;y1=y2;y2=buff;}

    mException((x1<0)||(x2>=border->width)||(y1<0)||(y2>=border->height),EXIT,"invalid input");

    x1 = MAX(0,x1);
    x2 = MIN(border->width,x2);
    y1 = MAX(0,y1);
    y2 = MIN(border->height,y2);
    
    border->y1 = y1;
    border->y2 = y2;
    
    for(int i=border->y1;i<border->y2;i++)
    {
        border->x1[i] = x1;
        border->x2[i] = x2;
    }
}

MImageBorder *mImageBorderCreate(int height,int width,int num,...)
{
    MImageBorder *border;
    
    border = (MImageBorder *)mMalloc(sizeof(MImageBorder));
    border->height = height;
    border->width = width;
    border->x1 = (short *)mMalloc(sizeof(short)*height);
    border->x2 = (short *)mMalloc(sizeof(short)*height);
    
    if(num<=0)
    {
        border->y1=0;
        border->y2=0;
        memset(border->x1,0,sizeof(short)*height);
        memset(border->x2,0,sizeof(short)*height);
        return border;
    }

    mException((num<3),EXIT,"invalid input");

    MList *polygon = mListCreate(num,NULL);
    mListPlace(polygon,num,sizeof(MImagePoint));
    MImagePoint **point = (MImagePoint **)(polygon->data);

    va_list para;
    va_start(para,num);
    for(int i=0;i<num;i++)
    {
        point[i]->x = (float)va_arg(para,int);
        point[i]->y = (float)va_arg(para,int);
        mException((point[i]->x<0)||(point[i]->x>=width)||(point[i]->y<0)||(point[i]->y>=height),EXIT,"invalid input");
    }
    va_end(para);
    mImagePolygonBorder(polygon,border);
    mListRelease(polygon);

    return border;
}


void mImageBorderRelease(MImageBorder *border)
{
    mFree(border->x1);
    mFree(border->x2);
    
    mFree(border);
}

MImagePoint *mImagePointCreate(float x,float y)
{
    MImagePoint *point;
    point = (MImagePoint *)mMalloc(sizeof(MImagePoint));
    point->x = x;
    point->y = y;
    return point;
}

MImageRect *mImageRectCreate(int x1,int y1,int x2,int y2)
{
    MImageRect *rect;
    rect = (MImageRect *)mMalloc(sizeof(MImageRect));
    rect->x1 = x1;
    rect->x2 = x2;
    rect->y1 = y1;
    rect->y2 = y2;
    return rect;
}

void mLineSetup(MList *line,float x1,float y1,float x2,float y2)
{
    mException(INVALID_POINTER(line),EXIT,"invalid input");
    
    mListPlace(line,2,sizeof(MImagePoint));
    
    MImagePoint **point = (MImagePoint **)(line->data);
    point[0]->x = x1;
    point[0]->y = y1;
    point[1]->x = x2;
    point[1]->y = y2;
}

void mTriangleSet(MList *triangle,float x1,float y1,float x2,float y2,float x3,float y3)
{
    mException(INVALID_POINTER(triangle),EXIT,"invalid input");
    
    mListPlace(triangle,3,sizeof(MImagePoint));
    
    MImagePoint **point = (MImagePoint **)(triangle->data);
    point[0]->x = x1;
    point[0]->y = y1;
    point[1]->x = x2;
    point[1]->y = y2;
    point[2]->x = x3;
    point[2]->y = y3;
}

void mQuadrangleSet(MList *quadrangle,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4)
{
    mException(INVALID_POINTER(quadrangle),EXIT,"invalid input");
    
    mListPlace(quadrangle,4,sizeof(MImagePoint));
    
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

void mShapeSetup(MList *polygon,int num,...)
{
    mException(INVALID_POINTER(polygon),EXIT,"invalid input");
    
    if(num<0)
        num = polygon->num;
    mException((num<1),EXIT,"invalid input");
    
    mListPlace(polygon,num,sizeof(MImagePoint));

    MImagePoint **point = (MImagePoint **)(polygon->data);

    va_list para;
    va_start(para,num);
    for(int i=0;i<num;i++)
    {
        point[i]->x = (float)va_arg(para,double);
        point[i]->y = (float)va_arg(para,double);
    }
    va_end(para);
}

void mPoissonDiskPoint(MList *list,float r,float x1,float x2,float y1,float y2)
{
    mException(list==NULL,EXIT,"invalid input");
    float grid_size = 0.707106781*r;
    float r2 = r*r;
    
    int gw = ceil((x2-x1)/grid_size);int gh = ceil((y2-y1)/grid_size);
    MImagePoint **grid = mMalloc(gh*sizeof(MImagePoint *));
    for(int j=0;j<gh;j++) {grid[j]=mMalloc(gw*sizeof(MImagePoint));memset(grid[j],0,gw*sizeof(MImagePoint));}

    //printf("grid_size is %f,gw is %d,gh is %d\n",grid_size,gw,gh);
    mListClear(list);

    MImagePoint point;point.x=(x1+x2)/2;point.y=(y1+y2)/2;
    mListWrite(list,DFLT,&point,sizeof(MImagePoint));
    int x=floor(point.x/grid_size);int y=floor(point.y/grid_size);grid[y][x]=point;

    for(int n=0;n<list->num;n++)
    {
        MImagePoint *p = list->data[n];
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
MImageLine *mImageLineCreate(int x1,int y1,int x2,int y2)
{
    MImageLine *line;
    line = (MImageLine *)mMalloc(sizeof(MImageLine));
    line->handle = NULL;
    mLineSetup(line,x1,y1,x2,y2);
    return line;
}

void mImageLineRelease(MImageLine *line)
{
    if(!INVALID_POINTER(line->handle))
        HandleRelease(line->handle);
    
    mFree(line);
}



void mPolygonSetup(MPolygon *polygon,int num,int x1,int y1,...)
{
    int i;
    va_list para;
    
    mException((num<3),"invalid input",EXIT);
    
    if((polygon->n<num)&&(polygon->vertex == NULL))
        mFree(polygon->vertex);
    
    polygon->n = num;
    
    polygon->vertex = (MImagePoint *)mMalloc(num*sizeof(MImagePoint));
    polygon->vertex[0].x = x1;
    polygon->vertex[0].y = y1;
    
    va_start(para,y1);
    for(i=1;i<num;i++)
    {
        polygon->vertex[0].x = va_arg(para,int);
        polygon->vertex[0].y = va_arg(para,int);
    }
    va_end(para);
}    



void mPolygonRelease(MPolygon *polygon)
{
    mException(INVALID_POINTER(polygon),"invalid input",EXIT);
    
    if(!INVALID_POINTER(polygon->vertex))
        mFree(polygon->vertex);
    
    mFree(polygon);
}

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