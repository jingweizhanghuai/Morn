## 定义

### 数据结构

----

#### 图像

``` c
typedef struct MImage {
    int cn;                         // 通道数
    int height;
    int width;
    
    unsigned char **data[MORN_MAX_IMAGE_CN];
    
    MMemory *memory;
    MHandleSet *handle;
    
    MImageBorder *border;
    
    struct
    {
        int image_type;              // 类型
    }info;
    
    void *reserve;
}MImage;
```

##### 成员

* cn：图像通道数，最大支持4通道
* height：图像高度
* width：图像宽度
* data：各通道图像的像素值（MORN_MAX_IMAGE_CN为4）
* memory：图像内存（不建议外部使用）
* handle：图像操作集（不建议以外部使用）
* border：图像处理边界
* info.image_type：图像类型
* reserve：预留

----

#### 图像边界

``` c
typedef struct MImageBorder
{
    short y1;
    short y2;
    short *x1;
    short *x2;
}MImageBorder;
```

##### 成员

* y1：图像上边界
* y2：图像下边界
* x1：图像各行左边界
* x2：图像各行右边界

----

### 函数

----

#### 图像创建

``` c
MImage *mImageCreate(int cn,int height,int width,unsigned char **data[])
```

##### 参数

* cn：输入，图像通道数，若传入为DFLT，则不指定通道数
* height：输入，图像高度，若传入为DFLT，则不指定高度
* width：输入，图像宽度，若传入为DFLT，则不指定宽度
* data：输入，图像各通道像素的初始值，若传入为NULL，则图像不初始化

##### 返回值

所创建的图像指针

----

#### 图像释放

``` c
void mImageRelease(MImage *img)
```

##### 参数

* img：所需释放的图像

##### 返回值

无

----

#### 图像重定义

``` c
void mImageRedefine(MImage *img,int cn,int height,int width)
```

##### 参数

* img：输入，所需要重定义的图像
* cn：输入，重定义图像的通道数，若传入为DFLT，则通道数不变
* height：输入，重定义图像的高度，若传入为DFLT，则高度不变
* width：输入，重定义图像的宽度，若传入为DFLT，则宽度不变

##### 返回值

无

----

#### 图像截取

``` c
void mImageCut(MImage *img,MImage *ROI,int x1,int x2,int y1,int y2)
```

##### 参数

* img：输入，源图像
* ROI：输出，截取后的图像
* x1：输入，截取图像在源图像上的左边界，若传入为DFLT，则左边界为0
* x2：输入，截取图像在源图像上的右边界，若传入为DFLT，则右边界为img宽度
* y1：输入，截取图像在源图像上的上边界，若传入为DFLT，则上边界为0
* y2：输入，截取图像在源图像上的下边界，若传入为DFLT，则下边界为img的高度

##### 返回值

无

----

## 图像文件

### 函数

----

#### 图像BMP格式保存

``` c
void mBMPSave(MFile *file,MImage *src)
```

##### 参数

* file：输入，待保存的文件
* src：输入，待保存的图像

##### 返回值

无

----

#### 图像BMP格式读取

```c
void mBMPRead(MFile *file,MImage *dst)
```

##### 参数

- file：输入，待读取的文件
- dst：输出，读取到的图像

##### 返回值

无

----

#### 图像JPG格式保存

```c
void mJPGSave(MFile *file,MImage *src)
```

##### 参数

- file：输入，待保存的文件
- src：输入，待保存的图像

##### 返回值

无

#####备注

此函数调用libjpeg函数库

----

#### 图像JPG格式读取

```c
void mJPGRead(MFile *file,MImage *dst)
```

##### 参数

- file：输入，待读取的文件
- dst：输出，读取到的图像

##### 返回值

无

##### 备注

此函数调用libjpeg函数库

----

#### 图像PNG格式保存

```c
void mPNGSave(MFile *file,MImage *src)
```

##### 参数

- file：输入，待保存的文件
- src：输入，待保存的图像

##### 返回值

无

##### 备注

此函数调用libpng函数库

------

#### 图像PNG格式读取

```c
void mPNGRead(MFile *file,MImage *dst)
```

##### 参数

- file：输入，待读取的文件
- dst：输出，读取到的图像

##### 返回值

无

##### 备注

此函数调用libpng函数库

----

## 图像运算

### 函数

----

#### 图像差异

``` c
void mImageDiff(MImage *src1,MImage *src2,MImage *diff)
```

##### 参数

* src1：输入，源图像
* src2：输入，源图像
* diff：输出，src1与src2各像素的差值绝对值，若传入为NULL，则差值图像保存入src1

##### 返回值

无

----

#### 图像加法

``` c
void mImageAdd(MImage *src1,MImage *src2,MImage *dst)
```

##### 参数

- src1：输入，源图像
- src2：输入，源图像
- dst：输出，src1与src2各像素之和（当大于255时，取255），若传入为NULL，则差值图像保存入src1

##### 返回值

无

----

#### 图像减法

```c
void mImageAdd(MImage *src1,MImage *src2,MImage *dst)
```

##### 参数

- src1：输入，源图像
- src2：输入，源图像
- dst：输出，src1与src2各像素之和（当小于0时，取0），若传入为NULL，则差值图像保存入src1

##### 返回值

无

----

#### 图像反色

``` c
void mImageInvert(MImage *src,MImage *dst)
```

##### 参数

* src：输入，源图像
* dst：输出，反色（255减去源图像像素）后的图像，若传入为NULL，则反色后图像保存入src

##### 返回值

无

----

#### 图像像素线性变换

#### 

```c
void mImageLinearMap(MImage *src,MImage *dst,float k,float,b)
```

##### 参数

- src：输入，源图像
- dst：输出，线性变换（源图像像素，乘以k，加上b）后的图像，若传入为NULL，则反色后图像保存入src
- k：输入，线性变换参数
- b：输入，线性变换参数

##### 返回值

无

----

#### 图像像素阈值限定

``` c
void mImageThreshold(MImage *src,MImage *dst,MThreshold *thresh)
```

##### 参数

* src：输入，源图像
* dst：输出，阈值限定后的输出图像，若传入为NULL，则输出图像保存入src
* thresh：输入，阈值

##### 返回值

无

----

## 图像颜色空间

### 函数

----

#### 图像RGB图转灰度图

``` c
void mImageRGBToGray(MImage *src,MImage *dst)
```

##### 参数

* src：输入，源RGB图像
* dst：输出，转换后的灰度图，若传入为NULL，则输出图像存入src

##### 返回值

无

----

####图像YUV图转灰度图

``` c
void mImageYUVToGray(MImage *src,MImage *dst)
```

##### 参数

- src：输入，源YUV图像
- dst：输出，转换后的灰度图，若传入为NULL，则输出图像存入src

##### 返回值

无

----

## 图像形变

### 函数

----







