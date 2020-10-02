## Morn：日志

Morn的日志是一个极简的，几乎没有学习成本的日志。它可以实现：

*  多种输出，包括动态文件、控制台、和用户自定义输出。

* 日志分级，选择性的日志输出，可自定义日志级别

* 预设日志格式，支持自定义格式

* 根据日志文件大小，自动分割日志文件。

* 线程安全

* 异步日志

  

### 接口

Morn日志相关的接口极简单，只有`mLogSet`和`mLog`两个接口，其中前者用于配置日志参数，后者用于输出日志。

#### 输出日志

```c
void mLog(int Level,const char *format,...);
```

这个接口简单到几乎可以不去解释。一个简单的例子：

```
int main()
{
    mLog(MORN_INFO, "this is a Morn log, num=%d\n",1);
    return 0;
}
```

其运行的结果为：

```
this is a Morn log, num=1
```



#### 日志设置

```c
void mLogSet(int levelset);
void mLogSet(const char *filename);
void mLogSet(int levelset,const char *filename);
void mLogSet(int output,const char *filename);
void mLogSet(int levelset,int output,const char *filename);
void mLogSet(int levelset,int output,const char *filename,int64_t filesize);
void mLogSet(int levelset,int output,void (*func)(void *,int,void *),void *para);
void mLogSet(int levelset,int output,const char *filename,int64_t filesize,void (*func)(void *,int,void *),void *para);
```

`mLogSet`用来设置日志相关的参数，它包括以上7种形式。

程序中并非必须使用mLogSet函数，当没有使用mLogSet函数时，默认将日志打印在控制台上，日志级别为Info（Release版本）或Debug（Debug版本）。

`mLogSet`通常写在程序的开头，用以配置日志参数。也可以在程序中多次调用，用以改变日志配置。

以上`mLogSet`接口中：

* levelset为设定的日志输出级别，当`mLog`函数中指定的日志级别大于等于此levelset时，日志才会被输出，否则被忽略。此项可设置为DFLT。

* output为日志输出方式，包括`MORN_LOG_CONSOLE`（控制台）、`MORN_LOG_FILE`（文件）`MORN_LOG_CUSTOM`（用户自定义），同时也可以设置为`MORN_LOG_CONSOLE & MORN_LOG_FILE`表示即在控制台输出，也在文件中输出。此项可指定为DFLT，若已设置filename则默认输出到文件，若已设置func则默认按照用户自定义方式输出，否则默认输出到控制台。

* filename为日志输出的文件名，若指定output为文件，则必须设置此项。

* filesize：日志文件分割大小，单位为字节，只有设置了output为文件时此项才有效。若不设置此项，则所有日志将输出到同一文件中（不作分割）。否则日志文件大小超过此值时，将会新建一日志文件。

* func：日志处理函数。它必须遵循以下形式：

  ```c
  void func(char *log_data,int log_size,void *para);
  ```

* para：即func项所需要的para。



### 使用

#### 设置日志级别

此函数中的level即为日志级别。当此level大于等于`mLogSet`所配置的日志等级（未配置时使用默认配置）时，日志才会输出，否则忽略。

Morn预设的日志级别由低到高分别是`MORN_DEBUG`，`MORN_INFO`，`MORN_WARNING`，`MORN_ERROR `，他们的定义分别为：

```c
#define MORN_DEBUG    0
#define MORN_INFO    16
#define MORN_WARNING 32
#define MORN_ERROR   48
```

用户除了可以使用以上四种级别外，还可以自定义日志级别。例如定义一种日志级别为NOTICE，其日志级别高于INFO低于WARNING，则可定义为

```c
#define NOTICE (MORN_INFO+1)
mLog(NOTICE, "this is a Morn log, num=%d\n",1);
```

#### 使用预定义格式

Morn里使用语法糖`mLogFormat`预设了**5种**日志格式。以下例说明之：

```c
int main()
{
    mLog(MORN_INFO, mLogFormat(1,"this is a Morn log, num=%d"),1);
    mLog(MORN_INFO, mLogFormat(2,"this is a Morn log, num=%d"),2);
    mLog(MORN_INFO, mLogFormat(3,"this is a Morn log, num=%d"),3);
    mLog(MORN_INFO, mLogFormat(4,"this is a Morn log, num=%d"),4);
    mLog(MORN_INFO, mLogFormat(5,"this is a Morn log, num=%d"),5);
    return 0;
}
```

此程序的输出为：

```
[test_log.c,line 16,function main]Info: this is a Morn log, num=1
[2020.09.13 18:03:43]Info: this is a Morn log, num=2
[thread001]Info: this is a Morn log, num=3
[2020.09.13 18:03:43 thread001]Info: this is a Morn log, num=4
[2020.09.13 18:03:43 thread001 test_log.c,line 20,function main]Info: this is a Morn log, num=5
```

以上可见，Morn所输出的日志格式包括：①日志所在文件、②日志所在行、③日志所在函数，③日志生产日期和时间，④日志所属线程，⑤日志级别。以上5种预设格式是以上这些信息的组合。

Morn库所有自带的日志输出（例如定时信息、异常信息），皆采用格式1。

值得一提的是`mLogFormat`只是一个语法糖，只有以上一种用法，以下两种用法是错误的：

```c
int a=5;
mLog(MORN_INFO, mLogFormat(a,"this is a Morn log"));//错误用法

char *message = "this is a Morn log";
mLog(MORN_INFO, mLogFormat(5,message));				//错误用法
mLog(MORN_INFO, mLogFormat(5,"%s"),message);		//正确用法
```

如果误用`mLogFormat`，将会导致编译语法错误。

#### 自定义格式

Morn认为：任何的预设格式都未必能满足用户的所有需求，Morn鼓励用户自定义日志格式。

例如，需要在日志格式中输出程序的作者，最简单的方式是：

```c
mLog(MORN_INFO, "[%s,line %d,function %s,author:%s]%s: this is a Morn log" ,__FILE__,__LINE__,__FUNCTION__,"JingWeiZhangHuai",mLogLevel(),1);
```

或

```c
mLog(MORN_INFO, mLogFormat(1,"author:%s. this is a Morn log"),"JingWeiZhangHuai",1);
```

以上两种方法皆可，但过于繁琐，Morn所鼓励的方式是仿照`mLogFormat`的方式定义日志格式，即：

```c
//定义在头文件中
#define MY_FORMAT(Message) "[%s,line %d,function %s,author:%s]%s:"  Message  "\n", __FILE__,__LINE__,__FUNCTION__,"JingWeiZhangHuai",mLogLevel()

//使用在函数中
mLog(MORN_INFO, MY_FORMAT("this is a Morn log"));
```

以下列出了几种可能在自定义格式时可能用到的接口：

* 日志等级

```c
const char *mLogLevel();  		
```

此接口返回`mLog`中所输入的level。为字符串："Debug"，“Info"，"Warning"或"Error"。

* 当前时间

```c
const char *mTimeString(int64_t time_value,const char *format);
```

详见[Morn：时间和日期](./Morn：时间和日期)。在日志中常使用`mTimeString(DFLT,NULL)`。

* 当前线程

```c
int mThreadID();
```

返回值为当前线程编号，此编号并非系统中的线程ID，是由1开始递增的整数，如第一个线程返回1，第二个线程返回2等。

#### 日志的多种输出

Morn日志可选择三种输出方式，分别为：

```c
#define MORN_LOG_CONSOLE  (~1)	//输出到控制台
#define MORN_LOG_FILE     (~2)	//输出到日志文件
#define MORN_LOG_CUSTOM   (~4)	//用户自定义输出
```

以上三种方式可以混合使用，例如：

```c
mLogSet(MORN_INFO);//输出到控制台
mLogSet(MORN_INFO,"./test.log");//输出到文件
mLogSet(MORN_INFO,MORN_LOG_CONSOLE&MORN_LOG_FILE,"./test.log");//既输出到控制台，也输出到文件
mLogSet(MORN_INFO,MORN_LOG_CUSTOM,my_log_func,my_log_func_para);//以用户自定义方式输出
mLogSet(MORN_INFO,MORN_LOG_CONSOLE&MORN_LOG_FILE&MORN_LOG_CUSTOM,"./test.log",DFLT,my_log_func,my_log_func_para);//同时以控制台、文件、用户自定义三种方式输出
```

其中用户自定义输出详见下文。

#### 日志文件分割

当日志过多，文件过大时，往往并不希望所有的日志都保存在同一文件里，这是需要对日志文件进行分割。设置`mLogSet`接口中的filesize项可实现此功能。

例如：

```c
mLogSet(DFLT,"./test.log",1024*1024);
```

以上表示每个日志文件的大小不超过1M字节。此时会在指定目录下生成test.log、test_1.log、test_2.log……等一系列不超过1M字节的日志文件。

#### 日志自定义输出

Morn已提供了控制台和文件两种输出方式，但是Morn认为：无论提供多少种日志输出方式都未必能够满足所有的应用场景，最好的办法是允许用户自定义日志的输出方式。

用户自定义日志输出方式是通过在`mLogSet`接口中设置func和para选项来实现的。

例如，将日志通过UDP上传到另一台计算机：

```c
void my_log_func(char *log_data,int log_size,char *udp_address)
{
    mUDPSend(udp_address,log_data,log_size);
}
...
int main()
{
    mLogSet(DFLT,MORN_LOG_CUSTOM,my_log_func,"192.168.0.123:8888");
    ...
    mLog(MORN_INFO,"this is a Morn log");
    ...
    return 0;
}
```



### 性能

为了测试Morn日志的性能，使用以下程序进行了测试，测试中将Morn日志与常用的glog、spdlog、log4cpp三种日志库进行比较：

```cpp
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<iostream>

#include "glog/logging.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"

#include "morn_util.h"

int main(int argc, char** argv)
{
    char datas[100][32];
    int datai[100];
    double datad[100];
    for(int i=0;i<100;i++)
    {
        //生成随机整数
        datai[i]=mRand(DFLT,DFLT);
        //生成随机浮点数
        datad[i]=(double)mRand(-1000000,1000000)/1000000.0;
        //生成随机字符串
        int len = mRand(15,31);
        for(int j=0;j<len;j++)datas[i][j]=mRand('a','z');
        datas[i][len]=0;
    }
    
    mTimerBegin("glog");
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::GLOG_INFO, "./test_log_glog.log");
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        LOG(INFO)<<": Hello glog, "<<"datai="<<datai[i]<<", datad="<<datad[i]<<", datas="<<&(datas[i][0]);
    }
    google::ShutdownGoogleLogging();
    mTimerEnd("glog");

    mTimerBegin("spdlog");
    auto console2 = spdlog::basic_logger_mt(argv[0],"./test_log_spdlog.log");
    spdlog::set_pattern("[%Y.%m.%d %H:%M:%S thread%t]%l: %v");
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        console2->info("[{} line {},function {}] Hello spdlog, datai={}, datad={}, datas={}",__FILE__,__LINE__,__FUNCTION__,datai[i],datad[i],&(datas[i][0]));
    }
    mTimerEnd("spdlog");

    mTimerBegin("log4cpp");
    log4cpp::FileAppender * appender = new log4cpp::FileAppender("appender","./test_log_log4cpp.log");
    log4cpp::PatternLayout* pLayout = new log4cpp::PatternLayout();
    pLayout->setConversionPattern("[%d thread%t %c]%p: %m%n");
    appender->setLayout(pLayout);
    log4cpp::Category& root =log4cpp::Category::getRoot();
    log4cpp::Category& infoCategory =root.getInstance(argv[0]);
    infoCategory.addAppender(appender);
    infoCategory.setPriority(log4cpp::Priority::INFO);
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        infoCategory.info("[%s,line %d,function %s] Hello log4cpp, datai=%d, datad=%f, datas=%s",__FILE__,__LINE__,__FUNCTION__,datai[i],datad[i],&(datas[i][0]));
    }
    log4cpp::Category::shutdown();
    mTimerEnd("log4cpp");

    mTimerBegin("Morn");
    mLogSet(DFLT,DFLT,"./test_log_morn.log");
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        mLog(MORN_INFO,mLogFormat(5,"Hello Morn, datai=%d, datad=%f, datas=%s"),datai[i],datad[i],&(datas[i][0]));
    }
    mTimerEnd("Morn");

    return 0;
}
```

以上分别测试了1000000输出条日志所需时间。四种日志的输出分别是：

* glog

```c
I20200913 21:14:36.839823   148 test_log2.cpp:48] : Hello glog, datai=17729, datad=-0.761655, datas=cuppeubmapohxinsmwoumohsrmfdi
```
* spdlog

```c
[2020.09.13 21:14:44 thread148]info: [test_log2.cpp line 59,function main] Hello spdlog, datai=17729, datad=-0.761655, datas=cuppeubmapohxinsmwoumohsrmfdi
```
* log4cpp

```
[2020-09-13 21:14:45,783 thread148 .\test_log2.exe]INFO: [test_log2.cpp,line 75,function main] Hello log4cpp, datai=17729, datad=-0.761655, datas=cuppeubmapohxinsmwoumohsrmfdi
```
* Morn

```
[2020.09.13 21:15:15 thread001 test_log2.cpp,line 85,function main]Info: Hello Morn, datai=17729, datad=-0.761655, datas=cuppeubmapohxinsmwoumohsrmfdi
```

测试结果为：

![](日志.PNG)

以上可见：

| 类别    | 速度（条/秒） | 速度（kB/s) |
| ------- | ------------- | ----------- |
| glog    | 18432         | 22319       |
| spdlog  | 82072         | 119414      |
| log4cpp | 9979          | 16574       |
| Morn    | 61977         | 87197       |

Morn的速度稍慢于spdlog，但是远快于glog和log4cpp。