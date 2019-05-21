## Morn：异常处理

首先声明，Morn的作者不喜欢Morn里面的异常处理，所以以下会多少有些吐槽

#### 接口

```c
mException(int exception_flag,int exception ID,char *info);
mExceptionBegin();
mExceptionEnd()；
```

Morn的异常处理包括两种：

第一种是作者喜欢的，即当发生异常时直接` exit(0)`，只使用mException的时候，就属于这种情况。例如下例：

```c
#define FILE_ERROR 1
void file_read(char *filename)
{
    FILE *f = fopen(filename,"r");
    mException((f==NULL),FILE_ERROR,"can not open file");	//出现异常时直接退出
    ……
}
```

如果以上这种直接退出的异常处理可以满足你的需求的话，就不要往后看了。

但是，在很多情况下，不允许程序直接退出，所以就有了第二种异常处理：

```c
#define FILE_ERROR 1
void file_list_read(char *file_list[],int file_num)
{
    for(int i=0;i<file_num;i++)
    {
        mExceptionBegin();					//开始监测异常
    	FILE *f = fopen(file_list[i],"r");
    	mException((f==NULL),FILE_ERROR,"can not open file");	//出现异常时跳转到mExceptionEnd处
    	……
            
        mExceptionEnd();					//结束监测异常，当出现异常时，将跳转到此处
        if(morn_exception == FILE_ERROR)
            continue;
        ……
    }
}
```

这段程序的意思是如果读不到文件，就跳过直接读下一文件，与之前的程序相比，这里除了使用mException以外，还使用了mExceptionBegin和mExceptionEnd。

当mException的执行顺序在mExceptionBegin和mExceptionEnd之间时，发生异常将不再直接退出，而是跳转到mExceptionEnd所在的位置继续执行。也就是说mExceptionBegin相当于其它编程语言里的try，mExceptionEnd相当于其它编程语言里的catch

#### 规则

有一些规则：

* mExceptionBegin和mExceptionEnd必须在同一函数中被调用，且必须成对调用（否则程序会编译出错）。

* mException并不要求与mExceptionBegin和mExceptionEnd同函数，只要执行顺序上处在mExceptionBegin和mExceptionEnd之间就可以了。也就是说，mException的跳转是可以跨函数的。

* 发生异常时，在mExceptionBegin和mExceptionEnd之间执行的mException执行跳转，并且将你输入的exception_ID赋值给morn_exception，其它的mExcepion执行直接退出的操作。

* mExceptionBegin和mExceptionEnd可以嵌套使用，mException执行跳转的时候是跳转到执行顺序上最近的mExceptionEnd处。

  ```c
  #define FILE_ERROR 1
  void file_read(char *filename)
  {
      mExceptionBegin();		//子函数中开始监测异常。
      FILE *f = fopen(filename,"r");
      mException((f==NULL),FILE_ERROR,"can not open file");	//这个异常出现时，将跳转到子函数的mExceptionEnd处。
      ……
      mExceptionEnd();		//子函数中结束监测异常。
      mException((morn_exception == FILE_ERROR),FILE_ERROR,"read file error");//这个异常出现时将跳转到上一层函数的mExceptionEnd处。
  }
  
  void file_list_read(char *file_list[],int file_num)
  {
      mException((file_num<=0),FILE_ERROR,"no file need to read");	//这个异常出现的时候将直接退出，它不在任何mExceptionBegin和mExceptionEnd之间。
      for(int i=0;i<file_num;i++)
      {
          mExceptionBegin();	//开始监测异常
      	file_read(file_list[i]);
          ……
          mExceptionEnd();	//结束监测异常
          if(morn_exception == FILE_ERROR)
              continue;
      }
  }
  ```

* mException只能在同一线程内跳转，不能跨线程跳转（这是显而易见的）。所以有多个线程中需要使用mException跳转的时候，需要在每个线程内都使用mExceptionBegin和mExceptionEnd。

* mExceptionBegin和mExceptionEnd不要用在递归中，例如下例：

  ```c
  void func(int a)
  {
      mException();	//开始监控异常，这种调用是不行的
      ……
      func（a);		//递归调用
      ……
      mException();	//开始监控异常，这种调用是不行的
  }
  ```

  

#### 痛点

以上的这些规则，虽然好几条，但是并不复杂，还算比较友好。但是以下这个才是最大的痛点，比如下例

```c
void func(int a)
{
    mException();	//开始监控异常
    ……
    int *array = mMalloc(100);
    ……
    mException((flag==1),ERROR_ID,"some error");//异常
   	……
   	mFree（array);
    ……
    mException();	//开始监控异常
}
```

以上这段程序在正常执行的时候没有任何问题，但是当遇见异常的时候，会从mException处直接跳转到mExceptionEnd处，mFree将不会被执行，也就是说申请的内存将不会被释放掉，会有内存问题。

当mException和mExceptionBegin、mExceptionEnd在同一函数中的时候，这种bug尚容易被察觉，但是如果他们不在同一函数中被使用，甚至不在同一文件中被使用，甚至不是由同一程序员编写的程序的时候，这种bug将很难被察觉。

我猜这也是为啥C语言本身没有异常处理的一个重要原因。

为了避免这种bug，尽量不要在mExceptionBegin和mExceptionEnd之间去申请和释放内存，但是，很显然很多时候不得不去这么做。这就极大的增加了Morn出现bug的风险。（虽然，当异常出现的频率不高的时候，这种bug是可以被接受的。）

不过，好在Morn还提供了内存检查的工具，所以，强烈建议在使用了这种跳转功能的mException之后，先用工具检查一下内存。









