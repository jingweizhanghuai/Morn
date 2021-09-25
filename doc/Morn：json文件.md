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
        int8_t   dataBool;	 //type为JSON_KEY_BOOL或JSON_BOOL时有效
        int32_t  dataS32;	 //type为JSON_KEY_INT或JSON_INT时有效
        double   dataD64;	//type为JSON_KEY_DOUBLE或JSON_DOUBLE时有效
        char    *string;		  //type为JSON_KEY_STRING或JSON_STRING时有效
        uint16_t num;		 //子节点数量，type为JSON_KEY_ARRAY、JSON_ARRAY、JSON_KEY_LIST或JSON_LIST时有效
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





