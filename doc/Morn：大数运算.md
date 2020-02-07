## Morn：大数运算

大数运算，顾名思义就是很大的数的四则运算。

C语言里的整数包括8位、16位、32位、64位，对于64位整数，所表示的数值取值范围是-2^63至2^63-1。对于比超出这个取值范围的数就需要使用大数运算了。



### 定义

```c
typedef struct MLInt
{
	uint32_t data[32];
	char sign;
	char len;
}MLInt;
```

这里data用于存储数据，这里有32个32位的整数用于存储，因此，最大可以存储1024位的数据，其取值范围是-2^1024+1至2^1024-1，此取值范围可以满足大部分大数运算的需求。

sign是数据的符号，sign==0表示大数是正数，sign==1表示大数是负数。

len是数据的位数，即表示使用了32个data中的几个，取值在0到32之间。



### 接口

#### MLInt与64位整数的相互转换

```c
void mS64ToLInt(int64_t in,MLInt *a);
int64_t mLIntToS64(MLInt *a);
```

此二函数用于Morn大数与64位整数（int64_t）之间的转换。

mLIntToS64要求输入的大数取值必须在-2^63至2^63-1之间（a->len<=2)，否则出错。



#### MLInt与字符串的相互转换

```c
void mLIntToString(MLInt *a,char *str);
void mStringToLInt(char *str,MLInt *a);
```

此二函数用于Morn大数与字符串之间的转换。

例如：

```c
char *num ="32101234567898765432101234567898765432101234567898765432101234567";
MLInt a;
mStringToLInt(num,&a);
char str[100];
mLIntToString(&a,str);
printf("a is %s\n",str);
```



#### MLInt比较

```c
int mLIntCompare(MLInt *a,MLInt *b);
```

两个Morn大数比较大小。如果a>b，则返回1，如果a<b，则返回-1，如果a==b，则返回0。



#### MLInt与32位有符号整数的运算

```c
void mLIntAddS32(MLInt *a,int b,MLInt *c);
void mLIntSubS32(MLInt *a,int b,MLInt *c);
void mLIntMulS32(MLInt *a,int b,MLInt *c);
void mLIntDivS32(MLInt *a,int b,MLInt *c,int *remainder);
```

这是Morn大数a与int整数b的加减乘除，运算结果保存为c，c如果传入NULL，则结果保存入a。

对于除法，其余数保存为remainder，若remainder传入为NULL，则不计算余数。



#### MLInt与32位无符号整数的运算

```c
void mLIntAddU32(MLInt *a,uint32_t b,MLInt *c);
void mLIntSubU32(MLInt *a,uint32_t b,MLInt *c);
void mLIntMulU32(MLInt *a,uint32_t b,MLInt *c);
void mLIntDivU32(MLInt *a,uint32_t b,MLInt *c,int32_t *remainder);
```

这是Morn大数a与uint32_t整数b的加减乘除，运算结果保存为c，c如果传入NULL，则结果保存入a。

对于除法，其余数保存为remainder，若remainder传入为NULL，则不计算余数。



#### MLInt运算

```c
void mLIntAdd(MLInt *a,MLInt *b,MLInt *c);
void mLIntSub(MLInt *a,MLInt *b,MLInt *c);
void mLIntMul(MLInt *a,MLInt *b,MLInt *c);
void mLIntDiv(MLInt *a,MLInt *b,MLInt *c,MLInt *remainder);
```

两个Morn大数之间的加减乘除，运算结果保存为c，c如果传入NULL，则结果保存入a。

对于除法，其余数保存为remainder，若remainder传入为NULL，则不计算余数。



### 示例

```c
int main1()
{
    char str[100];
    MLInt a,b,c,d,e,f,g;
    mS64ToLInt(12345678987654,&a);mLIntToString(&a,str);printf("a is %s\n",str);
    char *num ="32101234567898765432101234567898765432101234567";
    mStringToLInt(num,&b);       mLIntToString(&b,str);printf("b is %s\n",str);
    mLIntMulU32(&a,8987654,&c);  mLIntToString(&c,str);printf("c is %s\n",str);
    
    mLIntMul(&b,&c,&d);          mLIntToString(&d,str);printf("d is %s\n",str);
    mLIntAdd(&d,&a,&e);          mLIntToString(&e,str);printf("e is %s\n",str);
    
    mLIntDiv(&e,&c,&f,&g);       mLIntToString(&f,str);printf("f is %s\n",str);
                                 mLIntToString(&g,str);printf("g is %s\n",str);
                                 
    int flag1=mLIntCompare(&f,&b);
    printf("flag1 is %d\n",flag1);

    int flag2=mLIntCompare(&g,&a);
    printf("flag2 is %d\n",flag2);
    
    return 0;
}
```

此例中，大数a由64位整数转换得到，b由字符串转换得到，c由b与整数相乘得到，d由b与c相乘得到，e由d与a相加得到，f是e与c的商，g是e与c相除的余数。因为a小于c，所以计算结果f应该等于b，g应该等于a。