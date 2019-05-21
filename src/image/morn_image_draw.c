#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_image.h"

unsigned char morn_default_color[4] = {128,255,255,255};

void ImageDrawPoint1(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x+0.5);int y=(int)(point->y+0.5);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    for(int cn=0;cn<img->cn;cn++)
        img->data[cn][y][x] = color[cn];
}
void ImageDrawPoint2(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x);int y=(int)(point->y);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    for(int cn=0;cn<img->cn;cn++)
    {
        img->data[cn][y  ][x] = color[cn];img->data[cn][y  ][x+1] = color[cn];
        img->data[cn][y+1][x] = color[cn];img->data[cn][y+1][x+1] = color[cn];
    }
}
void ImageDrawPoint3(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x+0.5);int y=(int)(point->y+0.5);
    // printf("point is %f,%f,xy is %d,%d\n",point->x,point->y,x,y);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
    for(int cn=0;cn<img->cn;cn++)
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
    for(int cn=0;cn<img->cn;cn++)
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
    if(thick==1) ImageDrawPoint1(dst,point,color);\
    if(thick==2) ImageDrawPoint2(dst,point,color);\
    if(thick==3) ImageDrawPoint3(dst,point,color);\
    if(thick==4) ImageDrawPoint4(dst,point,color);\
}
struct DrawPara
{
    MImage *img;
    unsigned char *color;
    int thick;
};
void PointDraw(MImagePoint *point,void *info)
{
    struct DrawPara *para = info;
    int thick=para->thick;unsigned char *color=para->color;MImage *dst=para->img;
    DRAW_POINT;
}
void mImageDrawPoint(MImage *src,MImage *dst,MImagePoint *point,unsigned char *color,int thick)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    if(thick<=0)thick=1;else if(thick>4)thick=4;
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        mImageCopy(src,dst);
    }
    else
        dst = src;
    
    DRAW_POINT;
}

void mImageDrawRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color,int thick)
{
    if(thick<=0)thick=1;else if(thick>4)thick=4;
    if(INVALID_POINTER(color)) color = morn_default_color;
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        mImageCopy(src,dst);
    }
    else
        dst = src;
    MImagePoint point1;point1.x=rect->x1;point1.y=rect->y1;
    MImagePoint point2;point2.x=rect->x2;point2.y=rect->y1;
    MImagePoint point3;point3.x=rect->x2;point3.y=rect->y2;
    MImagePoint point4;point4.x=rect->x1;point4.y=rect->y2;
    struct DrawPara para;para.color=color;para.thick=thick;para.img=dst;
    LineTravel(&point1,&point2,1,PointDraw,&para);
    LineTravel(&point2,&point3,1,PointDraw,&para);
    LineTravel(&point3,&point4,1,PointDraw,&para);
    LineTravel(&point4,&point1,1,PointDraw,&para);
}
void mImageDrawLine(MImage *src,MImage *dst,MImagePoint *p1,MImagePoint *p2,unsigned char *color,int thick)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    if(thick<=0)thick=1;else if(thick>4)thick=4;
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        mImageCopy(src,dst);
    }
    else
        dst = src;
    struct DrawPara para;para.color=color;para.thick=thick;para.img=dst;
    LineTravel(p1,p2,1,PointDraw,&para);
}
void mImageDrawShape(MImage *src,MImage *dst,MList *shape,unsigned char *color,int thick)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    if(thick<=0)thick=1;else if(thick>4)thick=4;
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        mImageCopy(src,dst);
    }
    else
        dst = src;
    struct DrawPara para;para.color=color;para.thick=thick;para.img=dst;
    mPolygonSideTravel(shape,1,PointDraw,&para);
}

void mImageDrawCurve(MImage *src,MImage *dst,float (*func)(float,void *),void *para,unsigned char *color,int thick)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    if(thick<=0)thick=1;else if(thick>4)thick=4;
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        mImageCopy(src,dst);
    }
    else
        dst = src;
    struct DrawPara info;info.color=color;info.thick=thick;info.img=dst;
    for(int i=0;i<src->width;i++)
    {
        MImagePoint p1;p1.x=i  ;p1.y=func(p1.x,para);if((!isnan(p1.y))&&(!isinf(p1.y))) {if((p1.y>=0)&&(p1.y<=src->height)) {
        MImagePoint p2;p2.x=i+1;p2.y=func(p2.x,para);if((!isnan(p2.y))&&(!isinf(p2.y))) {if((p2.y>=0)&&(p2.y<=src->height)) {
        // printf("p1 is %f,%f,p2 is %f,%f\n",p1.x,p1.y,p2.x,p2.y);
        LineTravel(&p1,&p2,1,PointDraw,&info);}}}}
    }
}

float CircleFunc1(float x,void *info){float *para=info;return (para[1]+sqrt(para[2]*para[2]-(x-para[0])*(x-para[0])));}
float CircleFunc2(float x,void *info){float *para=info;return (para[1]-sqrt(para[2]*para[2]-(x-para[0])*(x-para[0])));}
void mImageDrawCircle(MImage *src,MImage *dst,MImagePoint *center,float r,unsigned char *color,int thick)
{
    float para[3]; para[0]=center->x;para[1]=center->y;para[2]=r;
    
    mImageDrawCurve(src,dst,CircleFunc1,para,color,thick);
    mImageDrawCurve(src,dst,CircleFunc2,para,color,thick);
}

void mImageFillRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color)
{
    int j,cn;
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    mException(INVALID_IMAGE(src)||INVALID_POINTER(rect),EXIT,"invalid input");
    
    if((!INVALID_POINTER(dst))&&(dst!=src))
    {
        mImageRedefine(dst,src->cn,src->height,src->width);
        for(j=0;j<src->cn*src->height;j++)
            memcpy(dst->data[0][j],src->data[0][j],src->width*sizeof(unsigned char));
    }
    else
        dst = src;
    
    int thick = rect->x2-rect->x1;
    
    for(cn=0;cn<dst->cn;cn++)
        for(j=rect->y1;j<rect->y2;j++)
            memset(dst->data[cn][j]+rect->x1,color[cn],thick);
}

/*
void mImageDrawLine(MImage *src,MImagePoint *p1,MImagePoint *p2,unsigned char *color)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    int thick=src->width;
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
                if((j>=0)&&(j<height)&&(i>=0)&&(i<thick))
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
                if((j>=0)&&(j<height)&&(i>=0)&&(i<thick))
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
                if((j>=0)&&(j<height)&&(i>=0)&&(i<thick))
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
                if((j>=0)&&(j<height)&&(i>=0)&&(i<thick))
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

