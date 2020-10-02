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
const char *mTimeString();
const char *mTimeString(const char *format);
const char *mTimeString(int64_t time_value,const char *format);
```

此函数主要是为了方便的打印时间和日期。接口中：

time_value是输入的时间值，若不设置或设置为DFLT，则为当前时间点。

format是用户预设的字符串格式，若不设置或设置为NULL，则使用默认格式`%Y.%02M.%02D %02H:%02m:%02S`

返回值是一个根据format生成的字符串。

先看一下效果：

```c
#include "morn_util.h"
int main()
{
    printf("%s\n",mTimeString());
    printf("%s\n",mTimeString(NULL));
    printf("%s\n",mTimeString("%Y.%02M.%02D %02H:%02m:%02S"));
    printf("%s\n",mTimeString(DFLT,"%Y.%02M.%02D %02H:%02m:%02S"));
    printf("%s\n",mTimeString(time(NULL),"%Y.%02M.%02D %02H:%02m:%02S"));

    printf("%s\n",mTimeString("%sM %D %Y %H:%m:%S %sW"          ));
    printf("%s\n",mTimeString("%Y %aM %D %aW %H:%m:%S"         ));
    printf("%s\n",mTimeString("%02H:%02m:%02S %Y.%02M.%02D %aW"));
    printf("%s\n",mTimeString("%CY年%CM月%CD日 星期%CW %CH时%Cm分%CS秒"));
    
    printf("%s\n",mTimeString(1218196800,NULL));
    printf("%s\n",mTimeString(1218196800,"%sM %D %Y %H:%m:%S %sW"          ));
    printf("%s\n",mTimeString(1218196800,"%Y %aM %D %aW %H:%m:%S"         ));
    printf("%s\n",mTimeString(1218196800,"%02H:%02m:%02S %Y.%02M.%02D %aW"));
    printf("%s\n",mTimeString(1218196800,"%CY年%CM月%CD日 星期%CW %CH时%Cm分%CS秒"));
    
    return 0;
}
```

其输出结果为：

```
2020.10.02 10:57:37
2020.10.02 10:57:37
2020.10.02 10:57:37
2020.10.02 10:57:37
2020.10.02 10:57:37
October 2 2020 10:57:37 Friday
2020 Oct 2 Fri 10:57:37
10:57:37 2020.10.02 Fri
二〇二〇年十月二日 星期五 十时五十七分三十七秒
2008.08.08 20:00:00
August 8 2008 20:0:0 Friday
2008 Aug 8 Fri 20:0:0
20:00:00 2008.08.08 Fri
二〇〇八年八月八日 星期五 二十时零分零秒
```

这里的重点是时间格式的设置。这里的`%Y %M %D %H %m %S`等都是格式化字符，其使用方法与C语言中`printf`系列函数的`%d %s`无异。这其中：

* %Y：输出公元纪年（year），为整数。

* %M：输出公历月份（month），为整数1至12之间。

* %D：输出公历日期（day），为整数1至31之间。

* %W：输出星期（week），为整数1至7之间。

* %H：输出小时（hour），为整数0至23之间。

* %m：输出分钟（minute），为整数0至59之间，注意与%M（月份）区分。

* %S：输出秒（second），为整数0至59之间。

* %sM：输出公历月份的英文名（string month），为字符串，如"January","February"等。

* %aM：输出公历月份的英文简写（abbreviated month），为字符串，如"Jan","Feb"等。

* %sW：输出星期的英文名（string week），为字符串，如"Sunday","Monday"等。

* %aW：输出星期的英文简写（abbreviated week），为字符串，如"Sun","Mon"等。

* %CY：输出公历年份的汉字名（Chinese year），为字符串，如"二〇二〇"，"一九四九"等。

* %CM：输出公历月份的汉字名（Chinese month），为字符串，如"一"，"二"，"三"等。

* %CD：输出公历日期的汉字名（Chinese day），为字符串，如"一"，"二十九"，"三十"等。

* %CW：输出星期的汉字名（Chinese week），为字符串，如"一"，"六"，"日"等。

* %CH：输出小时的汉字名（Chinese hour），为字符串，如"零"，"十五"，"二十三"等。

* %Cm：输出分钟的汉字名（Chinese minute），为字符串，如"零"，"四十五"，"五十九"等。

* %CS：输出秒的汉字名（Chinese second），为字符串，如"零"，"三十六"，"五十九"等。

  

#### 时间日期提取

```c
int64_t mStringTime();
int64_t mStringTime(char *in);
int64_t mStringTime(char *in,const char *format);
```

这个函数就是`mTimeString`的逆操作。

in是一个包含有时间日期信息的字符串，若不设置或设为NULL，则此函数与`time(NULL)`无异。

format是字符串的格式。若不设置或设置为NULL，则使用默认格式`%Y.%02M.%02D %02H:%02m:%02S`

返回值是in所对应的时间值。

例如：

```c
#include <stdio.h>
#include <string.h>
#include "morn_util.h"
int main()
{
    printf("%lld\n",mStringTime("2008.08.08 20:00:00"));
    printf("%lld\n",mStringTime("August 8 2008 20:0:0 Friday","%sM %D %Y %H:%m:%S %sW"));
    printf("%lld\n",mStringTime("2008 Aug 8 Sun 20:0:0","%Y %aM %D %aW %H:%m:%S"));
    printf("%lld\n",mStringTime("20:00:00 2008.08.08 Fri","%02H:%02m:%02S %Y.%02M.%02D %aW"));
    printf("%lld\n",mStringTime("二〇〇八年八月八日 星期五 二十时零分零秒","%CY年%CM月%CD日 星期%CW %CH时%Cm分%CS秒"));
    return 0;
}
```

其输出结果为：

```
1218196800 
1218196800 
1218196800 
1218196800 
1218196800 
```



值得说明的一点：`mTimeString`和`mStringTime`本质上是对标准库函数里`localtime`和`mktime`的封装，但是`mTimeString`和`mStringTime`**解决了时间值为负的问题**，使得可以使用此两函数处理1970年之前的时间点（比如1949年，但是不能超出int64_t的取值范围）。而`localtime`和`mktime`不能处理1970年之前的时间。



### 示例

```c
int main()
{
    printf("今天是%s\n",mTimeString(DFLT,"%Y.%M.%D %aW"));
    printf("1000天后是%s\n",mTimeString(time(NULL)+1000*24*3600,"%Y.%M.%D %aW"));
    printf("今天已经建国%d天\n",(time(NULL)-mStringTime("1949.10.1","%Y.%M.%D"))/(24*3600));
    printf("距离2021年高考还有%d天\n",(mStringTime("2021.6.7","%Y.%M.%D")-time(NULL))/(24*3600));
    printf("汶川大地震发生在星期%s\n", mTimeString(mStringTime("2008.5.12","%Y.%M.%D"),"%CW"));
    return 0;
}
```

程序的运行结果如下：

```
今天是2020.10.2 Fri
1000天后是2023.6.29 Thur
今天已经建国25934天
距离2021年高考还有247天
汶川大地震发生在星期一
```



