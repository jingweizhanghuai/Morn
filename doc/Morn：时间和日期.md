## Morn：时间和日期

几个时间和日期相关的小函数。



### 接口

#### 时间测量

```c
mTimerBegin()
mTimerEnd()
```

根据之前的[约定](./doc/Morn：约定.md)，此两个接口必须成对出现。

通常此接口用来测量程序耗时。时间测量的精度因操作系统而异，linux下精度为微秒，windows下精度为毫秒，常见的用法如下：

```c
mTimerBegin();
...
do something
...
mTimerEnd();
```

在`mTimerEnd()`将打印程序在begin至end之间的耗时，其打印格式为[Morn：日志]()格式，时间的单位为毫秒。

值得一提的是：此计时接口可以嵌套（最多嵌套16层）。例如一段程序分为若干步骤，我们既需要知道其中若干步骤的耗时，也需要知道程序的总耗时。此时可以写成：

```c
mTimerBegin();	//开始总计时
mTimerBegin();	//开始步骤1计时
step1();
mTimerEnd();	//结束步骤1计时
mTimerBegin();	//开始步骤2计时
step1();
mTimerEnd();	//结束步骤2计时
mTimerBegin();	//开始步骤3计时
step1();
mTimerEnd();	//结束步骤3计时
mTimerEnd();	//结束总计时
```



#### 时间日期格式化

```c
void mTimeString(char *out,const char *format);
```

此函数主要是为了方便的打印时间和日期。接口中，out用来存放格式化的输出字符串。format是用户预设的字符串格式。

先看一下效果：

```c
#include <stdio.h>
#include <string.h>
#include "morn_util.h"
int main()
{
    char t_str[32];
    mTimeString(t_str,NULL                                   );printf("%s"  ,t_str);
    mTimeString(t_str,"%Y.%M.%D %H:%m:%S"                    );printf("%s\n",t_str);
    mTimeString(t_str,"%h:%m:%S %Y.%M.%D %sW"                );printf("%s\n",t_str);
    mTimeString(t_str,"%02H:%02m:%02S %Y.%02M.%02D %aW"      );printf("%s\n",t_str);
    mTimeString(t_str,"%sM.%D %Y %H:%m:%S %sW"               );printf("%s\n",t_str);
    mTimeString(t_str,"%Y %aM %D %aW %H:%m:%S"               );printf("%s\n",t_str);
    mTimeString(t_str,"现在是%Y年%M月%D日，星期%W，%H时%m分%S秒" );printf("%s\n",t_str);
}
```

其输出结果为：

```
Thu Aug 13 22:28:03 2020
2020.8.13 10:28:3
22:28:3 2020.8.13 Thursday
10:28:03 2020.08.13 Thur.
August.13 2020 10:28:3 Thursday
2020 Aug. 13 Thur. 10:28:3
现在是2020年8月13日，星期4，10时28分3秒
```

如果format输入为NULL，则与C语言库函数`asctime`的输出无异。

这里的重点是时间格式的设置。这里的`%Y %M %D %H %m %S`等都是格式化字符，其使用方法与C语言中`printf`系列函数的`%d %s`无异。这其中：

* %Y用来输出公元纪年（year），为整数。
* %M用来输出公历月份（month），为整数1至12之间。
* %D用来输出公历日期（day），为整数1至31之间。
* %W用来输出星期（week），为整数1至7之间。

* %H用来输出小时（hour），为整数0至23之间。
* %h用来输出小时（hour），与%H的区别在于：%h为整数0至12之间。
* %m用来输出分钟（minute），为整数0至59之间，注意与%M（月份）。

* %S用来输出分钟（second），为整数0至59之间。

* %sM用来输出公历月份的英文名（string month），为字符串，如"January","February"等。

* %aM用来输出公历月份的英文简写（abbreviated month），为字符串，如"Jan.","Feb."等。

* %sW用来输出星期的英文名（string week），为字符串，如"Sunday","Monday"等。

* %aW用来输出星期的英文简写（abbreviated week），为字符串，如"Sun.","Mon."等。

  