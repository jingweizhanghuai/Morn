## Morn：编译

Morn的编译使用make和makefile。（如果你喜欢其它方式的话，你得自己探索一下）。

首先，你得确认一下，你的电脑上装了make，并且make的路径在你的环境变量里。（尤其对于在Windows上开发的使用者，如果没有make的话，你得先下载安装一个）。



### 依赖

为了通用和便于移植，Morn尽量减少对其它库的依赖。但即使这样，有些库还是必须要用的。

读写图像文件，需要依赖：libjpeg、libpng（libpng又依赖zlib）。这几个库可以算是世界上装机量最大的库了。

深度学习框架，需要依赖blas，我在开发的时候使用的是openblas。

Morn的多线程，使用的POSIX标准的pthread，在MSVC下开发的用户，需要额外下载这个库。

我编译了部分平台下的第三方依赖库，你可以去下载：

* Linux下使用gcc  5.4.0编译的库：https://pan.baidu.com/s/1PnRGLXpTLJL2UtCYyZWH5A
* Windows下使用MinGW-W64 gcc 8.1.0编译的库：https://pan.baidu.com/s/1mjx_wSZX4iv6pen5SGFyEA
* Windows下使用MSVC14编译的库：https://pan.baidu.com/s/1I-DdMrBuYQ8V-EaVi1udFg

或者，你可以从这些库的官方网站下载编译这些库。

这些依赖的库文件，我把他放在../lib/里相对应的文件夹下。



### Linux+gcc编译

makefile是 [../gnu-makefile](../gnu-makefile)。你可以使用以下命令编译：

```
make -f gnu_makefile     		#编译静态库和tool
make static -f gnu_makefile		#编译静态库
make share -f gnu_makefile		#编译动态库
make clean -f gnu_makefile		#编译清空
```

编译后，会在[../lib/x64/gnu]()文件夹下，生成libmorn.a库文件（静态编译）。



### Windows+MinGW编译

makefile是 [../mingw-makefile](../mingw-makefile)。你可以使用以下命令编译：

```
make -f mingw_makefile     		#编译静态库和tool
make static -f mingw_makefile	#编译静态库
make share -f mingw_makefile	#编译动态库
make clean -f mingw_makefile	#编译清空
```

编译后，会在[../lib/x64/mingw]()文件夹下，生成libmorn.a库文件（静态编译）。



### Windows+VC编译

这个稍稍复杂，首先，你得先打开cmd命令行。cd到你安装的vc文件夹下（比如，我的文件夹是D:\ProgramFiles\VS2015\VC\bin\amd64）然后，执行这个文件夹里的vcvars64.bat脚本（32位或其他架构类似，但脚本文件名不同）。或者你把你的VC的文件夹添加到你的环境变量里，然后执行vcvars64.bat（或类似的）脚本。

然后回到Morn目录下，执行make命令。makefile是 [../msvc-makefile](../msvc-makefile)。你可以使用以下命令编译：

```
make -f msvc_makefile     		#编译静态库和tool
make static -f msvc_makefile	#编译静态库
make share -f msvc_makefile		#编译动态库
make clean -f msvc_makefile		#编译清空
```

编译后，会在[../lib/x64/msvc]()文件夹下，生成morn.lib库文件（静态编译）。



其他平台的编译可以参考以上。