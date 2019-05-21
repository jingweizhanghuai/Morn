#ifndef _MORN_IMAGE_H_
#define _MORN_IMAGE_H_

#include "morn_Math.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_MAX_IMAGE_CN 4

#define MORN_IMAGE_GRAY 1
#define MORN_IMAGE_RGB  3
#define MORN_IMAGE_RGBA 4
#define MORN_IMAGE_YUV  5
#define MORN_IMAGE_HSV  6

#define MORN_BORDER_UNDEFINED DFLT
#define MORN_BORDER_BLACK     0
#define MORN_BORDER_WHITE     1
#define MORN_BORDER_REPLICATE 2
#define MORN_BORDER_REFLECT   3
#define MORN_BORDER_IMAGE     4
#define MORN_BORDER_INVALID   5

typedef struct MImageBorder
{
    short height;
    short width;
    short y1;
    short y2;
    short *x1;
    short *x2;
}MImageBorder;

typedef struct MImage {
    int cn;                         // ?????
    int height;
    int width;
    
    unsigned char **data[MORN_MAX_IMAGE_CN];
    
    MList *handle;
    
    MImageBorder *border;
    
    // struct
    // {
        // int image_type;
        // int border_type;
    // }info;
    MInfo info;
    void *reserve;
}MImage;

#define INVALID_IMAGE(Img) ((((Img) ==NULL)||((intptr_t)(Img) == DFLT))?1:(((Img)->data == NULL)||((intptr_t)((Img)->data) == DFLT)\
                                                                    ||((Img)->cn <= 0)\
                                                                    ||((Img)->height <= 0)||((Img)->width <= 0)\
                                                                    ||((Img)->handle == NULL)))

MImage *mImageCreate(int cn,int height,int width,unsigned char **data[]);
void mImageRedefine(MImage *img,int cn,int height,int width);
void mImageRelease(MImage *img);
#define mImageSet(Img,Cn,Height,Width) {\
    if(Img == NULL)\
        Img = mImageCreate(Cn,Height,Width,NULL);\
    else\
        mImageRedefine(Img,Cn,Height,Width);\
}

void mImageExpand(MImage *img,int r,int border_type);
void mImageCopy(MImage *src,MImage *dst);

void mImageDiff(MImage *src1,MImage *src2,MImage *diff);
void mImageAdd(MImage *src1,MImage *src2,MImage *dst);
void mImageSub(MImage *src1,MImage *src2,MImage *dst);
void mImageInvert(MImage *src,MImage *dst);

#define MORN_IMAGE_STREAM_HWC DFLT
#define MORN_IMAGE_STREAM_HCW 1
#define MORN_IMAGE_STREAM_CHW 2
#define MORN_IMAGE_STREAM_CWH 3
#define MORN_IMAGE_STREAM_WHC 4
#define MORN_IMAGE_STREAM_WCH 5

void mImageDataInputU8( MImage *img,U8  *stream,int stream_type,unsigned char (*func)(U8 ,void *),void *para);
void mImageDataInputS8( MImage *img,S8  *stream,int stream_type,unsigned char (*func)(S8 ,void *),void *para);
void mImageDataInputU16(MImage *img,U16 *stream,int stream_type,unsigned char (*func)(U16,void *),void *para);
void mImageDataInputS16(MImage *img,S16 *stream,int stream_type,unsigned char (*func)(S16,void *),void *para);
void mImageDataInputS32(MImage *img,S32 *stream,int stream_type,unsigned char (*func)(S32,void *),void *para);
void mImageDataInputF32(MImage *img,F32 *stream,int stream_type,unsigned char (*func)(F32,void *),void *para);
void mImageDataInputD64(MImage *img,D64 *stream,int stream_type,unsigned char (*func)(D64,void *),void *para);
#define mImageDataInput(Type,Img,Stream,Stream_type,Func,Para) mImageDataInput##Type(Img,Stream,Stream_type,Func,Para)

void mImageDataOutputU8( MImage *img,U8  *stream,int stream_type,unsigned char (*func)(U8 ,void *),void *para);
void mImageDataOutputS8( MImage *img,S8  *stream,int stream_type,unsigned char (*func)(S8 ,void *),void *para);
void mImageDataOutputU16(MImage *img,U16 *stream,int stream_type,unsigned char (*func)(U16,void *),void *para);
void mImageDataOutputS16(MImage *img,S16 *stream,int stream_type,unsigned char (*func)(S16,void *),void *para);
void mImageDataOutputS32(MImage *img,S32 *stream,int stream_type,unsigned char (*func)(S32,void *),void *para);
void mImageDataOutputF32(MImage *img,F32 *stream,int stream_type,unsigned char (*func)(F32,void *),void *para);
void mImageDataOutputD64(MImage *img,D64 *stream,int stream_type,unsigned char (*func)(D64,void *),void *para);
#define mImageDataOutput(Type,Img,Stream,Stream_type,Func,Para) mImageDataOutput##Type(Img,Stream,Stream_type,Func,Para)

void mImageToGray(MImage *src,MImage *dst); 

void mImageCoordinateTransform(MImage *src,MImage *dst,float (*x_func)(int,int,void *),float (*y_func)(int,int,void *),void *para);

MImageBorder *mImageBorderCreate(int height,int width,int num,...);
void mImageBorderRelease(MImageBorder *border);
#define ImageY1(Img) (((Img)->border==NULL)?0:(((Img)->border)->y1))
#define ImageY2(Img) (((Img)->border==NULL)?((Img)->height):(((Img)->border)->y2))
#define ImageX1(Img,n) (((Img)->border==NULL)?0:(((Img)->border)->x1[n]))
#define ImageX2(Img,n) (((Img)->border==NULL)?((Img)->width):(((Img)->border)->x2[n]))


#define mImageExchange(Src,Dst) mObjectExchange(Src,Dst,MImage)
#define mImageReset(Img) mHandleReset(Img->handle)

typedef struct MImageRect
{
    int x1;
    int y1;
    int x2;
    int y2;
}MImageRect;
#define mRect(Rect,X1,Y1,X2,Y2) {(Rect)->x1=X1;(Rect)->y1=Y1;(Rect)->x2=X2;(Rect)->y2=Y2;}
#define mRectHeight(Rect) ((Rect)->y2-(Rect)->y1)
#define mRectWidth(Rect)  ((Rect)->x2-(Rect)->x1)

typedef struct MImagePoint
{
    float x;
    float y;
}MImagePoint;
#define mPoint(Point,X,Y) {(Point)->x=X; (Point)->y=Y;}
MImagePoint *mImagePointCreate(float x1,float y1);

void mBMPSave(MImage *src,const char *filename);
void mBMPLoad(const char *filename,MImage *dst);
void mJPGSave(MImage *src,const char *filename);
void mJPGLoad(const char *filename,MImage *dst);
void mPNGSave(MImage *src,const char *filename);
void mPNGLoad(const char *filename,MImage *dst);
void mImageLoad(const char *filename,MImage *img);
void mImageSave(MImage *img,const char *filename);

void mImageRotate(MImage *src,MImage *dst,MImagePoint *src_hold,MImagePoint *dst_hold,float angle);

#define MORN_RESIZE_UNUNIFORM  0
#define MORN_RESIZE_MINUNIFORM 1
#define MORN_RESIZE_MAXUNIFORM 2
#define MORN_RESIZE_LINEAR     0x00
#define MORN_RESIZE_NEAREST    0x10
void mImageResize(MImage *src,MImage *dst,int height,int width,int type);

extern unsigned char morn_default_color[4];
void mImageDrawPoint(MImage *src,MImage *dst,MImagePoint *point,unsigned char *color,int width);
void mImageDrawRect(MImage *src,MImage *dst,MImageRect *rect,unsigned char *color,int width);
void mImageDrawLine(MImage *src,MImage *dst,MImagePoint *p1,MImagePoint *p2,unsigned char *color,int width);
void mImageDrawShape(MImage *src,MImage *dst,MList *shape,unsigned char *color,int width);
void mImageDrawCircle(MImage *src,MImage *dst,MImagePoint *center,float r,unsigned char *color,int thick);
void mImageDrawCurve(MImage *src,MImage *dst,float (*func)(float,void *),void *para,unsigned char *color,int thick);

void LineTravel(MImagePoint *p1,MImagePoint *p2,int stride,void (*func)(MImagePoint *,void *),void *para);
void mLineTravel(MList *line,int stride,void (*func)(MImagePoint *,void *),void *para);
void mPolygonSideTravel(MList *polygon,int stride,void (*func)(MImagePoint *,void *),void *para);

#define PointLineCheck(px,py,lx1,ly1,lx2,ly2) (((lx1)-(lx2))*((py)-(ly2))-((ly1)-(ly2))*((px)-(lx2)))
int mLinePointCheck(MList *line,MImagePoint *point);
int LineCrossCheck(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2);
int mLineCrossCheck(MList *line1,MList *line2);
int LineCrossPoint(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2,float *px,float *py);
void mPolygon(MList *polygon,int num,...);
float mPolygonArea(MList *polygon);
float TriangleArea(float x1,float y1,float x2,float y2,float x3,float y3);
float mTriangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3);
float QuadrangleArea(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4);
float mQuadrangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3,MImagePoint *p4);
#define PointDistance(X1,Y1,X2,Y2) ((float)sqrt(((X1)-(X2))*((X1)-(X2))+((Y1)-(Y2))*((Y1)-(Y2))))
#define mPointDistance(P1,P2) PointDistance((P1)->x,(P1)->y,(P2)->x,(P2)->y)
double PointVerticalDistance(double px,double py,double lx1,double ly1,double lx2,double ly2,float *vx,float *vy);
float mPointVerticalDistance(MImagePoint *point,MList *line,MImagePoint *pedal);
#define mPointInRect(Point,Rect) (((Point)->x>(Rect)->x1)&&((Point)->x<(Rect)->x2)&&((Point)->y>(Rect)->y1)&&((Point)->y<(Rect)->y2));
int PointInPolygon(double x,double y,MList *polygon);
int mPointInPolygon(MImagePoint *point,MList *polygon);
float mPolygonIntersetArea(MList *polygon1,MList *polygon2);
int PolygonCross(MList *polygon1,MList *polygon2);
int mPolygonCross(MList *polygon1,MList *polygon2);
int mPolygonConcaveCheck(MList *polygon);
void mShapeBounding(MList *shape,MList *bounding);

void mVideoBackground(MImage *src,MImage *bgd,int time_thresh,int diff_thresh);

void mImageMeanFilter(MImage *src,MImage *dst,int r);
void mImageMidValueFilter(MImage *src,MImage *dst);

void mImageDirection(MImage *src,MImage *dst,int r,int thresh);
void mImageDirectionGradient(MImage *src,MImage *dst,int direction,int r,int thresh);
void mImageGradientValue(MImage *src,MImage *dst,int r,int thresh);
void mImageGradientCheck(MImage *src,MImage *dst,int r,int thresh);
void mImageGradientAdaptCheck(MImage *src,MImage *dst,int r,float thresh);
void mImageGradient(MImage *src,MImage *direction,MImage *value,int r,int thresh);
void mImageGradientFilter(MImage *dir,MImage *value,MImage *ddst,MImage *vdst,int r);
void mImageGradientSuppression(MImage *dir,MImage *value,MImage *dst,int r);
void mImageCanny(MImage *src,MImage *dst,int r,int thresh);

int mColorCluster(MImage *src,MImage *dst,MSheet *color,MList *list,int r,int thresh);


void mImageCoordinateTransform(MImage *src,MImage *dst,float (*x_func)(int,int,void *),float (*y_func)(int,int,void *),void *para);
void mImageRotate(MImage *src,MImage *dst,MImagePoint *src_hold,MImagePoint *dst_hold,float angle);
void mImageAffineCorrection(MImage *src,MImage *dst,MImagePoint *ps,MImagePoint *pd);
void mImagePerspectiveCorrection(MImage *src,MImage *dst,MImagePoint *ps,MImagePoint *pd);


#ifdef __cplusplus
}
#endif

#endif
