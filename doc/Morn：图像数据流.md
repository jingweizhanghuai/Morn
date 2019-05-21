## Morn：图像数据流

这个主要是为了方便Morn和其它函数库之间进行转换。

之前说过了，Morn中图像使用MImage来表示的，MImage中，图像数据按照通道-行-列的顺序来存储，并且数据在内存中是不连续的。

但是其它的函数库，其图像的数据存储肯定和Morn是不一样的，例如著名的OpenCV中，不管是cv::Mat还是CvMat或者IplImage，它们的数据都是按照行-列-通道的顺序来存储的，并且数据在内存中是连续的。

假设，你在工程中使用了多种图像格式，那么怎么把其它格式的图像转成Morn呢？这就是以下接口所解决的问题。

#### 接口

```c
void mImageDataInput(TypeName,MImage *img,Type *stream,int stream_type,unsigned char (*func)(Type,void *),void *para);
void mImageDataOutput(TypeName,MImage *img,Type *stream,int stream_type,unsigned char (*func)(Type,void *),void *para);
```

以上是数据的导入和导出。和Morn中其它接口一样，Type可以选择（U8、S8、U16、S16、U32、S32、F32和D64）。

img是Morn中所使用的图像，stream是图像数据流的首地址（图像是连续存储的），stream_type是数据流的格式，它包括：

```c
MORN_IMAGE_STREAM_HWC：按照行-列-通道顺序存储，OpenCV就是这种存储方式
MORN_IMAGE_STREAM_HCW：按照行-通道-列顺序存储
MORN_IMAGE_STREAM_CHW：按照通道-行-列顺序存储，Morn就是这种存储方式
MORN_IMAGE_STREAM_CWH：按照通道-列-行顺序存储
MORN_IMAGE_STREAM_WHC：按照列-行-通道顺序存储
MORN_IMAGE_STREAM_WCH：按照列-通道-行顺序存储
```

stream_type的默认值是MORN_IMAGE_STREAM_HWC。

func是在数据导入导出的时候进行的数据转换函数，例如你可以在导入导出的时候进行数据归一化、数据标准化或者像素值映射等。当然，你也可以什么都不做，这时func输入NULL就行了，para是func的参数，不需要的话就输入NULL。

以下为两个示例程序，首先从OpenCV里导入数据到MImage（用OpenCV读入图片，然后转成MImage，再用Morn保存图片）：

```c
int main()
{
    Mat src = imread("./test.jpg");
    
    MImage *dst = mImageCreate(src.channels(),src.rows,src.cols,NULL);
    mImageDataInput(U8,dst,src.data,DFLT,NULL,NULL);
    
    mImageSave(dst,"./test_out.bmp");
    
    mImageRelease(dst);
    return 0；
}
```

然后是从MImage里导出数据到OpenCV（用Morn读入图片，然后转成cv::Mat，再用OpenCV保存图片）：

```c
int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("./test.jpg",src);
    
    Mat dst = Mat(src->height,src->width,CV_8UC3);
    mImageDataOutput(U8,src,dst.data,DFLT,NULL,NULL);
    
    imwrite("./test_out.png",dst);
    
    mImageRelease(src);
    return 0;
}
```

