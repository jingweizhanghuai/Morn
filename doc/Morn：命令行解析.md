## Morn：命令行解析

../tool文件夹里的那些工具，在解析命令的时候，用的就是这个。



### 接口

```c
char *mStringArgument(int argc,char **argv,const char *flag);
char *mStringArgument(int argc,char **argv,const char *flag,const char *format,...);
```

argc、argv就是main函数传入的参数，argc是参数个数，argv是参数内容（从argv[1]开始）。

flag是要寻找的参数标志，这里的标志必须以字符“-”打头，flag可以是一个字符，也可以是多个字符。

返回值就是找到的命令参数，它是一个字符串。

format用以指定参数值的格式。其用法与标准库`scanf`系列函数相同。例如：

```c
// test.exe -a=5
// 以下两种写法是等效的
int a;
char *str=atoi(mStringArgument(argc,argv,"a"));if(str!=NULL) a=atoi(str);//方式1
mStringArgument(argc,argv,"a","%d",&a);						    	     //方式2

// test.exe -m a=5
// 以下两种写法是等效的
char name[8];int value;
char *str=atoi(mStringArgument(argc,argv,"m"));if(str!=NULL) sscanf("%[^=]=%d",name,&value);   //方式1
mStringArgument(argc,argv,"m","%[^=]=%d",name,&value);   //方式二
```

如果没有找到，返回值是NULL。

例如：

```c
int main(int argc,char **argv)
{
    char *para = mStringArgument(argc,argv,"a");
    if(para!=NULL) printf("para is %s\n",para);
    else printf("no para\n");
}
```

以上面的程序为例，可见`mStringArgument`支持以下形式的参数：

```
$ test.exe -a12345
para is 12345

$ test.exe -a 12345
para is 12345

$ test.exe -a=12345
para is 12345

$ test.exe -a:12345 -b=6789
para is 12345

$ test.exe -a
para is ?
```

也即：以下三种形式的参数都是合法的，且三种方式等价：`-a12345`，`-a 12345`，`-a=12345`。

对于最后一种，虽有参数，但未指定值的情况，将返回一个字符串，其值为“?”。

对于以下两种情况：

```
$ test.exe
no para

$ test.exe -b=12345
no para
```

即①没有参数或②虽有参数但是与给定的标志不匹配（参数为b，指定为a），其返回值都为NULL。

**注意：返回值为NULL与返回一个指向"?"的指针是不同的含义**，前者是没有对应的flag，后者是有定义flag但未指定值。做这样的设计是因为很多时候并不是每个flag都需要指定值，例如`test.exe --help`:

```c
int main(int argc,char **argv)
{
    if(mStringArgument(argc,argv,"-help")!=NULL)
    {
        ...
    }
}
```



对于以下情况，应在程序设计时避免出现

```
$ test.exe -ab=12345
para is b=12345
```

或许开发者的本意是参数`ab`的值为`12345`，但是对于参数解析函数来说，不能区分①参数为`ab`，值为`12345`，②参数为`a`，值为`b=12345`，这两种情况。因此应避免使用，否则将可能出错。



对于多个参数有相同的标志位的，`mStringArgument`将循环读取各个参数。

例如：

```c
int main(int argc,char **argv)
{
    char *para = mStringArgument(argc,argv,"a");
    char *para0=para; 
    do{
        if(para!=NULL) printf("para is %s\n",para);
        else {printf("no para\n");break;}
        para = mStringArgument(argc,argv,"a");
    }while(para!=para0);
}
```

运行以上程序，会得到：

```c
$ test.exe -a=12 34 5
para is 12
para is 34
para is 5

$ test.exe -a12 -a34 -a5
para is 12
para is 34
para is 5

$ test.exe -a12 -a=34 -a:5
para is 12
para is 34
para is 5
    
$ test.exe -a12 34 -b 67 89 -a5
para is 12
para is 34
para is 5
```



### 示例

以[../tool/imageformat.c](../tool/imageformat.c)为例，此文件所完成的功能另见文档[Morn：图像加载和保存](Morn：图像加载和保存)。

```c
int main(int argc,char *argv[])
{
    ...
    char *file_in = mStringArgument(argc,argv,"i" );
    char *file_out= mStringArgument(argc,argv,"o" );
    char *dir_in  = mStringArgument(argc,argv,"di");
    char *dir_out = mStringArgument(argc,argv,"do");
    char *type_in = mStringArgument(argc,argv,"ti");
    char *type_out= mStringArgument(argc,argv,"to");
    ...
}
```

此例中，假设我们传入的是：

```
imageformat.exe -i ./test.jpg -o ./test.png
```

则经过解析后，file_in即为"./test.jpg"，file_out即为"./test.png"，其它dir_in、dir_out、type_in、type_out都会被置为NULL。

再如，假设我们传入的是：

```
imageformat.exe -di ./test/in/ -ti jpg -do ./test/out/ -to png
```

则经过解析后，dir_in即为"./test/in/"，dir_out即为"./test/out/"，type_in即为"jpg"，type_out即为"png"，file_in、file_out会被置为NULL。

