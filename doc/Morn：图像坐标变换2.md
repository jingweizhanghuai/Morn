## Morn：图像坐标变换2

之前说过一次图像坐标变换，上次说的主要是整幅图像的坐标变换，这次重点说说图像的局部坐标变换，也就是图像模板变换。

#### 接口

```c
void mImageTemplateTransform(MImage *src,MImage *dst,MObject *temp,int x,int y);
```

src不用说了，就是源图像，dst是变换后的目标图像，默认值是src（即不指定目标图片的话，就保存（覆盖）到原图）。

temp就是变换模板。所以使用此函数之前，你首先得有一个模板，或者需要生成一个模板。

x、y是源图像上的一个坐标。它是模板作用的位置。

在Morn里，给出了一种模板，叫透镜模板，透镜可以是放大镜也可以是放小镜。生成这种模板使用以下函数：

```c
void mImageLensTemplate(MObject *temp,float k,int r);
```

temp就是要生成的模板，k和r是模板的参数。其中r是模板的尺寸（多少个像素？），k表示了模板的作用是放大还是缩小，它的取值需在[-1,1]之间，当k为整数时表示放大，k为负数的时候表示缩小。

#### 示例

这里写了个程序用于演示模板变换：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("./test2.jpg",src);
    
    MImage *dst = mImageCreate(DFLT,DFLT,DFLT,NULL);
    MObject *temp = mObjectCreate(NULL);
    
    mImageLensTemplate(temp,-0.5f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out1.jpg");
    
    mImageLensTemplate(temp,-0.3f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out2.jpg");
    
    mImageLensTemplate(temp,-0.1f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out3.jpg");
    
    mImageLensTemplate(temp,0.1f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out4.jpg");
    
    mImageLensTemplate(temp,0.3f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out5.jpg");
    
    mImageLensTemplate(temp,0.5f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out6.jpg");
    
    mImageRelease(src);
    mImageRelease(dst);
    mObjectRelease(temp);
    return 0;
}
```

原始图像是：

![](E:\morn\doc\test2.jpg)

当k=-0.5时，结果为：

![](E:\morn\doc\test_Transform_out1.jpg)

当k=-0.3时，结果为：

![](E:\morn\doc\test_Transform_out2.jpg)

当k=-0.1时，结果为：

![](E:\morn\doc\test_Transform_out3.jpg)

当k=0.1时，结果为：

![](E:\morn\doc\test_Transform_out4.jpg)

当k=0.3时，结果为：

![](E:\morn\doc\test_Transform_out5.jpg)

当k=0.5时，结果为：

![](E:\morn\doc\test_Transform_out6.jpg)

可以看到结果还是挺有意思，这种透镜模板可以用来做“瘦脸”，不过要说明一下，这个模板不是用来做“美颜”的，如果你需要的话，可以写出更好的美颜模板（原理还是这个原理）。