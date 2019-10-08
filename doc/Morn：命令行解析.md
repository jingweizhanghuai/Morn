## Morn：命令行解析

../tool文件夹里的那些工具，在解析命令的时候，用的就是这个函数。



### 接口

```c
char *mStringArgument(int argc,char **argv,const char *flag,int *ok)；
```

argc、argv就是main函数传入的参数，argc是参数个数，argv是参数内容（从argv[1]开始）。

flag是要寻找的参数标志，这里的标志必须以字符“-”打头。

ok是输出的值，如果找到了相应的参数，ok就会被置1，否则会被置0。

返回值就是找到的命令参数，它是字符串格式，通常，如果参数是数值的话，还需要和`atoi`，`atof`函数一起使用。如果没有找到，返回值是NULL。



### 示例

以[../tool/imageformat.c](../tool/imageformat.c)为例，此文件所完成的功能另见文档[Morn：图像加载和保存](Morn：图像加载和保存)。

```c
int main(int argc,char *argv[])
{
    ...
    char *file_in = mStringArgument(argc,argv,"i" ,NULL);
    char *file_out= mStringArgument(argc,argv,"o" ,NULL);
    char *dir_in  = mStringArgument(argc,argv,"di",NULL);
    char *dir_out = mStringArgument(argc,argv,"do",NULL);
    char *type_in = mStringArgument(argc,argv,"ti",NULL);
    char *type_out= mStringArgument(argc,argv,"to",NULL);
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

