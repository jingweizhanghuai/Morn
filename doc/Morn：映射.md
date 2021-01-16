## Morn：key-value映射

映射就是键值表，表中每个元素由两部分组成，即键（key）和值（value），只要输入键就能找到相应的值。

在Morn中，映射是用链表MChain来实现的。在键值表中增加一个映射也就是在链表中插入一个节点，在兼职表中删除一个映射也就是在链表中删除一个节点。（当然怎么快速的找到节点位置、插入节点、删除节点，这是算法需要解决的问题）

在Morn中，映射键值对中的键和值都可以是任意数据类型（整数、浮点数、字符串、数组、指针、结构体等）。

值得一提的是Morn的map不是红黑树，是二分查找。

Morn的map具有以下特点：

* 适用任何数据类型。
* 高性能
* 接口简单（三个接口）



### 接口

映射（map）首先是链表（MChain），所以在使用映射之前，需要先使用`mChainCreate`函数来创建映射（链表），在使用结束后需要使用`mChainRelease`函数来释放映射（链表）。

除此以外，映射增加的操作主要有：



#### 向映射中写入键值对

``` c
void *mMapWrite(MChain *map,const void *key,const void *value);
void *mMapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size);
```

这里，key就是键，指向任意类型的指针，key_size是key的长度（字节），如果key的类型是字符串，可不设置key_size或设置为DFLT。

value是key所对应的值，也是指向任意类型的指针，value_size是value的长度（字节），如果value的类型是字符串，那么value_size可不设置或设置为DFLT。

函数的返回值是value在map中存储的内存地址。

这里注意：对于每一个键值对，键必须是唯一的，如果两个键值对的键是相同的，那么后写入的将会覆盖之前的。



#### 从映射中读取键值对

```c
void *mMapRead(MChain *map,const char *key);
void *mMapRead(MChain *map,const char *key,char *value);
void *mMapRead(MChain *map,const void *key,int key_size,void *value,int value_size);
```

key是要读取的键，key_size是键的长度（字节），key为字符串时可不设置key_size或设置为DFLT。

value是要读出值的保存位置的指针，如果只是读，不需要copy到value指向的位置，那么value可以为NULL，value为NULL时，value_size没有意义，否则，value_size是要copy的字节数，value为字符串的话可以设置为DFLT。

函数返回值是所找到的key对应的value值在map中的内存地址。



#### 删除键值对

```c
void mMapDelete(MChain *map,const char *key);
void mMapDelete(MChain *map,const void *key,int key_size);
```

这就是把键为key的键值对从映射里删除掉。key_size在为字符串时可不设置或设置为DFLT。



#### Map遍历

```c
void mMapNodeOperate(MChain *map,void (*func)(const void *,const int,void *,int,void *),void *para);
```

其中func是遍历时对每个Node的操作函数，其必须使用以下形式：

```c
void func(const void *key,const int key_size,void *value,int value_size,void *para);
```

para是func中使用的参数。

注意：key-value在map中的排列是有序的，遍历时不能改变key的值，否则将破坏其有序性。



### 示例

下面是一些示例程序：

#### 示例一

key为字符串，value为整数。

```c
int main()
{
    MChain *map = mChainCreate();
    
    int n;
    n=0; mMapWrite(map,"zero" ,DFLT,&n,sizeof(int));
    n=1; mMapWrite(map,"one"  ,DFLT,&n,sizeof(int));
    n=2; mMapWrite(map,"two"  ,DFLT,&n,sizeof(int));
    n=3; mMapWrite(map,"three",DFLT,&n,sizeof(int));
    n=4; mMapWrite(map,"four" ,DFLT,&n,sizeof(int));
    n=5; mMapWrite(map,"five" ,DFLT,&n,sizeof(int));
    n=6; mMapWrite(map,"six"  ,DFLT,&n,sizeof(int));
    n=7; mMapWrite(map,"seven",DFLT,&n,sizeof(int));
    n=8; mMapWrite(map,"eight",DFLT,&n,sizeof(int));
    n=9; mMapWrite(map,"nine" ,DFLT,&n,sizeof(int));
    
    int *p;
    p = mMapRead(map,"zero" );if(p!=NULL)printf("zero = %d\n",*p);
    p = mMapRead(map,"one"  );if(p!=NULL)printf("one  = %d\n",*p); 
    p = mMapRead(map,"two"  );if(p!=NULL)printf("two  = %d\n",*p); 
    p = mMapRead(map,"three");if(p!=NULL)printf("three= %d\n",*p);
    p = mMapRead(map,"four" );if(p!=NULL)printf("four = %d\n",*p);
    p = mMapRead(map,"five" );if(p!=NULL)printf("five = %d\n",*p);
    p = mMapRead(map,"six"  );if(p!=NULL)printf("six  = %d\n",*p);
    p = mMapRead(map,"seven");if(p!=NULL)printf("seven= %d\n",*p);
    p = mMapRead(map,"eight");if(p!=NULL)printf("eight= %d\n",*p);
    p = mMapRead(map,"nine" );if(p!=NULL)printf("nine = %d\n",*p);
    p = mMapRead(map,"ten"  );if(p!=NULL)printf("ten  = %d\n",*p);
    
    mChainRelease(map);
    return 0;
}
```

此例是往映射里写入了十个键值对，然后再分别读出并打印，程序运行结果为：

```
zero = 0
one  = 1
two  = 2
three= 3
four = 4
five = 5
six  = 6
seven= 7
eight= 8
nine = 9
```

注意：在写入的时候并没有写入key为“ten”的值，因此，在读“ten”所对应的值时，将返回NULL。

#### 示例二

key为整数，value为字符串。

```c
int main()
{
    MChain *map = mChainCreate();
    
    int n;
    n=0; mMapWrite(map,&n,sizeof(int),"zero" ,DFLT);
    n=1; mMapWrite(map,&n,sizeof(int),"one"  ,DFLT);
    n=2; mMapWrite(map,&n,sizeof(int),"two"  ,DFLT);
    n=3; mMapWrite(map,&n,sizeof(int),"three",DFLT);
    n=4; mMapWrite(map,&n,sizeof(int),"four" ,DFLT);
    n=5; mMapWrite(map,&n,sizeof(int),"five" ,DFLT);
    n=6; mMapWrite(map,&n,sizeof(int),"six"  ,DFLT);
    n=7; mMapWrite(map,&n,sizeof(int),"seven",DFLT);
    n=8; mMapWrite(map,&n,sizeof(int),"eight",DFLT);
    n=9; mMapWrite(map,&n,sizeof(int),"nine" ,DFLT);
    
    char *p;
    n=0; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=1; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p); 
    n=2; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p); 
    n=3; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=4; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=5; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=6; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=7; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=8; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=9; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    n=10;p = mMapRead(map,&n,sizeof(int),NULL,DFLT);printf("%d = %s\n",n,p);
    
    mChainRelease(map);
    return 0;
}
```

运行结果为：

```
0 = zero  
1 = one   
2 = two   
3 = three 
4 = four  
5 = five  
6 = six   
7 = seven 
8 = eight 
9 = nine  
10 = (null)
```

注意：在写入的时候并没有写入key为10的值，因此，在读10所对应的值时，将返回NULL。

#### 示例三

使用Morn的map，其key和value可以是任意类型，比如整数、浮点数、指针、字符串、数组、结构体等等，且多种数据类型可混合使用。如下例：

```c
int main()
{
    MChain *map = mChainCreate();

    //key为指针
    int *a=NULL;int idx=1;
    mMapWrite(map,&a,sizeof(int *),&idx,sizeof(int));

    //key为整数
    int b=1;idx=2;
    mMapWrite(map,&b,sizeof(int),&idx,sizeof(int));

    //key为浮点
    float c=2;idx=3;
    mMapWrite(map,&c,sizeof(float),&idx,sizeof(int));

    //key为数组
    double d[3]={3,4,5};idx=4;
    mMapWrite(map,d,(3*sizeof(double)),&idx,sizeof(int));

    //key为字符串
    char *e="test string";idx=5;
    mMapWrite(map,e,strlen(e),&idx,sizeof(int));

    //key为结构体
    struct {int d1;float d2;double d3[3];} f;idx=6;
    mMapWrite(map,&f,sizeof(f),&idx,sizeof(int));
    
    printf("f idx=%d\n",*(int *)mMapRead(map,&f,sizeof(f),NULL,DFLT));
    printf("e idx=%d\n",*(int *)mMapRead(map, e,strlen(e),NULL,DFLT));
    printf("d idx=%d\n",*(int *)mMapRead(map, d,sizeof(d),NULL,DFLT));
    printf("c idx=%d\n",*(int *)mMapRead(map,&c,sizeof(c),NULL,DFLT));
    printf("b idx=%d\n",*(int *)mMapRead(map,&b,sizeof(b),NULL,DFLT));
    printf("a idx=%d\n",*(int *)mMapRead(map,&a,sizeof(a),NULL,DFLT));

    mChainRelease(map);
    return 0;
}
```

运行结果为：

```
f idx=6
e idx=5
d idx=4
c idx=3
b idx=2
a idx=1
```

虽然Morn的map支持多种数据类型混合使用，但在实际应用中并**不建议key值混合使用多种类型**。如下例，将会出现错误：

```c
int main()
{
    MChain *map = mChainCreate();
    
    char *a="abcd";int idx=1;
    mMapWrite(map,a,strlen(a),&idx,sizeof(int));
    
    unsigned char b[4]={97,98,99,100};idx=2;
    mMapWrite(map,b,sizeof(b),&idx,sizeof(int));
    
    int c = 1684234849;idx=3;
    mMapWrite(map,&c,sizeof(c),&idx,sizeof(int));
    
    float d = 16777999408082104000000.0f;idx=4;
    mMapWrite(map,&d,sizeof(d),&idx,sizeof(int));
    
    printf("a idx=%d\n",*(int *)mMapRead(map, a,strlen(a),NULL,DFLT));
    printf("b idx=%d\n",*(int *)mMapRead(map, b,sizeof(b),NULL,DFLT));
    printf("c idx=%d\n",*(int *)mMapRead(map,&c,sizeof(c),NULL,DFLT));
    printf("d idx=%d\n",*(int *)mMapRead(map,&d,sizeof(d),NULL,DFLT));
    
    mChainRelease(map);
    return 0;
}
```

其运行结果为：

```c
a idx=4
b idx=4
c idx=4
d idx=4
```

可见，结果与预想的不同，这是因为程序中，虽然a、b、c、d具有不同的数据类型，但是在内存中，它们是完全一样的，其长度皆为4字节，其内存皆为0x64636261，四次写入传入`mMapWrite`的参数完全相同，因此后一次写入将覆盖前一次写入。



### 性能

这里主要比较Morn中映射和C++ STL中的map，测试内容包括写入、读出和删除。

以下测试中，使用以下程序生成随机的整数和字符串：

```c
struct TestData
{
    char data_s[32];
    int data_i;
};
void data_gerenate(struct TestData *data,int number)
{
    int i,j;
    for(i=0;i<number;i++)
    {
        for(j=0;j<mRand(10,31);j++){data[i].data_s[j] = mRand('a','z'+1);}data[i].data_s[j]=0;
        data[i].data_i = mRand();
    }
}
```

#### 测试一

key为字符串，value为整数：

```c
mTimerBegin("STL map key string 10000*100");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_map[data[i].data_s]=data[i].data_i;
    for(int i=0;i<100;i++) int data_i = stl_map.find(data[i].data_s)->second;
    for(int i=0;i<100;i++) stl_map.erase(data[i].data_s);
}
mTimerEnd("STL map key string 10000*100");

mTimerBegin("Morn map key string 10000*100");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
    for(int i=0;i<100;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
    for(int i=0;i<100;i++) mMapNodeDelete(morn_map,data[i].data_s);
}
mTimerEnd("Morn map key string 10000*100");
```

测试了：①读写100个键值对，读写10000次，②读写1000个键值对，读写1000次，③读写10000个键值对，读写100次。其测试结果如下：

![映射1](映射1.PNG)

可见，**键值为字符串时，Morn全面优于stl**。

#### 测试二

key为随机整数，value为字符串：

```c
mTimerBegin("STL map key int 10000*100");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_map[data[i].data_i]=data[i].data_s;
    for(int i=0;i<100;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
    for(int i=0;i<100;i++) stl_map.erase(data[i].data_i);
}
mTimerEnd("STL map key int 10000*100");

mTimerBegin("Morn map key int 10000*100");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
    for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,DFLT);
    for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
}
mTimerEnd("Morn map key int 10000*100");
```

测试了：①读写100个键值对，读写10000次，②读写1000个键值对，读写1000次，③读写10000个键值对，读写100次。其测试结果如下：

![映射2](映射2.PNG)

可见，对于100对键值对Morn落后于stl，1000对键值对Morn优于stl，10000对键值对差别不大。**键值为整数时，Morn与stl互有胜负**。

#### 测试三

key为有序整数，value为字符串：

```c
mTimerBegin("STL map key int 10000*100");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_map[i]=data[i].data_s;
    for(int i=0;i<100;i++) std::string data_s = stl_map.find(i)->second;
    for(int i=0;i<100;i++) stl_map.erase(i);
}
mTimerEnd("STL map key int 10000*100");

mTimerBegin("Morn map key int 10000*100");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
    for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,DFLT);
    for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
}
mTimerEnd("Morn map key int 10000*100");
```

测试了：①读写100个键值对，读写10000次，②读写1000个键值对，读写1000次，③读写10000个键值对，读写100次。其测试结果如下：

![映射3](映射3.PNG)

可见，**对于有序数据stl优于Morn**。

#### 测试四

大数据量测试，key为字符串，value为整数：

```c
mTimerBegin("STL map write");
for(int i=0;i<number;i++) stl_map[data[i].data_s]=data[i].data_i;
mTimerEnd("STL map write");

mTimerBegin("Morn map write");
for(int i=0;i<number;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
mTimerEnd("Morn map write");

mTimerBegin("STL map read");
for(int i=0;i<number;i++) int data_i = stl_map.find(data[i].data_s)->second;
mTimerEnd("STL map read");

mTimerBegin("Morn map read");
for(int i=0;i<number;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
mTimerEnd("Morn map read");

mTimerBegin("STL map erase");
for(int i=0;i<number;i++) stl_map.erase(data[i].data_s);
mTimerEnd("STL map erase");

mTimerBegin("Morn map delete");
for(int i=0;i<number;i++) mMapNodeDelete(morn_map,data[i].data_s);
mTimerEnd("Morn map delete");

```

分别测量：①十万对键值对，②百万对键值对。其测试结果如下：

![映射4](映射4.PNG)

可见，**键值为字符串，在数据量小于等于十万时Morn仍然保持优势，但数据量百万时Morn落后于stl**。

#### 测试五

大数据量测试，key为无序整数，value为字符串：

```c
mTimerBegin("STL map write");
for(int i=0;i<number;i++) stl_map[data[i].data_i]=data[i].data_s;
mTimerEnd("STL map write");

mTimerBegin("Morn map write");
for(int i=0;i<number;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,DFLT);
mTimerEnd("Morn map write");

mTimerBegin("STL map read");
for(int i=0;i<number;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
mTimerEnd("STL map read");

mTimerBegin("Morn map read");
for(int i=0;i<number;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,DFLT);
mTimerEnd("Morn map read");

mTimerBegin("STL map erase");
for(int i=0;i<number;i++) stl_map.erase(data[i].data_i);
mTimerEnd("STL map erase");

mTimerBegin("Morn map delete");
for(int i=0;i<number;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
mTimerEnd("Morn map delete");
```

分别测量：①十万对键值对，②百万对键值对。其测试结果如下：

![映射5](映射5.PNG)

可见，**若键值为整数，大数据量时，Morn全面落后于stl。**

以上测试可见，①对于键值为字符串、数组、结构体等数据类型时，有很好的性能表现。②对于键值为整数、浮点数时，Morn的性能与stl相当，③Morn不适应于大数据量的情景（stl也不适用大数据量类型，但表现优于Morn），④Morn不适合键值本身有序的情况（键值本身有序的话其实不需要使用map）。











