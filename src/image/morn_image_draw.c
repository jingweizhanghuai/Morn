/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_image.h"

unsigned char morn_default_color[3] = {128,255,0};

void ImageDrawPoint1(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x+0.5);int y=(int)(point->y+0.5);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    // printf("x is %d,y is %d\n",x,y);
    for(int cn=0;cn<img->channel;cn++)
        img->data[cn][y][x] = color[cn];
}
void ImageDrawPoint2(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x);int y=(int)(point->y);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    for(int cn=0;cn<img->channel;cn++)
    {
        img->data[cn][y  ][x] = color[cn];img->data[cn][y  ][x+1] = color[cn];
        img->data[cn][y+1][x] = color[cn];img->data[cn][y+1][x+1] = color[cn];
    }
}
void ImageDrawPoint3(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x+0.5);int y=(int)(point->y+0.5);
    
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    for(int cn=0;cn<img->channel;cn++)
    {
        img->data[cn][y-1][x] = color[cn];img->data[cn][y][x-1] = color[cn];
        img->data[cn][y  ][x] = color[cn];
        img->data[cn][y][x+1] = color[cn];img->data[cn][y+1][x] = color[cn];
    }
}
void ImageDrawPoint4(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x);int y=(int)(point->y);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    for(int cn=0;cn<img->channel;cn++)
    {
        img->data[cn][y-1][x  ] = color[cn];img->data[cn][y-1][x+1] = color[cn];
        img->data[cn][y  ][x-1] = color[cn];img->data[cn][y  ][x  ] = color[cn];
        img->data[cn][y  ][x+1] = color[cn];img->data[cn][y  ][x+2] = color[cn];
        img->data[cn][y+1][x-1] = color[cn];img->data[cn][y+1][x  ] = color[cn];
        img->data[cn][y+1][x+1] = color[cn];img->data[cn][y+1][x+2] = color[cn];
        img->data[cn][y+2][x  ] = color[cn];img->data[cn][y+2][x+1] = color[cn];
    }
}
#define DRAW_POINT {\
    if(width==1) ImageDrawPoint1(dst,point,color);\
    if(width==2) ImageDrawPoint2(dst,point,color);\
    if(width==3) ImageDrawPoint3(dst,point,color);\
    if(width==4) ImageDrawPoint4(dst,point,color);\
}
void mImageDrawPoint(MImage *src,MImage *dst,MImagePoint *point,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    if(width<=0)width=1;else if(width>4)width=4;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    DRAW_POINT;
}

struct DrawPara
{
    MImage *dst;
    unsigned char *color;
    int width;
};
void PointDraw(MImagePoint *point, void *info)
{
    struct DrawPara *para = info;
    MImage *dst = para->dst;
    unsigned char *color = para->color;
    int width = para->width;
    DRAW_POINT;
}
void mImageDrawRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
      
    MImagePoint point1;point1.x=rect->x1  ;point1.y=rect->y1  ;
    MImagePoint point2;point2.x=rect->x2-1;point2.y=rect->y1  ;
    MImagePoint point3;point3.x=rect->x2-1;point3.y=rect->y2-1;
    MImagePoint point4;point4.x=rect->x1  ;point4.y=rect->y2-1;
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    LineTravel(&point1,&point2,1,PointDraw,&para);
    LineTravel(&point2,&point3,1,PointDraw,&para);
    LineTravel(&point3,&point4,1,PointDraw,&para);
    LineTravel(&point4,&point1,1,PointDraw,&para);
    PointDraw(&point3,&para);
}
void mImageDrawLine(MImage *src,MImage *dst,MImagePoint *p1,MImagePoint *p2,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    if(width<=0)width=1;else if(width>4)width=4;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
       
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    LineTravel(p1,p2,1,PointDraw,&para);
}
void mImageDrawShape(MImage *src,MImage *dst,MList *shape,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    if(width<=0)width=1;else if(width>4)width=4;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    mPolygonSideTravel(shape,1,PointDraw,&para);
}

void mImageDrawCircle(MImage *src,MImage *dst,MImageCircle *circle,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    if(width<=0)width=1;else if(width>4)width=4;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    float cx = circle->center.x;float cy = circle->center.y;float r=circle->r;
    MImagePoint p1,p2;
    float y0=0.0f;
    for(float x=cx-r+1.0f;x<cx+r+1.0f;x+=1.0f)
    {
        float y;
        if(x>cx+r) y=0.0f;
        else y=sqrt(r*r-(x-cx)*(x-cx));
        
        mImagePointSetup(&p1,x-1.0f,cy+y0);
        mImagePointSetup(&p2,x     ,cy+y );
        LineTravel(&p1,&p2,1,PointDraw,&para);
        
        mImagePointSetup(&p1,x-1.0f,cy-y0);
        mImagePointSetup(&p2,x     ,cy-y );
        LineTravel(&p1,&p2,1,PointDraw,&para);
        
        y0=y;
    }
}

void mImageFillRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color)
{
    int j,cn;
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    mException(INVALID_IMAGE(src)||INVALID_POINTER(rect),EXIT,"invalid input");
    
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    int width = rect->x2-rect->x1;
    
    for(cn=0;cn<dst->channel;cn++)
        for(j=rect->y1;j<rect->y2;j++)
            memset(dst->data[cn][j]+rect->x1,color[cn],width);
}

void mImageDrawCurve(MImage *src,MImage *dst,float (*func)(float,void *),void *para,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    if(width<=0)width=1;else if(width>4)width=4;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    struct DrawPara draw_para;draw_para.dst=dst;draw_para.color=color;draw_para.width=width;
    for(int i=0;i<src->width;i++)
    {
        MImagePoint p1;p1.x=i  ;p1.y=func(p1.x,para);if((!isnan(p1.y))&&(!isinf(p1.y))) {if((p1.y>=0)&&(p1.y<=src->height)) {
        MImagePoint p2;p2.x=i+1;p2.y=func(p2.x,para);if((!isnan(p2.y))&&(!isinf(p2.y))) {if((p2.y>=0)&&(p2.y<=src->height)) {
        LineTravel(&p1,&p2,1,PointDraw,&draw_para);}}}}
    }
}
