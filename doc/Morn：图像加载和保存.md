## Morn：MImage图像加载和保存

Morn支持的加载和保存图像的格式包括jpg、png和bmp三种。

这其中，jpg和png格式都需要借助第三方库，jpg需要借助libjpeg、png需要借助libpng（libpng又依赖libz），所以如果你想用Morn来读取jpg、png的话，你需要首先安装libjpeg和libpng（好在，这两个库可能是目前装机量最大的两个函数库之二，安装很简单，不会的话，百度一下）。这也是Morn里为数不多的需要依赖第三方库的地方（主要是因为Morn的作者水平不够，自己不会写，否则就不依赖了）。

### 接口

```c
void mBMPSave(MImage *src,const char *filename);
void mBMPLoad(const char *filename,MImage *dst);
void mJPGSave(MImage *src,const char *filename);
void mJPGLoad(const char *filename,MImage *dst);
void mPNGSave(MImage *src,const char *filename);
void mPNGLoad(const char *filename,MImage *dst);
```

以上分别是bmp、jpg、和png图像的保存和加载函数。对于Save函数，输入是图像MImage，输出是文件，filename是文件的路径。对于Load函数，输入是图像的路径filename，输出是图像MImage。

此外，还可以使用以下函数：

```c
void mImageLoad(const char *filename,MImage *img);
void mImageSave(MImage *img,const char *filename);
```

`mImageLoad`和上面图片读取函数的区别在于，它会根据文件数据，自动判断图片属于什么格式。

`mImageSave`和上面图片保存函数的区别在于，它会根据文件的后缀名判断文件的类型，然后保存图片。

典型的图片读写和保存如下：

```c
MImage *src=mImageCreate(DFLT,DFLT,DFLT,NULL);
mImageLoad("test_in.jpg",src);
...
mImageSave(src,"test_out.bmp");
mImageRelease(src);
```

### 工具

在[../tool](../tool)文件夹里有一个image_format工具，可以用于转换（批量转换）图片格式，所使用的就是上面这些函数（把一个格式的图片读进来，在保存成另一种格式）。

