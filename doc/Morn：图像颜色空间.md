## Morn：图像颜色空间

关于什么是颜色空间，就不多废话了。

Morn支持四种颜色空间即Gray，RGB（RGBA）、YUV和HSV。在Morn中被定义为：

```
MORN_IMAGE_GRAY
MORN_IMAGE_RGB 
MORN_IMAGE_RGBA
MORN_IMAGE_YUV 
MORN_IMAGE_HSV 
```

查看当前图像所使用的颜色空间，使用下面的函数：

```c
int image_type = mInfoGet(&(img->info),"image_type");
```

单通道图像在创建时，默认图片类型是MORN_IMAGE_GRAY，三通道图像在创建时，默认图片类型是

MORN_IMAGE_RGB，四通道图像在创建时，默认图片类型是MORN_IMAGE_RGBA。

对于RGB图像，图像的**1通道为B通道（蓝原色）、2通道为G通道（绿原色）、3通道为R通道（红原色） **。

RGBA图像与RGB图像相同，增加的第4通道为A通道。

对于YUV图像，图像的1通道为Y通道（亮度），2通道为U通道，3通道为V通道。

对于HSV图像，图像的1通道为H通道（色调），2通道为S通道（饱和度），3通道为V通道（明度），这里需要说明的是：Morn中HSV图像中各通道的取值范围都是[0,240]，比如H通道，0表示红色，40表示黄色，80表示绿色，120表示青色，160表示蓝色，200表示粉红色。S通道0表示饱和度为0（灰色），240表示饱和度为100%。V通道0表示最暗，240表示最亮。

#### 接口

##### 转换为灰度图

```c
void mImageRGBToGray(MImage *src,MImage *dst);
void mImageYUVToGray(MImage *src,MImage *dst);
void mImageToGray(MImage *src,MImage *dst);
```

这里的src是输入图像，dst是输出图像，dst取默认值（NULL）的时候，转换后的图像保存回src里。下文其它接口相同。

mImageRGBToGray是RGB（或RGBA）图像转成灰度图，这个最为常用。

mImageYUVToGray是YUV图像转成灰度图。

mImageToGray是一个总的接口，输入可以是RGB图像，也可以是YUV图像。

##### 转换成RGB图像

```c
void mImageYUVToRGB(MImage *src,MImage *dst);
void mImageHSVToRGB(MImage *src,MImage *dst);
void mImageToRGB(MImage *src,MImage *dst);
```

mImageYUVToRGB是YUV图像转RGB图像。

mImageHSVToRGB是HSV图像转RGB图像。

mImageToRGB是一个总的接口，输入可以是YUV图像，也可以是HSV图像。

##### RGB图像转换成YUV图像

```c
void mImageRGBToYUV(MImage *src,MImage *dst);
```

##### RGB图像转换成HSV图像

```c
void mImageRGBToHSV(MImage *src,MImage *dst);
```

