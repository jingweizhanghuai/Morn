## Morn：MTensor张量

MTensor其实准确的说应该是四维张量（而不是任意维的）。它只用在Morn的深度学习框架里。



#### 定义

```c
typedef struct MTensor{
    int batch;
    int channel;
    int height;
    int width;
    float **data;
    MList *handle;
    MInfo info;
    void *reserve;
}MTensor;
```

这是个典型的Morn结构体，其中：handle、info和reserve是几乎所有Morn结构体所共有的。

batch就是批次，channel就是通道（厚度），height就是高度，width就是宽度。

data是个索引，里面存放的是张量中每个元素的值。比如张量tns中第2批，第4通道，第3行，第5列的值就是tns->data\[2][4*tns->height\*tns->width+3*tns->width+5]。

也就是说张量里，同一批次的数据在内存里是连续存放的。这么放主要是为了方便用blas。同一批次数据的存放顺序是通道、列、行。

另外，MTensor中的数据都是float类型的，因此可知，Morn的深度学习框架用的是浮点运算。



#### 函数

这里列举的MTensor函数都定义在文件[../src/dl/morn_Tensor.c](../src/dl/morn_Tensor.c)中。



##### 创建张量

```c
MTensor *mTensorCreate(int batch,int channel,int height,int width,float **data);
```

典型的Morn创建函数，MTensor只能也必须用这个函数创建。

batch就是张量的批次，channel就是通道（厚度），height就是高度，width就是宽度。它们的默认值都是0，取默认值时，只会创建一个向量头，不会为数据申请空间。

data是数据索引，如果没有初始化的外部数据，那么data传入NULL。



##### 释放张量

```c
void mTensorRelease(MTensor *tns);
```

用`mTensorCreate`创建的MTensor**必须且只能**用`mTensorRelease`来释放。



##### 张量重定义

```c
void mTensorRedefine(MTensor *tns,int batch,int channel,int height,int width,float **data);
```

如果你想改变一个张量的批数、通道数、行数、列数，那么你就需要使用`mTensorRedefine`。这个函数会重新分配内存空间，重新生成索引。当然，它也会把你对该向量的handle都重置了。

