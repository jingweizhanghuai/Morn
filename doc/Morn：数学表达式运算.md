## Morn：数学表达式运算

写这个，主要是因为我不喜欢那个图形界面的运算器，我喜欢命令行。

运算不是重点，重点是表达式解析。



### 接口

#### 表达式运算

```c
double mCaculate(char *str);
```

str即为输入的表达式字符串。返回值是算的的结果。

例如：

```c
int main()
{
    printf("1+2*3/4            = %f\n",mCaculate("1+2*3/4"));
    printf("pi                 = %f\n",mCaculate("pi"));
    printf("sin(pi/2)          = %f\n",mCaculate("sin(pi/2)"));
    printf("ln(e)              = %f\n",mCaculate("ln(e)"));
    printf("sqrt(3^2+pow(4,2)) = %f\n",mCaculate("sqrt(3^2+pow(4,2))"));
    return 0;
}
```

其运行结果为：

```
1+2*3/4            = 2.500000
pi                 = 3.141593
sin(pi/2)          = 1.000000
ln(e)              = 1.000000
sqrt(3^2+pow(4,2)) = 5.000000
```

表达式中原生支持的函数包括：取绝对值abs(x)，取小值min(x1,x2)，取大值max(x1,x2)，向上取整ceil(x)，向下取整floor(x)，四舍五入取整round(x)，开方运算sqrt(x)，自然指数exp(x)（相当于`e^x`），指数运算pow(x1,x2)（相当于`x1^x2`），自然对数ln(x)（相当于`log(e,x)`），以10为底的对数log10(x)（相当于`log(10,x)`），对数运算log(x1,x2)，正弦函数sin(x)，余弦函数cos(x)，正切函数tan(x)，反正弦函数asin(x)，反余弦函数acos(x)，反正切函数atan(x)。

正弦函数sin(x)，余弦函数cos(x)，正切函数tan(x)的输入都是弧度制（不是角度值）。反正弦函数asin(x)，反余弦函数acos(x)，反正切函数atan(x)所得到的结果也是弧度值（不是角度值）。



#### 自定义函数

```c
void mCaculateFunction(void *Func);
```

数学表达式中除了支持上述函数外，还支持用户自定义函数。

用户自定义函数应符合以下形式：

```c
double myfunc(double a,double b,...);
```

即：①必须至少有一个输入参数，至多有8个输入参数，②必须有返回值，③参数和返回值只能是double类型。

对于用户自定义函数，应首先使用`mCaculateFunction`接口对函数进行注册。下例演示了函数的用法。

例如：用户自定义了由弧度转角度运算函数`r2a`和由角度转弧度的运算函数`a2r`。其函数实现如下：

```c
double r2a(double r)
{
    return r*180.0/MORN_PI;
}

double a2r(double a)
{
    return a*MORN_PI/180.0;
}
```

在数学表达式中使用此两个函数时，应先对此二函数进行注册，之后就可使用`mCaculate`函数中获得支持。如下：

```c
int main()
{   
    mCaculateFunction(a2r);
    printf("sin(a2r(30)) = %f\n",mCaculate("sin(a2r(30))"));
    
    mCaculateFunction(r2a);
    printf("r2a(atan(1)) = %f\n",mCaculate("r2a(atan(1))"));
    
    return 0;
}
```

此函数运行结果如下：

```
sin(a2r(30)) = 0.500000
r2a(atan(1)) = 45.000000
```

再如：用户自定义了三个数求中值的函数`mid`，其实现如下：

```c
double mid(double a,double b,double c)
{
    if((a>b)==(c>=a)) return a;
    if((a>b)==(b>=c)) return b;
    return c;
}
```

使用此自定义函数如下：

```c
int main()
{
    mCaculateFunction(mid);
    char *str = "mid(5,1,2)";
    printf("%s = %f\n",str,mCaculate(str));

    return 0;
}
```

其运行结果为：

```
mid(5,1,2) = 2.000000
```



### 规则

* 优先级

  * 括号的优先级最高，然后是指数运算运算（^），再次是乘除和取余（*、/、%），优先级最低的是加减（+、-）。因此例如`-3^2`，将被解释为”负的三的平方“，其运算结果为-9（而不是9）。
  * 对于连续的指数运算，从右向左运算，例如`2^3^2`，实际相当于`2^(3^2)`，结果为512。
  * 对于除乘方外相同的优先级的运算，从左向右运算。
  * 如果你记不住这些优先级，那就加括号。

* “%”在表达式中是取余的意思，而不是百分数的意思，因此：`5%+2`，将被解释为“五对正二取余”，其运算结果为1（而不是2.05）。

* 空格在表达式中没有任何作用，因此“一万”你可以写成`10000`或`10 000`，但是不能写成`10,000`。

* 数学函数名、pi（圆周率）、e（自然底数）都不区分大小写。

* 数学函数不能省略括号，例如`ln5`这种写法是不支持的，必须写成`ln(5)`。

* 乘号\*不能省略，例如`2pi`这种写法是不支持的，必须写成`2*pi`。

  

### 工具

在[../tool](../tool)文件夹里有一个命令行计算器caculate。其使用很简单，例如

```
>1+2*3/4
result is 2.500000
>
>pi
result is 3.141593
>
>sin(pi/2)
result is 1.000000
>
>ln(e)
result is 1.000000
>
>sqrt(3^2+pow(4,2))
result is 5.000000
>
>5/0
result is 1.#INF00
>
>exit
```

退出命令行运算，输入exit。





