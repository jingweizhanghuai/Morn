## Morn：视频背景提取

首先把最重要的事情说下：这个视频背景的提取，只能提取固定场景的视频，也就是由位置固定的枪击摄像头拍摄的，视野没有旋转，焦距没有伸缩的摄像头。



#### 函数

```c
void mVideoBackground(MImage *src,MImage *bgd,int time_thresh,int diff_thresh);
```

其中src是输入的视频序列图像，通常此函数在循环中被执行，src按照视频序列的顺序依次送入。

bgd是输出的背景，这里要说明的是，只有视频持续一段时间后，才能提取到视频的背景图像。

time_thresh是视频帧阈值，当视频中物体运动缓慢时，此值需要设置比较大，当视频中物体运动迅速时，此值需要设置比较小。

diff_thresh时像素差阈值，视频对比度高时，此值应设置较大，反之设置较小。



##### 示例

假设我们已经有了一个视频帧序列，程序目的是提取其背景。

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_image.h"
int main()
{
    MImage *img = mImageCreate(DFLT,DFLT,DFLT,NULL);
    MImage *bgd = mImageCreate(DFLT,DFLT,DFLT,NULL);
    char filename[128];
    for(int n=1;n<400;n++)
    {
        sprintf(filename,"E:/morn/test/img%05d.jpg",n);
        mImageLoad(filename,img);
        mVideoBackground(img,bgd,32,20);
        sprintf(filename,"E:/morn/test2/img%05d.jpg",n);
        mImageSave(bgd,filename);
    }
    mImageRelease(img);
    mImageRelease(bgd);
    return 0;
}
```

这里，我们输入的是视频帧：

![background_in](./background_in.gif)

得到的视频帧是：

![background_in](./background_out.gif)

可以看到，所得到的背景中，前景物体是逐渐被”擦除“的，在前n帧内所得并非完整的背景图片。