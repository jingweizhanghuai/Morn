## Morn：日志文件

Morn中的日志保存到文件里，一种是打印在屏幕上。

Morn的日志分为三个等级：一般信息（INFO），警告信息（WARNING）和错误信息（ERROR）。

#### 基本操作

如果你想把日志保存到日志文件的话，那么你首先需要用`mFileCreate`创建一个日志文件，程序结束的时候要用`mFileRelease`函数把创建的MFile释放掉。

##### 日志设置

```c
void mLogSet(MFile *file,int level);
```

level是所处理日志的等级。如果设置为INFO，那么所有的日志信息都会被处理，如果设置为WARNING，那么只有警告和错误信息会被处理，一般信息不回被处理，如果设置为ERROR，那么只有错误信息会被处理，警告信息和一般信息都不回被处理，level的默认值是INFO。

通常错误信息和警告信息是由异常处理函数`mException`调用的。

#### 写日志

```c
void mLog(int level,char *format,...);
```

level是当前这一条日志的等级（INFO，WARNING或ERROR）。

之后跟的是日志的内容。

注意：如果在`mLog`之前，调用过`mLogSet`函数的话（只要在执行顺序上`mLogSet`在`mLog`之前即可，不要求`mLogSet`和`mLog`出现在同一函数里），那么日志就会被写入你设置的文本文件里。但是如果在`mLog`之前，没有调用过`mLogSet`函数的话，日志内容会打印在屏幕上，这时类似于`printf`。





