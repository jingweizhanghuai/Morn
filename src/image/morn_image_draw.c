/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_image.h"

static unsigned char morn_default_color[4] = {128,255,0,255};

void ImageDrawPoint1(MImage *img,MImagePoint *point,unsigned char *color)
{
    int x=(int)(point->x+0.5);int y=(int)(point->y+0.5);
    if((x<0)||(x>=img->width)||(y<0)||(y>img->height)) return;
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
    else if(width==2) ImageDrawPoint2(dst,point,color);\
    else if(width==3) ImageDrawPoint3(dst,point,color);\
    else if(width==4) ImageDrawPoint4(dst,point,color);\
}
void m_ImageDrawPoint(MImage *src,MImage *dst,MImagePoint *point,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    mException((width<-1)||(width>4),EXIT,"invalid input");if(width<=0)width=1;
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
    struct DrawPara *para = (struct DrawPara *)info;
    MImage *dst = para->dst;
    unsigned char *color = para->color;
    int width = para->width;
    DRAW_POINT;
}
void m_ImageDrawRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    mException((width<-1)||(width>4),EXIT,"invalid input");if(width<=0)width=1;
    MImagePoint point1;point1.x=rect->x1  ;point1.y=rect->y1  ;
    MImagePoint point2;point2.x=rect->x2-1;point2.y=rect->y1  ;
    MImagePoint point3;point3.x=rect->x2-1;point3.y=rect->y2-1;
    MImagePoint point4;point4.x=rect->x1  ;point4.y=rect->y2-1;
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    mLineTravel(&point1,&point2,1,PointDraw,&para);
    mLineTravel(&point2,&point3,1,PointDraw,&para);
    mLineTravel(&point3,&point4,1,PointDraw,&para);
    mLineTravel(&point4,&point1,1,PointDraw,&para);
    PointDraw(&point3,&para);
}
void m_ImageDrawLine(MImage *src,MImage *dst,MImagePoint *p1,MImagePoint *p2,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    mException((width<-1)||(width>4),EXIT,"invalid input");if(width<=0)width=1;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
       
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    mLineTravel(p1,p2,1,PointDraw,&para);
}
void m_ImageDrawShape(MImage *src,MImage *dst,MList *shape,unsigned char *color,int width)
{
    mException(shape==NULL,EXIT,"invalid shape");
    if(shape->num==0) return;
    
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    mException((width<-1)||(width>4),EXIT,"invalid input");if(width<=0)width=1;
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    struct DrawPara para;para.dst=dst;para.color=color;para.width=width;
    mPolygonSideTravel(shape,1,PointDraw,&para);
}

void m_ImageDrawCircle(MImage *src,MImage *dst,MImageCircle *circle,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    
    mException((width<-1)||(width>4),EXIT,"invalid input");if(width<=0)width=1;
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
        
        mPoint(&p1,x-1.0f,cy+y0);
        mPoint(&p2,x     ,cy+y );
        mLineTravel(&p1,&p2,1,PointDraw,&para);
        
        mPoint(&p1,x-1.0f,cy-y0);
        mPoint(&p2,x     ,cy-y );
        mLineTravel(&p1,&p2,1,PointDraw,&para);
        
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

void m_ImageDrawCurve(MImage *src,MImage *dst,MImageCurve *curve,unsigned char *color,int width)
{
    if(INVALID_POINTER(color)) color = morn_default_color;
    mException((width<-1)||(width>4),EXIT,"invalid input");if(width<=0)width=1;
    
    if((!INVALID_POINTER(dst))&&(dst!=src)) mImageCopy(src,dst);
    else dst = src;
    
    struct DrawPara draw_para;draw_para.dst=dst;draw_para.color=color;draw_para.width=width;
    mCurveTravel(curve,1,PointDraw,&draw_para);
    // if(curve->type<=0)
    // {
    //     for(int i=MAX(curve->v1.x,0);i<MIN(curve->v2.x,src->width);i++)
    //     {
    //         MImagePoint p1;p1.x=i  ;p1.y=curve->curve(p1.x,curve->para);if((p1.y<0)||(p1.y>src->height)) continue;
    //         MImagePoint p2;p2.x=i+1;p2.y=curve->curve(p2.x,curve->para);if((p2.y<0)||(p2.y>src->height)) continue;
    //         // printf("x is %f,y is %f\n",p1.x,p1.y);
    //         mLineTravel(&p1,&p2,1,PointDraw,&draw_para);
    //     }
    // }
    // else
    // {
    //     for(int i=MAX(curve->v1.y,0);i<MIN(curve->v2.y,src->width);i++)
    //     {
    //         MImagePoint p1;p1.y=i  ;p1.x=curve->curve(p1.y,curve->para);if((p1.x<0)||(p1.x>src->width)) continue;
    //         MImagePoint p2;p2.y=i+1;p2.x=curve->curve(p2.y,curve->para);if((p2.x<0)||(p2.x>src->width)) continue;
    //         printf("x is %f,y is %f\n",p1.x,p1.y);
    //         mLineTravel(&p1,&p2,1,PointDraw,&draw_para);
    //     }
    // }
}

void ImageFillShape(MImage *img,int x0,int y0)
{
    if((x0<0)||(x0>=img->width)) return;
    unsigned char **data = img->data[0];
    for(int i=y0;i>=0;i--) 
    {
        if(data[i][x0]!=0) break;
        data[i][x0]=255;
        if(data[i][x0-1]==0) ImageFillShape(img,x0-1,i);
        if(data[i][x0+1]==0) ImageFillShape(img,x0+1,i);
    }
    for(int i=y0+1;i<img->height;i++) 
    {
        if(data[i][x0]!=0) break;
        data[i][x0]=255;
        if(data[i][x0-1]==0) ImageFillShape(img,x0-1,i);
        if(data[i][x0+1]==0) ImageFillShape(img,x0+1,i);
    }
}


// void ImageFillShape(MImage *img,int x0,int y0)
// {
//     unsigned char **data = img->data[0];
//     for(int i=x0;i>=0;i--)
//     {
//         if(data[y0][i]!=0) break;
//         data[y0][i]=255;
//     }
//     for(int i=x0+1;i<img->width;i++) 
//     {
//         if(data[y0][i]!=0) break;
//         data[y0][i]=255;
//     }
//     if(y0-1>=0)
//     {
//         for(int i=x0  ;i>=0        ;i--) {if(data[y0][i]!=255) {break;} if(data[y0-1][i]==0) ImageFillShape(img,i,y0-1);}
//         for(int i=x0+1;i<img->width;i++) {if(data[y0][i]!=255) {break;} if(data[y0-1][i]==0) ImageFillShape(img,i,y0-1);}
//     }
//     if(y0+1< img->height)
//     {
//         for(int i=x0  ;i>=0        ;i--) {if(data[y0][i]!=255) {break;} if(data[y0+1][i]==0) ImageFillShape(img,i,y0+1);}
//         for(int i=x0+1;i<img->width;i++) {if(data[y0][i]!=255) {break;} if(data[y0+1][i]==0) ImageFillShape(img,i,y0+1);}
//     }
// }