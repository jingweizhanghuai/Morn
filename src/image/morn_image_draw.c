#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_Image.h"

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

/*
void mImageDrawRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color)
{
    int j,cn;
    
    mException(INVALID_IMAGE(src)||INVALID_POINTER(rect),EXIT,"invalid input");
    
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        for(j=0;j<src->cn*src->height;j++)
            memcpy(dst->data[0][j],src->data[0][j],src->width*sizeof(unsigned char));
    }
    else
        dst = src;
    
    int width = rect->x2-rect->x1;
    
    for(cn=0;cn<dst->cn;cn++)
    {
        memset(dst->data[cn][rect->y1]+rect->x1,color[cn],width);
    
        for(j=rect->y1;j<rect->y2;j++)
        {
            dst->data[cn][j][rect->x1] = color[cn];
            dst->data[cn][j][rect->x2] = color[cn];
        }
        memset(dst->data[cn][rect->y2]+rect->x1,color[cn],width);
    }
}
*/
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

/*
void mImageDrawLine(MImage *src,MImagePoint *p1,MImagePoint *p2,unsigned char *color)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    int width=src->width;
    int height=src->height;
    
    if(ABS(p1->x-p2->x)>ABS(p1->y-p2->y))
    {
        if((p1->x)<(p2->x))
        {
            float l = p1->y;
            float step = (p2->y - p1->y)/(p2->x - p1->x);
            for(int i = p1->x;i<p2->x;i++)
            {
                int j = (int)(l+0.5);
                if((j>=0)&&(j<height)&&(i>=0)&&(i<width))
                    for(int cn=0;cn<src->cn;cn++)
                        src->data[cn][j][i] = color[cn];
                l = l+step;
            }
        }
        else
        {
            float l = p2->y;
            float step = (p2->y - p1->y)/(p2->x - p1->x);
            for(int i = p2->x;i<p1->x;i++)
            {
                int j = (int)(l+0.5);
                if((j>=0)&&(j<height)&&(i>=0)&&(i<width))
                    for(int cn=0;cn<src->cn;cn++)
                        src->data[cn][j][i] = color[cn];
                l = l+step;
            }
        }
    }
    else
    {
        if((p1->y)<(p2->y))
        {
            float l = p1->x;
            float  step = (p2->x - p1->x)/(p2->y - p1->y);
            for(int j = p1->y;j<p2->y;j++)
            {
                int i = (int)(l+0.5);
                if((j>=0)&&(j<height)&&(i>=0)&&(i<width))
                    for(int cn=0;cn<src->cn;cn++)
                        src->data[cn][j][i] = color[cn];
                l = l+step;
            }
        }
        else
        {
            float l = p2->x;
            float step = (p2->x - p1->x)/(p2->y - p1->y);
            for(int j = p2->y;j<p1->y;j++)
            {
                int i = (int)(l+0.5);
                if((j>=0)&&(j<height)&&(i>=0)&&(i<width))
                    for(int cn=0;cn<src->cn;cn++)
                        src->data[cn][j][i] = color[cn];
                l = l+step;
            }
        }
    }
}


void mImageDrawShape(MImage *src,MList *shape,unsigned char *color)
{
    mException((INVALID_IMAGE(src))||(INVALID_POINTER(shape)),EXIT,"invalid input");
    mException((shape->num<2),EXIT,"invalid shape");
    
    MImagePoint **point = (MImagePoint **)(shape->data);
    
    mImageDrawLine(src,point[shape->num-1],point[0],color);
    
    if(shape->num == 2)
        return;
    
    for(int i=0;i<shape->num-1;i++)
        mImageDrawLine(src,point[i],point[i+1],color);
}
*/

// void mImageFill

