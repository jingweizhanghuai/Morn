## Morn：文件加密解密

Morn所提供的文件加密解密是DES加密解密，什么是DES？百度一下就会知道。



### 接口

#### 文件加密

```c
void mEncrypt(const char *in_name,const char *out_name,uint64_t key);
```

这是把一个文件经过加密后输出到另一个文件。in_name就是输入的文件名，out_name就是输出的文件名。

key是秘钥，秘钥是一个64位整数。当然你也可以输入秘钥为DFLT，这个时候将使用默认秘钥（默认秘钥是Morn提供的，对于所有的Morn用户都是同一个秘钥，所以如果对方也用Morn的话，可以很容易解密）。



#### 文件解密

```c
void mDecrypt(const char *in_name,const char *out_name,uint64_t key);
```

这是把一个文件经过加密后输出到另一个文件。in_name就是输入的文件名，out_name就是输出的文件名。

key是秘钥，秘钥是一个64位整数，如果设置为DFLT，则使用默认秘钥。



#### MFile加密

```c
void mFileEncrypt(MFile *file,uint64_t key);
```

这个和`mEncrypt`的区别在于，这个是对MFile的操作，加密后还是保存在file里。



#### MFile解密

```c
void mFileDecrypt(MFile *file,uint64_t key);
```

这个和`mDecrypt`的区别在于，这个是对MFile的操作，加密后还是保存在file里。

通常的应用场景是：你向客户交付的时候，不想提供源码，也不想提供原始文件，而是提供一个可执行文件和一个加密后的数据文件。你希望解密后的文件只有你可见而用户不可见，那么这时候你就需要使用`mFileDecrypt`这个函数了。

例如你向用户提供的配置文件（.ini）是经过加密的，在程序内部配置的时候就需要这么写：

```c
MFile *file = mFileCreate("./encrypt_file.ini");
mFileDecrypt(file,my_key);
mINIRead(file,"section_name","key_name");
...
mFileRelease(file);
```



总之`mEncrypt`和`mFileDecrypt`这两个函数会更常用一些，另外两个函数不太常用到。



### 工具

Morn除了提供了DES加密解密的函数以外，还提供了一个工具Crypt.exe。这个工具在[../tool/](../tool/)文件夹里。

在命令行（cmd或shell或其它）里，运行`Crypt.exe --help`可以看到工具的帮助信息。

```
Options:
--help              Display this information
-v                  Display version information
-e                  Set input file for encrypt
-d                  Set input file for decrypt
-o                  Set output file
```

简单的说，你想加密一个文件就是：

```
Crypt.exe -e ./file_in -o ./file_out -k abcdefgh
```

你想解密一个文件就是：

```
Crypt.exe -d ./file_in -o ./file_out -k abcdefgh
```

