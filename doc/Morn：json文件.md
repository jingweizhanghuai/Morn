## Morn：json文件

Morn提供了对.json文件的加载、解析、读写、增删、保存等功能。

一个典型的json文件如下（本文以此为例）：

```json
{
    "hello": "world",
    "t": true ,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1415926,
    "a1": [0,1,2,3],
    "a2": [[00,01,02,03],[10,11,12,13],[20,21,22,23]],
    "a3": [
        [[000,001,002],[010,011,012]],
        [[100,101,102],[110,111,112]],
        [[200,201,202],[210,211,212]]
    ],
    "a4": [{"value":0},{"value":1}],
    "date":
    {
        "year" :2021,
        "month":"June",
        "day":5
    },
    "city":[
        {"Beijing":["Dongcheng","Xicheng","Haidian","Chaoyang"]},
        "Shanghai",
        "Tianjin"
    ],
    "province":
    {
        "Hebei":["Shijiazhuang","Tangshan","Hengshui"],
        "Anhui":["Hefei","Huangshan"],
        "Gansu":"Lanzhou"
    }
}
```

json字符串是由一个个节点（node）按照一定的规则组合在一起形成的。这些node包括多种形式，它们可以是一个键值对（key-value)，也可以存在于数组中是一个值（value），在Morn中将其定义为以下类型：

```c
#define JSON_UNKNOWN     0
#define JSON_KEY_UNKNOWN 1
#define JSON_BOOL        2
#define JSON_KEY_BOOL    3	//例如上例中的 "t": true ,"f": false,
#define JSON_INT         4	//例如上例中的 0,1,2,3
#define JSON_KEY_INT     5	//例如上例中的 "i": 123,
#define JSON_DOUBLE      6
#define JSON_KEY_DOUBLE  7	//例如上例中的 "pi": 3.1415926,
#define JSON_STRING      8	//例如上例中的 "Dongcheng","Xicheng","Haidian","Chaoyang"
#define JSON_KEY_STRING  9	//例如上例中的 "hello": "world",
#define JSON_LIST       10	
#define JSON_KEY_LIST   11	//例如上例中的 "date":{"year":2021,"month":"June","day":5},
#define JSON_ARRAY      12	//例如上例中的 [10,11,12,13]
#define JSON_KEY_ARRAY  13	//例如上例中的 "a1": [0,1,2,3],
```

Morn中对于节点（node）的定义如下：

```c
struct JSONNode
{
    union
    {
        char   dataBool;
        int    dataS32;
        double dataD64;
        char   *string;
    };
    char *key;
    char type;
};
```

其中：dataBool对应布尔型数值，dataS32对应整形数据，dataD64对应浮点型数据，string是字符串型数据的首地址。key是键值。type是节点类型，为以上14种类型之一。



### 接口

#### 加载json文件

```c
struct JSONNode *mJSONLoad(MFile *jsonfile);
```

输入是一个json的文件，输出为经过解析后的json的顶层节点（node）。

一个简单的应用如下：

```c
MFile *file = mFileCreate("./test_json.json");
struct JSONNode *json=mJSONLoad(file);
...
mFileRelease(file);
```

#### 解析json字符串

```c
struct JSONNode *mJSONParse(MString *jsondata);
```

输入时一个符合json格式的字符串，输出为解析后的json顶层节点（node）。

一个简单的应用如下：

```c
MString *string = mStringCreate("{\"hello\":\"world\",\"t\":true,\"i\":123}");
struct JSONNode *json=mJSONLoad(string);
...
mStringRelease(string);
```

以上：`mJSONLoad`用于解析文件，`mJSONParse`用于解析字符串，对于文件的解析，可以直接使用`mJSONLoad`解析，也可以先把文件读入字符串中，再使用`mJSONParse`解析。

#### 读json节点

```c
struct JSONNode *mJSONRead(struct JSONNode *node);
struct JSONNode *mJSONRead(struct JSONNode *node,int n);
struct JSONNode *mJSONRead(struct JSONNode *node,const char *key);
```

其中所输入的node，必须是list类型（`JSON_LIST`或`JSON_KEY_LIST`）array类型（`JSON_ARRAY `或`JSON_KEY_ARRAY`）（否则读取失败），读取失败返回值是NULL。

此接口有三种形式：

```c
struct JSONNode *child;
child = mJSONRead(mother);			//mother是list或array类型，读取mother的第1个子节点
child = mJSONRead(mother,5);		//mother是list或array类型，读取mother的第5个子节点
child = mJSONRead(mother,"[5]");	//mother是array类型，读取mother的第5个子节点
child = mJSONRead(mother,"child5");	//mother是list类型，读取mother所有子节点中,key值为“child5"的子节点
child = mJSONRead(mother,"a.b[3].c.d[6]");	//读取深层节点
```

#### json子节点数量

```c
int mJSONNodeNumber(struct JSONNode *node);
```

例如上例中：

```c
MFile *file = mFileCreate("./test_json.json");

struct JSONNode *json=mJSONLoad(file);
int json_num = mJSONNodeNumber(json);//结果为12，分别为"hello","t","f","n","i","a1","a2","a3","a4","date","city","province"

struct JSONNode *date=mJSONRead(json,"date");
int date_num = mJSONNodeNumber(date);//结果为3，分别为“year","month","day"

struct JSONNode *a1=mJSONRead(json,"a1");
int a1_num = mJSONNodeNumber(a1);//结果为4，分别为0,1,2,3

mFileRelease(file);
```









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





