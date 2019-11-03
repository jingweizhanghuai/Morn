## Morn：ini文件

一个工程里面，配置文件是不可或缺的。Morn所推荐的配置文件是最简单的(.ini)文件，（Morn另提供了json文件操作，json文件也可以作为配置文件）。

ini文件的格式，你可以百度一下就能知道。简单的说，一个ini文件包含若干个节（section），每个节包含若干个键值对（key-value），key就是配置的名称，value就是配置的值。

一个典型的ini文件如下：

```ini
[张三]
年级 = 2
班级 = 1
类别 = 理科
成绩1 = 数学(90)
成绩2 = 英语(70)
成绩3 = 物理(95)
成绩4 = 化学(80)

[李四]
年级 = 3
班级 = 4
类别 = 文科
成绩1 = 数学(60)
成绩2 = 语文(90)
成绩3 = 英语(70)
成绩4 = 历史(95)
成绩5 = 政治(85)

[王二麻]
年级 = 1
班级 = 2
类别 = 退学
```



### 接口

ini文件也是文件，在进行ini文件操作之前，先要用`mFileCreate`创建一个文件，在结束的时候要用`mFileRekease`把创建的文件释放掉。

#### 读ini文件

```
char *mINIRead(MFile *file,const char *section,const char *key);
```

比如读张三的班级信息，那就是`int class=atoi(mINIRead(file,"张三","班级"));`。

因为读出的信息都是字符串，所以往往需要配合字符串处理的相关函数对结果进行处理。

如果在ini文件中没有找到你指定的section或key的话，函数将返回NULL。



#### 获取ini的节

```
MList *mINISection(MFile *file);
```

返回的是一个列表，列表里面的元素是字符串。



#### 获取ini的键

```
MList *mINIKey(MFile *file,const char *section);
```

其中section是传入你要获取哪个节里面的键。返回的也是一个列表，列表里面的元素是字符串。



### 示例

一个例子：计算上面这个ini里的每个学生的平均成绩：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_Util.h"

int main()
{
    int i,j;
    char *value;
    MFile *file = mFileCreate("./test_INIFile.ini");
    MList *student = mINISection(file);
    MList *score = mListCreate(DFLT,NULL);
    for(i=0;i<student->num;i++)
    {
        value = mINIRead(file,student->data[i],"年级");
        int grade = atoi(value);
        value = mINIRead(file,student->data[i],"班级");
        int class = atoi(value);
        char *label = mINIRead(file,student->data[i],"类别");
        float sum=0.0f;
        for(j=1;;j++)
        {
            char name[16];sprintf(name,"成绩%d",j);
            value = mINIRead(file,student->data[i],name);
            if(value==NULL) break;
            mStringSplit(value,"(",score);
            sum+=atof(score->data[1]);
        }
        printf("学生：%s %d年级%d班 %s 平均成绩:%f\n",
        student->data[i],grade,class,label,sum/(j-1));
    }
    mListRelease(score);
    mFileRelease(file);
}
```

运行结果为：

```
学生：张三 2年级1班 理科 平均成绩:83.750000
学生：李四 3年级4班 文科 平均成绩:80.000000
学生：王二麻 1年级2班 退学 平均成绩:-1.#IND00
```



另外，如果你的ini文件里有中文的话，注意编码方式。



