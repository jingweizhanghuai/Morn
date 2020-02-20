## Morn：json文件

Morn提供了对.json文件的解析函数。

json文件的格式，百度一下就知道。

一个典型的json文件如下（本文以此为例）：

```json
{
	"年级":"3",
	"班级":"2",
	"班主任":"李老师",
	"任课老师":["王老师","张老师","刘老师"],
	"学生":[
  		{"姓名":"张三","性别":"男","成绩":{"语文":90,"数学":70,"文综":85}},
  		{"姓名":"李四","性别":"女","成绩":{"语文":60,"数学":100,"理综":95}},
  		{"姓名":"赵五","性别":"女","成绩":{"语文":90,"数学":60,"文综":52}},
  		{"姓名":"王二麻","性别":"男","成绩":{"语文":50,"数学":98,"理综":97}}
  	]
}
```

很容易理解：json就是一棵树，如上例，树上有”年级“、”班级“，”班主任“，”任课老师“，”学生“等若干个节点，其中节点”学生“是这棵树的一棵子树，”学生“子树有”姓名“、”性别“，”成绩“等若干节点，而节点”成绩“又是”学生“的一棵子树，它有”语文“，”数学“，”文综“，”理综“等节点。

因此，在内存中，json文件会被存储为一个MTree，关于树，你可以参考[Morn：树](Morn：树.md)

Morn中，json相关函数主要是满足读取json格式配置文件的需求，因此接口只有“读”（或者叫反序列化），没有“写”（序列化）。



### 接口

#### 加载json文件

```c
void mJSONLoad(const char *filename,MTree *tree);
```

这是一个非常Morn风格的接口，功能是将json文件读入一个MTree中。在使用这个函数之前，你需要先用`mTreeCreate`创建一棵树，在程序结束之前，你需要使用`mTreeRelease`释放这棵树。

这里，filename就是json文件的路径。tree是输出的树。

**在使用json文件的时候，你总是需要先执行这个函数。**



#### 树节点的json信息

```c
char *mJSONName(MTreeNode *node);
char *mJSONValue(MTreeNode *node);
```

其中`mJSONName`是获取树节点的json名，`mJSONValue`是获取树节点的json值。

例如`"年级":"3"`，这里“年级”就是json名，“3”就是json值。不论json名还是json值都是一个字符串。

例如，以下程序

```c
MTreeNode *node = json->treenode->child[0];
for(int i=0;i<node->child_num;i++)
    printf("%s是%s\n",mJSONName(node->child[i]),mJSONValue(node->child[i]));
```

得到的结果会是：

```
年级是3         
班级是2         
班主任是李老师      
任课老师是王老师     
任课老师是张老师     
任课老师是刘老师     
学生是          
学生是          
学生是          
学生是
```



#### json搜索

```c
void mJSONSearch(MTree *tree,MList *list,char *name);
```

这个是用来获取特定节点的值。tree是`mJSONLoad`的输出结果，list是一个用来保存搜索结果的容器，name是所搜索的节点名。

例如，想获取“班主任”的值：

```c
mJSONSearch(json,list,"班主任");
printf("班主任是%s\n",list->data[0]);
```

得到的结果将是：

```
班主任是李老师
```

再例如，想获取“任课老师”的值：

```c
mJSONSearch(json,list,"任课老师");
for(int i=0;i<list->num;i++) printf("任课老师%d是%s\n",i,list->data[i]);
```

得到的结果将是：

```
任课老师0是王老师
任课老师1是张老师
任课老师2是刘老师
```

再例如，想获取学生的数学成绩的值：

```c
MList *name = mListCreate(DFLT,NULL);mJSONSearch(json,name,"学生.姓名");
MList *score= mListCreate(DFLT,NULL);mJSONSearch(json,score,"学生.成绩.数学");
for(int i=0;i<list->num;i++) 
    printf("%s的数学成绩是%d\n",name->data[i],atoi(score->data[i]));
mListRelease(name);
mListRelease(score);
```

注意，因为学生的姓名在树的第三层，数学成绩在树的第四层，所以这里需要**用`.`分割各层的名字**，即找数学成绩就需要先找“学生”节点，再在其中找“成绩”节点，然后在其中找“数学”节点。

得到的结果是：

```
张三的数学成绩是70
李四的数学成绩是100
赵五的数学成绩是60
王二麻的数学成绩是98
```



### 示例

除了以上接口外，其他更多的对json树节点的操作可以使用MTree的相关函数，比如遍历树的各个节点`mTreeTraversal`，搜索某个节点`mTreeSearch`等。

这里重点介绍使用`mTreeSearch`函数对JSON树进行操作。

下例是实现“找出所有数学成绩高于90分的学生”。

```c
int main()
{
    MTree *json=mTreeCreate();
    mJSONLoad("./test_JSON_file.json",json);

    int func(MTreeNode *ptr,void *para)
    {return ((strcmp(mJSONName(ptr),"数学")==0)&&(atoi(mJSONValue(ptr))>=90));}
    
    MTreeNode *node = json->treenode; // 从树根开始搜索
    while(1)
    {
        node = mTreeSearch(node,func,NULL,0);
        if(node == NULL) break;
        printf("姓名%s\n",mJSONValue(node->parent->parent->child[0]));
    }

    mTreeRelease(json);
    return 0；
}
```

这个程序里从根节点开始寻找，找到json名为“数学”且json值大于90的节点时，就把对应的学生名打印出来。执行结果是：

```
姓名李四
姓名王二麻
```



下例是实现：将学生信息读入结构体中。

```c
int main()
{
    MTree *json=mTreeCreate();
    mJSONLoad("./test_JSON_file.json",json);
    
    struct Student
    {
        char *name;
        char *sex;
        char *course[3];
        int score[3];
    };
    
    int func1(MTreeNode *ptr,void *para) 
    {return (strcmp(mJSONName(ptr),para)==0);}
    
    node = json->treenode;  // 从树根开始搜索
    while(1)
    {
        struct Student student;
        MTreeNode *student_node = mTreeSearch(node,func1,"学生",0);
        if(student_node == NULL) break;
        node = mTreeSearch(student_node,func1,"姓名",0);
        student.name = mJSONValue(node);
        node = mTreeSearch(student_node,func1,"性别",0);
        student.sex  = mJSONValue(node);
        node = mTreeSearch(student_node,func1,"成绩",0);
        student.course[0] = mJSONName(node->child[0]);
        student. score[0] = atoi(mJSONValue(node->child[0]));
        student.course[1] = mJSONName(node->child[1]);
        student. score[1] = atoi(mJSONValue(node->child[1]));
        student.course[2] = mJSONName(node->child[2]);
        student. score[2] = atoi(mJSONValue(node->child[2]));
        printf("student name is %s,sex is %s,course0 is %s,score0 is %d,course1 is %s,score1 is %d,course2 is %s,score2 is %d\n",
               student.name,student.sex,
               student.course[0],student.score[0],
               student.course[1],student.score[1],
               student.course[2],student.score[2]);
    }
    mTreeRelease(json);
    return 0；
}
```

此例中：先从树根节点开始搜索”学生“节点，再从”学生“子树开始搜索“姓名”，“性别”，“成绩”节点，并将结果赋给结构体相应的项。得到的结果为：

```
student name is 张三,sex is 男,course0 is 语文,score0 is 90,course1 is 数学,score1 is 70,course2 is 文综,score2 is 85
student name is 李四,sex is 女,course0 is 语文,score0 is 60,course1 is 数学,score1 is 100,course2 is 理综,score2 is 95
student name is 赵五,sex is 女,course0 is 语文,score0 is 90,course1 is 数学,score1 is 60,course2 is 文综,score2 is 52
student name is 王二麻,sex is 男,course0 is 语文,score0 is 50,course1 is 数学,score1 is 98,course2 is 理综,score2 is 97
```



另外，如果你的json文件里有中文的话，**注意编码方式**。





