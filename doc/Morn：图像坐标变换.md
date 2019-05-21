## Morn：图像坐标变换

其实之前说过的图像缩放也是图像坐标变换的一种，只不过缩放比较常用，所以单独拿出来说了。这里说说其它的图像坐标变换。

#### 接口

##### 图像旋转

```c
void mImageRotate(MImage *src,MImage *dst,MImagePoint *src_hold,MImagePoint *dst_hold,float angle);
```

这里，src是输入的源图像，dst是输出的旋转后的图像，dst的默认值是src（也就是保存回原图），这里需要说明的是，dst如果没有设置明确的图像尺寸的话，输出图像将与输入图像同尺寸。

src_hold和dst_hold是图像的旋转点（图像绕着这个点做旋转，dst上的dst_hold就对应了src上的src_hold）。如果不指定旋转点（输入是DFLT）的话，那么旋转点将取图像上的中央点。换一个说法，做图像旋转的时候可以同时做图像平移，src_hold和dst_hold取默认值的意思就是不做平移。

angle是图像的旋转角度（顺时针，注意是角度不是弧度），默认值是90度。

比如：

```c
mImageRotate(src,dst,DFLT,DFLT,60);
```

它的执行结果如下：



