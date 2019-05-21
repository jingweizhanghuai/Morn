## Morn：内存块MMemory

因为Morn的作者很不喜欢把内存来来回回的申请释放，也不喜欢把内存拷贝来拷贝去，所以Morn使用了一个叫内存块的东西。

虽然MMemory在整个Morn中非常非常的重要，但是Morn的使用者完全可以无视它，通常你用不到它。

所以，这里先说使用者需要了解的东西。

**Morn中几乎所有的数据结构中（除非作者特别声明的），数据的存储都不是（或者不一定是）连续的。**比如图像MImage中，1通道和2通道的数据不是连续的，同一通道里，第一行的数据和第二行的数据也是不连续的。所以你不能试图用一层循环去遍历图像的每个像素，你必须使用索引。再比如，容器MList中，第二个元素不一定保存在第一个元素的后面，你要遍历整个列表也必须使用索引。诸如此类等等。

看完上面的结论，如果你不是想做Morn的源码开发的话，就不需要看后面的了。

Morn中，几乎所有的数据都被存在内存块里，内存块顾名思义就是一块连续的内存。

Morn在使用内存的时候，会申请一块内存来使用，然后可能用着用着就不够用了（比如你对数据结构进行了Redefine，或者你往容器里写入了元素，再或者你往树上新增了节点之类），这时候不是申请一块更大的内存，然后把已有的内容复制过去，而是新申请一个内存块，接着用，然后新的索引指向新的内存块。这样新的内存块和原来的内存块就是不连续的。

所有的内存块都会在数据结构体被Release的时候全部释放掉。



####MMemory的定义

```c
#define MMemory MList
```

可以看到，MMemory就是一个MList表单，表单中每个元素就是一块内存，新增一个内存块就是在MList上新增一个元素。



#### MMemory的基本操作

**创建MMemory**：

```
MMemory *mMemoryCreate(int num,int size);
```

其中num是内存单元的个数，size是每个内存单元的大小，也就是说新开的内存块的容量可以理解为是size×num（不过因为数据对齐的原因，实际容量可能会稍大于这个值）。num和size都可以取默认值，默认值为0，此时会返回一个容量为0的内存块。



**释放MMemory**：

```
void mMemoryRelease(MMemory *memory);
```

此函数通常会在各种数据结构Release的时候被使用（但不一定是在Release函数中被使用）。



**获取内存块的容量**：

```c
int mMemorySize(MMemory *memory);
```

返回值就是内存块的容量，单位为字节。



**检查一个指针是不是指向了内存块里的数据**：

```C
int mMemoryCheck(MMemory *memory,void *check);
```

memory是内存块列表，check是待检查的指针，在此内存块列表里就返回1，否则返回0。



**列表里新增一个内存块**：

```c
void *mMemoryAppend(MMemory *memory,int num,int size);
```

num是内存单元的个数，size是每个内存单元的大小。



**向内存块里写入数据**：

```c
void *mMemoryWrite(MMemory *memory,void *data,int size);
```

data就是待写入的数据，size是待写入数据的大小（字节）。返回值是写入后，数据在内存块里的地址。

写入的是字符串时，size的值可以设置成DFLT。

data也可以传入NULL，这时候只是在内存块里申请空间，但是(data==NULL)&&(size==0)是不行的。



**生成内存索引**：

```c
void mMemoryIndex(MMemory *memory,int num,int size,void *index[]);
```

num是内存单元的个数，size是每个内存单元的大小。index是输出的索引，index[0]就指向第1个大小为size的内存单元，index[1]就指向第2个大小为size的内存单元，以此类推，index[num-1]就指向最后一个大小为size的内存单元。但是注意，index[1]不一定等于(index[0]+size)，也就是不一定连续。



**根据索引回收内存块**：

```c
void mMemoryCollect(MMemory *memory,void **index,int num);
```

内存块都是需要通过索引来访问的，如果有一个内存块，没有任何索引指向它，那么就意味着它没有用了，可以回收了（内存块可以被Free了）。当然，开发者可以自己定一种策略，比如隔多长时间回收一次内存，或者其它（取决于对速度和内存占用的需求）。

index就是内存索引，num是索引的数量。



**内存块拷贝**：

```c
void mMemoryCopy(MMemory *src,void **isrc,MMemory *dst,void **idst,int num);
```

这是从src拷贝到dst里（深拷贝），并生成新的索引。isrc是src的索引（输入），idst是dst的索引（输出），num是索引的数量。



**内存块合并**：

```c
void mMemoryMerge(MMemory *mem1,MMemory *mem2,MMemory *dst);
```

mem1和mem2两个内存块列表，合并成dst一个内存块列表。



