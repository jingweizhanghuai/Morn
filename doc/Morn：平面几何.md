## Morn：平面几何

Morn提供了一系列平面几何的算法，也就是平面上的点、线、三角形、四边形、多边形之间的关系的算法。

#### 定义

##### MImagePoint的定义

```c
typedef struct MImagePoint
{
    float x;
    float y;
}MImagePoint;
```

MImagePoint就是平面上的点，有两个坐标，x是横坐标，y是纵坐标。

##### MImageRect的定义

```c
typedef struct MImageRect
{
    int x1;
    int y1;
    int x2;
    int y2;
}MImageRect;
```

MImageRect就是矩形，有四个坐标，x1是左边沿，x2是右边沿，y1是上边沿，y2是下边沿。（x1,y1)是左上角点的坐标。（x2-x1)是矩形的宽度，（y2-y1)是矩形的高度。

##### 其它图形的定义

其它图形皆可看作是点的序列，比如线就是两个点的序列，三角形就是三个点的有序序列，多边形就是多个点的有序序列。因此，其它图形用容器MList来表示，容器中的元素就是MImagePoint。

三角形、四边形、五边形等等诸类，以下统称多边形。

一个任意形状的外轮廓也可以看作是一个多边形，只不过是一个有很多很多顶点的多边形。



#### 接口

##### 图形初始化

```c
void mPoint(MImagePoint *point,float x,float y);
void mRect(MImageRect *rect,int x1,int y1,int x2,int y2);
void mPolygon(MList *polygon,int num,...);
```

mPoint函数就是初始化一个点，比如`mPoint（p,1,2);`实际上就是`p->x=1;p->y=2`，也就是指定了一个坐标为(1,2)的点。

mRect函数就是初始化一个矩形，比如`mRect(rect,1,2,3,4);`就是初始化了一个左上角点为(1,2)，右下角点为(3,4)的矩形，需要说的是`mRect(rect,3,4,1,2);`或者`mRect(rect,1,4,3,2);`初始化得到的都是同一个矩形。

mPolygon就是初始化一个线或者一个多边形。num是点的个数，num=2就是初始化一条线，num=5就是初始化一个五边形。比如`mPolygon(polygon,3,1,2,3,4,5,6);`就是初始化了一个三角形，三角形的三个顶点分别是(1,2)，(3,4)，(5,6)。这里需要提醒一句，这里的polygon需要使用mListCreate来创建，用完后要用mListRelease来释放。



##### 点点距离

```c
float mPointDistance(MImagePoint *P1,MImagePoint *P2);
```

也就是线段的长度或矩形对角线长度，返回值为距离（或长度）。



##### 点线关系

```c
int mLinePointCheck(MList *line,MImagePoint *point);
```

如果点在线上，则返回0，否则返回非0。



##### 点线距离

```c
float mPointVerticalDistance(MImagePoint *point,MList *line,MImagePoint *pedal);
```

pedal是算得的垂足的坐标，如果不需要垂足的话，可以设为NULL。

函数的返回值就是点到直线的距离（也就是点到垂足的距离）。



##### 线线关系

```c
int mLineCrossCheck(MList *line1,MList *line2);
```

这个函数里的线是线段，不是直线，也不是射线。两线相交则返回1，否则返回0。



##### 两线交点

```c
int mLineCrossPoint(MList *line1,Mlist *line2,MImagePoint *point)
```

两线相交则返回1，否则返回0。point为输出的交点。



##### 点形关系

```c
int mPointInPolygon(MImagePoint *point,MList *polygon);
```

如果点在多边形内，则返回1，否则返回0。



##### 线形关系、形形关系

```c
int mPolygonCross(MList *polygon1,MList *polygon2);
```

判断两个多边形（或者一条线和一个多边形）是否有交叠，相交则返回1，否则返回0。



##### 线、多边形边沿点遍历

```c
void mPolygonSideTravel(MList *polygon,int stride,void (*func)(MImagePoint *,void *),void *para)；
```

遍历线（或多边形的边）上的点。polygon是需要遍历的线或图形，stride是遍历的时候的步长，func是对对遍历点所执行的函数，para是func函数的参数。

一个最简单的应用，比如在图像上画图形，就是遍历边沿上的每个点，对其涂色。



##### 多变形的面积

```c
float mPolygonArea(MList *polygon);
```

返回值是面积。此外，还可以直接用以下两个接口计算面积：

```c
float mTriangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3);
float mQuadrangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3,MImagePoint *p4);
```

mTriangleArea是计算三角形的面积，mQuadrangleArea是计算四边形的面积。



##### 矩形间交并面积

```c
float mRectIntersetArea(MImageRect *rect1,MImageRect *rect2);
float mRectUnionsetArea(MImageRect *rect1,MImageRect *rect2);
```

mRectIntersetArea返回的是两个矩形交集的面积，mRectUnionsetArea返回的是两个矩形并集的面积。



##### 多边形交并面积

```c
float mPolygonIntersetArea(MList *polygon1,MList *polygon2);
float mPolygonUnionsetArea(MList *polygon1,MList *polygon2);
```

mPolygonIntersetArea返回的是两个多边形交集的面积，mPolygonUnionsetArea返回的是两个多边形并集的面积。

这个函数里的多边形需要是凸多边形。



##### 凸多边形检查

```c
int mPolygonConcaveCheck(MList *polygon);
```

如果多边形是凸多边形则返回1，否则返回0。



##### 边界轮廓的最小外接长方形

```c
void mShapeBounding(MList *shape,MList *bounding)；
```

这里的外界长方形不一定是MImageRect所定义的那种“正”的矩形，而是任意角度的矩形，用矩形的四个顶点表示。

shape是输入的待处理的轮廓边界，bounding是输出的最小外接长方形。



##### 轮廓简化

```c
void mEdgeBoundary(MList *edge,MList *polygon,float thresh);
```

一个形状的轮廓实际上就是一个多边形，但是这个多边形的顶点太多，用来做几何运算的话会增加运算量，所以可以对轮廓做适当的简化，用一个较少顶点的多边形来表示这个轮廓。

这里edge是待处理的轮廓，polygon是算得的简化后的轮廓。thresh是简化时误差的阈值。此阈值越大，则获得的多边形越简单，但是与原有轮廓的差别也越大。反之则多边形越负责，但是会更贴近原轮廓。



##### 点集凸包

```c
void mConvexHull(MList *point,MList *polygon);
```

凸包的定义是：①它是一个多边形；②多边形的顶点都是点集中的点；③点集中除了多边形顶点以外的点，其余的都在凸包多边形内。

这里point是点集，polygon是算得的凸包。



#### 示例

这里写了一小段程序，以作示例：

```c
int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("./test_geometry.jpg",src);
    
    MImage *dst = mImageCreate(1,src->height,src->width,NULL);
    int thresh(unsigned char *data,void *para)
    {
        int min=MIN(data[0],MIN(data[1],data[2]));
        int max=MAX(data[0],MAX(data[1],data[2]));
        int mean = (data[0]+data[1]+data[2])/3;
        return ((mean>80)&&(max-min<50))?255:0;
    }
    mImageOperate(src,dst,thresh,NULL);
    mImageBinaryFilter(dst,dst,2,3);
    mImageSave(dst,"./test_geometry_bin.jpg");
    
    MSheet *sheet=mSheetCreate(DFLT,NULL,NULL);
    mImageBinaryEdge(dst,sheet,NULL);
    
    MList *polygon1 = mListCreate(sheet->col[0],sheet->data[0]);
    MList *polygon2 = mListCreate(sheet->col[1],sheet->data[1]);
    
    unsigned char color1[3] = {255,0,0};
    unsigned char color2[3] = {0,0,255};
    
    mEdgeBoundary(polygon1,polygon1,2);
    mEdgeBoundary(polygon2,polygon2,2);
    for(int i=0;i<polygon1->num;i++)
        mImageDrawPoint(src,src,polygon1->data[i],color1,3);
    for(int i=0;i<polygon2->num;i++)
        mImageDrawPoint(src,src,polygon2->data[i],color2,3);
    mImageSave(src,"./test_geometry_rst1.bmp");
    
    float area1 = mPolygonArea(polygon1);
    printf("area1 is %f\n",area1);
    float area2 = mPolygonArea(polygon2);
    printf("area2 is %f\n",area2);
    
    mShapeBounding(polygon1,polygon1);
    mImageDrawShape(src,src,polygon1,color1,2);
    mShapeBounding(polygon2,polygon2);
    mImageDrawShape(src,src,polygon2,color2,2);
    mImageSave(src,"./test_geometry_rst2.bmp");
    
    mImageRelease(src);
    mImageRelease(dst);
    mSheetRelease(sheet);
    mListRelease(polygon1);
    mListRelease(polygon2);
    
    return 0;
}
```

原图是这样的：

![test_geometry](.\test_geometry.jpg)

用一个阈值，将图像转换成二值图像，是这样的：

![test_geometry_bin](.\test_geometry_bin.jpg)

从二值图像上提取到图像边缘后，验证了这里的三个几何算法。

首先是轮廓简化，将简化后的轮廓点，画在图上，如下：

![test_geometry_rst1](.\test_geometry_rst1.bmp)

然后，计算了简化后的多边形的面积。

最后，计算了轮廓的最小外接长方形，如下图：

![test_geometry_rst2](E:\morn\doc\test_geometry_rst2.bmp)

