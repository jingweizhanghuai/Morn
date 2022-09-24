## Map

Map is organized by lots of key-value nodes. You can find the value by the key. 

Morn Provides some function to operate the key-value nodes in the map, such as increase, write, read and delete.

Key and value can be of any type (which can be integer float string array pointer or stuct).

The Morn map  has the following characteristics:

* can be used for any types
* high-performance
* simple API
*  lightweight  algorithm(with about 250 lines codes)

The source code is [morn_map.c](../src/util/morn_map.c)



### API

#### Create Map

```c
MMap *mMapCreate();
```

`mMapCreate` is a must with before using Morn map.



#### Release Map

```c
void mMapRelease(MMap *map);
```

`mMapRelease` is a must after using Morn map.



#### Write to Map

``` c
void *mMapWrite(MMap *map,const void *key,const void *value);
void *mMapWrite(MMap *map,const void *key,int key_size,const void *value,int value_size);
```

`key` is a pointer to any types. `key_size` is the bytes of `key`. when the key type is a string, `key_size` can be set DFLT.

`value` is  the corresponding value of `key`, value_size is the bytes of `value`, when value is a string, the `value_size` can also be set DFLT.

The return is a pointer to value of map node written in the memory.

Note: for one key-value node, the key must be unique. If two key-value nodes have the same key, the last one will override the previous one.



#### Read from Map

```c
void *mMapRead(MMap *map,const char *key);
void *mMapRead(MMap *map,const char *key,char *value);
void *mMapRead(MMap *map,const void *key,int key_size,void *value,int *value_size);
```

when the key type is a string, `key_size` can be set as NULL.

`value` is a memory pointer to save the data, when  the data be read, it will be copied to this memory address. if copy is not necessary, `value` can be set NULL.

`value_size` is used to save the bytes of `value`, if it is necessary.

The return is a pointer to value of map node in map memory.



#### Delete Map Node

```c
void mMapDelete(MMap *map,const char *key);
void mMapDelete(MMap *map,const void *key,int key_size);
```

You can delete a key-value node from the `map` using `mMapDelete`. 

`key_size` can be set DFLT when the `key` is a string.



#### Map Node  Traversaling

```c
void mMapNodeOperate(MChain *map,void (*func)(const void *,const int,void *,int,void *),void *para);
```

You can do some operation for every map node using `mMapNodeOperate`.

`func` is a pointer to operate function, the form of this function must be: 

```c
void func(const void *key,const int key_size,void *value,int value_size,void *para);
```

`para` is the parameter of `func`.

Note: map node is ordered by key. So when operate the key-value node, the key must not be changed.



### Example

Example source code is [test_map2.cpp](../test/test_map2.cpp)

#### Example 1

The key is string and the value is integer.

```c
int main()
{
    MMap *map = mMapCreate();
    
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
    
    mMapRelease(map);
    return 0;
}
```

In this example we write 10 key-value nodes to the map, and then read print it. Output is:

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

Note: Because we did not write key "ten" to the map, when we read it, the return is NULL.



#### Example 2

The key is integer and the value is string.

```c
int main()
{
    MMap *map = mMapCreate();
    
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
    n=0; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=1; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p); 
    n=2; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p); 
    n=3; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=4; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=5; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=6; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=7; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=8; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=9; p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    n=10;p = mMapRead(map,&n,sizeof(int));printf("%d = %s\n",n,p);
    
    mMapRelease(map);
    return 0;
}
```

Output is:

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

Note: Since we did not write key 10 to the map, when we read it, the return is NULL.



#### Example 3

This example shows that: in Morn map we can use any type of keys and values, such as integer float pointer array string and struct etc. And we can mix use any type in map.

```c
int main()
{
    MMap *map = mMapCreate();

    //key is pointer
    int *a=NULL;int idx=1;
    mMapWrite(map,&a,sizeof(int *),&idx,sizeof(int));

    //key is integer
    int b=1;idx=2;
    mMapWrite(map,&b,sizeof(int),&idx,sizeof(int));

    //key is float
    float c=2;idx=3;
    mMapWrite(map,&c,sizeof(float),&idx,sizeof(int));

    //key is array
    double d[4]={1,9,4,9};idx=4;
    mMapWrite(map,d,4*sizeof(double),&idx,sizeof(int));

    //key is string
    char *e="test string";idx=5;
    mMapWrite(map,e,strlen(e),&idx,sizeof(int));

    //key is struct
    struct {int d1;float d2;double d3[3];} f;idx=6;
    mMapWrite(map,&f,sizeof(f),&idx,sizeof(int));
    
    printf("f idx=%d\n",*(int *)mMapRead(map,&f,sizeof(f),NULL,DFLT));
    printf("e idx=%d\n",*(int *)mMapRead(map, e,strlen(e),NULL,DFLT));
    printf("d idx=%d\n",*(int *)mMapRead(map, d,sizeof(d),NULL,DFLT));
    printf("c idx=%d\n",*(int *)mMapRead(map,&c,sizeof(c),NULL,DFLT));
    printf("b idx=%d\n",*(int *)mMapRead(map,&b,sizeof(b),NULL,DFLT));
    printf("a idx=%d\n",*(int *)mMapRead(map,&a,sizeof(a),NULL,DFLT));

    mMapRelease(map);
    return 0;
}
```

Output is:

```
f idx=6
e idx=5
d idx=4
c idx=3
b idx=2
a idx=1
```

Note: **mixing types of key is not recommended in practice**. In the following example, an error will occur:   

```c
int main()
{
    MMap *map = mMapCreate();
    
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
    
    mMapRelease(map);
    return 0;
}
```

Output is:

```c
a idx=4
b idx=4
c idx=4
d idx=4
```

Although a/b/c/d has different types, they are the same in memory, which is 0x64636261 used 4 bytes. So all four  `mMapWrite` are exactly use the same parameters, and it will be overwrite one by one.



### Performance

Here we main compared Morn map using C and STL map/unordered_map using C++. usually the std::map is a red-black tree, and std::unordered_map is a hash-table. 

Tests include writing, reading, and deleting.

 Complete test file is [test_map2.cpp](https://github.com/jingweizhanghuai/Morn/blob/master/test/test_map2.cpp).

We use the following command to compile the program:

```
g++ -O2 -DNDEBUG test_map2.cpp -lmorn -o test_map2.exe
```

The following code is used to generate random integers and random strings for the test:

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
        mRandString(data[i].data_s,15,31);
        data[i].data_i = mRand();
    }
}
```

#### Test 1

Test with the key is string, and the value is integer:

```c
printf("\n10000 times test with 100 node for key is string and value is integer:\n");
mTimerBegin("STL map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_map[data[i].data_s]=data[i].data_i;
    for(int i=0;i<100;i++) int data_i = stl_map.find(data[i].data_s)->second;
    for(int i=0;i<100;i++) stl_map.erase(data[i].data_s);
}
mTimerEnd("STL map");

mTimerBegin("STL unordered_map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
    for(int i=0;i<100;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
    for(int i=0;i<100;i++) stl_unorderedmap.erase(data[i].data_s);
}
mTimerEnd("STL unordered_map");

mTimerBegin("Morn map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
    for(int i=0;i<100;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
    for(int i=0;i<100;i++) mMapNodeDelete(morn_map,data[i].data_s);
}
mTimerEnd("Morn map");
```

We test: 1.  read and write 100 key-value nodes with 10000 times, 2. read and write 1000 key-value nodes with 1000 times, 3. read and write 10000 key-value nodes with 100 times. The Output is:   

[![c0HZGD.png](https://z3.ax1x.com/2021/04/12/c0HZGD.png)](https://imgtu.com/i/c0HZGD)

Thus: **when key is string Morn is faster then std::map and std::unorderd_map**。

#### Test 2

Test with the key is integer, and the value is string:

```c
printf("\n10000 times test with 100 node for key is integer and value is string:\n");
mTimerBegin("STL map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_map[data[i].data_i]=data[i].data_s;
    for(int i=0;i<100;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
    for(int i=0;i<100;i++) stl_map.erase(data[i].data_i);
}
mTimerEnd("STL map");

mTimerBegin("STL unordered_map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
    for(int i=0;i<100;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
    for(int i=0;i<100;i++) stl_unorderedmap.erase(data[i].data_i);
}
mTimerEnd("STL unordered_map");

mTimerBegin("Morn map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
    for(int i=0;i<100;i++) char *data_s=(char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int));
    for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
}
mTimerEnd("Morn map");
```

We test: 1.  read and write 100 key-value nodes with 10000 times, 2. read and write 1000 key-value nodes with 1000 times, 3. read and write 10000 key-value nodes with 100 times. The Output is:   

[![c0HVPO.png](https://z3.ax1x.com/2021/04/12/c0HVPO.png)](https://imgtu.com/i/c0HVPO)

Thus: **when key is integer, Morn is faster than std::map and std::unorderd_map**.

#### Test 3

Test with the key is ordered integer, and the value is string:

```c
printf("\n10000 times test with 100 node for key is orderly integer and value is string:\n");
mTimerBegin("STL map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_map[i]=data[i].data_s;
    for(int i=0;i<100;i++) std::string data_s = stl_map.find(i)->second;
    for(int i=0;i<100;i++) stl_map.erase(i);
}
mTimerEnd("STL map");

mTimerBegin("STL unordered_map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) stl_unorderedmap[i]=data[i].data_s;
    for(int i=0;i<100;i++) std::string data_s = stl_unorderedmap.find(i)->second;
    for(int i=0;i<100;i++) stl_unorderedmap.erase(i);
}
mTimerEnd("STL unordered_map");

mTimerBegin("Morn map");
for(int n=0;n<10000;n++)
{
    for(int i=0;i<100;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
    for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int));
    for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
}
mTimerEnd("Morn map");
```

We test: 1.  read and write 100 key-value nodes with 10000 times, 2. read and write 1000 key-value nodes with 1000 times, 3. read and write 10000 key-value nodes with 100 times. The Output is:   

[![c0HAIK.png](https://z3.ax1x.com/2021/04/12/c0HAIK.png)](https://imgtu.com/i/c0HAIK)

Thus: **when key is integer, Morn is faster than std::map and std::unorderd_map**.

#### Test 4

Test for large amount of data with key is string and value is integer:

```c
mTimerBegin("STL map write");
for(int i=0;i<number;i++) stl_map[data[i].data_s]=data[i].data_i;
mTimerEnd("STL map write");

mTimerBegin("STL unordered_map write");
for(int i=0;i<number;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
mTimerEnd("STL unordered_map write");

mTimerBegin("Morn map write");
for(int i=0;i<number;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
mTimerEnd("Morn map write");

mTimerBegin("STL map read");
for(int i=0;i<number;i++) int data_i = stl_map.find(data[i].data_s)->second;
mTimerEnd("STL map read");

mTimerBegin("STL unordered_map read");
for(int i=0;i<number;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
mTimerEnd("STL unordered_map read");

mTimerBegin("Morn map read");
for(int i=0;i<number;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
mTimerEnd("Morn map read");

mTimerBegin("STL map erase");
for(int i=0;i<number;i++) stl_map.erase(data[i].data_s);
mTimerEnd("STL map erase");

mTimerBegin("STL unordered_map erase");
for(int i=0;i<number;i++) stl_unorderedmap.erase(data[i].data_s);
mTimerEnd("STL unordered_map erase");

mTimerBegin("Morn map delete");
for(int i=0;i<number;i++) mMapNodeDelete(morn_map,data[i].data_s);
mTimerEnd("Morn map delete");
```

We test 100000 key-value nodes and 1000000 key-value nodes, Output is:

[![c0HeRe.png](https://z3.ax1x.com/2021/04/12/c0HeRe.png)](https://imgtu.com/i/c0HeRe)

It can be seen that: **for large amount of data, when key is string, Morn is faster than std::map. But if there is millions of nodes, Morn will fall behind of std::unorderd_map**. 

#### Test 5

Test for large amount of data with key is integer and value is string:

```c
mTimerBegin("STL map write");
for(int i=0;i<number;i++) stl_map[data[i].data_i]=data[i].data_s;
mTimerEnd("STL map write");

mTimerBegin("STL unordered_map write");
for(int i=0;i<number;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
mTimerEnd("STL unordered_map write");

mTimerBegin("Morn map write");
for(int i=0;i<number;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,DFLT);
mTimerEnd("Morn map write");

mTimerBegin("STL map read");
for(int i=0;i<number;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
mTimerEnd("STL map read");

mTimerBegin("STL unordered_map read");
for(int i=0;i<number;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
mTimerEnd("STL unordered_map read");

mTimerBegin("Morn map read");
for(int i=0;i<number;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,DFLT);
mTimerEnd("Morn map read");

mTimerBegin("STL map erase");
for(int i=0;i<number;i++) stl_map.erase(data[i].data_i);
mTimerEnd("STL map erase");

mTimerBegin("STL unordered_map erase");
for(int i=0;i<number;i++) stl_unorderedmap.erase(data[i].data_i);
mTimerEnd("STL unordered_map erase");

mTimerBegin("Morn map delete");
for(int i=0;i<number;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
mTimerEnd("Morn map delete");
```

We test 100000 key-value nodes and 1000000 key-value nodes, Output is:

[![c0Hka6.png](https://z3.ax1x.com/2021/04/12/c0Hka6.png)](https://imgtu.com/i/c0Hka6)

It can be seen that: **for large amount of data, when key is integer, std::unorderd_map is faster than Morn map. And if there is millions of nodes, Morn will fall behind of std::map**.

The above tests show that: 1. Morn has extreme performance when the data amount is not too large(<100000). 2. Morn has high performance when the key type is string, struct, array and other complex types.













