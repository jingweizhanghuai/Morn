## Morn：深度学习框架

我终究还是落了俗套，写了一个深度学习框架。



### 函数

Morn的深度学习框架，API力求简单，深度学习函数就两个，即预测和训练。



#### 预测

所谓预测，就是输入一个张量（或者，也可以有多个输入），然后输出一个张量（或者，也可以有多个输出）。

输入的张量就是预测的对象，它可以来自图片、音频、视频（图像序列）或者兼而有之。通常开发者需要把你的原始数据（图像、音频等）转换成张量，这个过程是预测的前处理过程。

输出的向量就是预测的结果，它表征的可以是分类的结果、检测的结果、分割的结果或者重建的结果等等。把输出的向量转换成所需要的结果的过程就是预测的后处理。

预测的过程，即从输入向量得到输出向量的过程，就是深度神经网络的一次前向传播。

```c
void mNetworkPredict(MFile *ini,char *name[],MTensor *tns[]);
```

这里，ini就是一个ini格式的配置文件，里面定义了深度网络的模型，以及其它配置参数。后续会重点说说这个文件怎么写。

tns就是输入输出的张量，通常，它包括至少一个输入张量，至少一个输出张量。

name是张量的名字，这个名字要与ini文件里张量的名字相同，程序根据这个名字来分配数据。



#### 训练

所谓训练，就是深度神经网络进行一次前向传播，再进行一次反向传播，反向传播过程中更新模型参数。

训练和预测的区别在于，预测是传入题目，得到答案，目的是获得输出张量。而训练是同时传入题目和标准答案，目的是让模型学习（也就是更新模型参数）。

```c
void mNetworkTrain(MFile *ini,char *name[],MTensor *tns[]);
```

这里所有的参数都和`mNetworkPredict`一样。

这里的训练是指执行一次正向传播和一次反向传播。当然，一个工程的训练需要不断的去：①准备数据、②正向传播、③反向传播。其中②③两步由此`mNetworkTrain`函数完成。所以开发者需要去写一个循环去执行以上三步。或者你可以试一下以下这个函数。

```c
void NetworkTrain(MFile *ini);
```

使用这个函数的前提是，你已经把所需要的所有训练数据写入了.morn文件，并且在ini文件里，指定了这些文件的路径。

`NetworkTrain`这个函数所执行的就是循环的：①把数据从文件里取出来，②执行`mNetworkTrain`函数训练。



### 示例

一个至简的深度学习训练：

```c
int main()
{
    MFile *ini = mFileCreate("./test.ini");
    NetworkTrain(ini);
    mFileRelease(ini);
    return 0;
}
```

这也许是目前最简单的深度学习训练程序，训练一个深度学习模型只需要三步，①创建一个MFile文件，②训练，③把MFile文件释放掉。

下面，以MNIST数据集（就是那个手写数字的数据集）为例：

首先，你需要准备数据，下载数据的时候网页上已经写明了数据的格式（或者你百度一下它的格式），所以，按照它的格式，把数据提取出来，然后保存成Morn所支持的.morn格式。

```c
int main()
{
    FILE *f_label= fopen("E:/minist/train-labels.idx1-ubyte","rb");
    FILE *f_data = fopen("E:/minist/train-images.idx3-ubyte","rb");
    MTensor *in = mTensorCreate(1,1,28,28,NULL);
    MTensor *out= mTensorCreate(1,1, 1,10,NULL);
    unsigned char label;
    unsigned char data[28*28];
    fseek(f_label,8,SEEK_SET);
    fseek(f_data,16,SEEK_SET);
    char mornname[256];
    for(int n=0;n<60000;n++)
    {
        fread(&label,1,1,f_label);
        fread(data,1,28*28,f_data);
        memset(out->data[0],0,10*sizeof(float));out->data[0][label]=1;
        for(int i=0;i<28*28;i++) in->data[0][i]=(float)data[i]/256;
        sprintf(mornname,"E:/minist/train_data/train%05d.morn",n);
        MFile *morn = mFileCreate(mornname);
        mMORNWrite(morn, "input",(void **)( in->data),1,28*28*sizeof(float));
        mMORNWrite(morn,"output",(void **)(out->data),1,   10*sizeof(float));
        mFileRelease(morn);
    }
    mTensorRelease(in);
    mTensorRelease(out);
    fclose(f_label);
    fclose(f_data);
    return 0;
}
```

然后，你需要定义你的网络，也就是写一个.ini文件。

```ini
[para]
time_max = 50000
time_save = 500
error_thresh = 0.01
coverage_ratio = 2
update_ratio = 0.1
train_batch = 32
predict_batch = 1
para_dir = ./
para_file = test_minist.morn
data_dir = E:\minist\train_data
rate = 0.0001
decay = 0.01
momentum = 0.95

[input]
type = Input
channel = 1
height = 28
width = 28

[conv]
type = Conv
prev = input
knl_num = 16
knl_height = 3
knl_width = 3
x_stride = 1
y_stride = 1

[pool]
type = MaxPool
prev = conv
pool_height = 2
pool_width = 2
x_stride = 2
y_stride = 2

[activ]
type = Activation
prev = pool
actv_func = ReLu

[connect1]
type = Connect
prev = activ
channel = 1
height = 1
width = 256

[connect2]
type = Connect
prev = connect1
channel = 1
height = 1
width = 10

[output]
type = Output
prev = connect2
channel = 1
height = 1
width = 10
loss = Softmax
```

这里可以看到：我定义了一个很简单的网络，它包括一个输入层、一个卷积层、一个池化层，一个激活层，两个全连接层和一个输出层。这里面的参数以后再讲。

最后，运行我们的第一个程序`NetworkTrain(ini);`就可以完成网络的训练。