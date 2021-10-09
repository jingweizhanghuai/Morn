## Morn：json文件

Morn提供了对.json文件的解析功能。Morn的json解析具有**简单（仅有两个接口）和快速（远快于rapidjson）**的特点。

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
#define JSON_LIST       10	//例如上例中的 {"value":0}
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
        bool     dataBool;   //type为JSON_KEY_BOOL或JSON_BOOL时有效
        int32_t  dataS32;    //type为JSON_KEY_INT或JSON_INT时有效
        double   dataD64;    //type为JSON_KEY_DOUBLE或JSON_DOUBLE时有效
        char    *string;     //type为JSON_KEY_STRING或JSON_STRING时有效
        uint16_t num;        //子节点数量，type为JSON_KEY_ARRAY、JSON_ARRAY、JSON_KEY_LIST或JSON_LIST时有效
    };
    char *key;
    char type;
};
```

其中：dataBool对应布尔型数值，dataS32对应整形数据，dataD64对应浮点型数据，string是字符串型数据的首地址。key是键值。type是节点类型，为以上14种类型之一。



### 接口

#### 加载和解析json

```c
struct JSONNode *mJSONLoad(MFile *jsonfile);
struct JSONNode *mJSONLoad(MString *jsondata);
```

输入是一个json的文件或json字符串，输出为经过解析后的json的顶层节点（node）。

一个简单的应用如下：

```c
MFile *file = mFileCreate("./test_json.json");
struct JSONNode *json=mJSONLoad(file);
...
mFileRelease(file);
```

或：

```c
MString *string = mStringCreate("{\"hello\":\"world\",\"t\":true,\"i\":123}");
struct JSONNode *json=mJSONLoad(string);
...
mStringRelease(string);
```

以上：对于文件的解析，可以直接使用`mJSONLoad`解析，也可以先把文件读入字符串中，再解析。



#### 读json节点

```c
struct JSONNode *mJSONRead(struct JSONNode *node);
struct JSONNode *mJSONRead(struct JSONNode *node,int n);
struct JSONNode *mJSONRead(struct JSONNode *node,const char *key);
struct JSONNode *mJSONRead(struct JSONNode *node,struct JSONNode *dst);
struct JSONNode *mJSONRead(struct JSONNode *node,int n,struct JSONNode *dst);
struct JSONNode *mJSONRead(struct JSONNode *node,const char *key,struct JSONNode *dst);
```

其中所输入的node，必须是list类型（`JSON_LIST`或`JSON_KEY_LIST`）array类型（`JSON_ARRAY `或`JSON_KEY_ARRAY`）（否则读取失败），读取失败返回值是NULL。

此接口有三种形式：

```c
struct JSONNode *child;
child = mJSONRead(mother);          //mother是list或array类型，读取mother的第1个子节点
child = mJSONRead(mother,5);        //mother是list或array类型，读取mother的第5个子节点
child = mJSONRead(mother,"[5]");    //mother是array类型，读取mother的第5个子节点
child = mJSONRead(mother,"child5"); //mother是list类型，读取mother所有子节点中,key值为“child5"的子节点
child = mJSONRead(mother,"a.b[3].c.d[6]");   //读取深层节点
```

或：

```c
struct JSONNode child;
mJSONRead(mother,&child);           //mother是list或array类型，读取mother的第1个子节点
mJSONRead(mother,5,&child);         //mother是list或array类型，读取mother的第5个子节点
mJSONRead(mother,"[5]",&child);     //mother是array类型，读取mother的第5个子节点
mJSONRead(mother,"child5",&child);  //mother是list类型，读取mother所有子节点中,key值为“child5"的子节点
mJSONRead(mother,"a.b[3].c.d[6]",&child);    //读取深层节点
```



### 示例

本例中的完整程序参见[test_JSON_file.c](https://github.com/jingweizhanghuai/Morn/blob/master/test/test_JSON_file.c)

以本文开始的json文件为例，可使用以下程序读取此文件：

```c

char *jsontype[15]={"UNKNOWN","KEY_UNKNOWN","BOOL","KEY_BOOL","INT","KEY_INT","DOUBLE","KEY_DOUBLE","STRING","KEY_STRING","LIST","KEY_LIST","ARRAY","KEY_ARRAY","UNKNOWN"};

int main()
{
    MFile *file = mFileCreate("./test_json.json");

    struct JSONNode *json=mJSONLoad(file);
    printf("json->type=%s\n",jsontype[json->type]);
    printf("json->num=%d\n",json->num);

    struct JSONNode *node;
    node=mJSONRead(json,"hello");
    printf("node->type=%s\n",jsontype[node->type]);
    printf("node->key=%s\n",node->key);
    printf("node->string=%s\n",node->string);
    
    mFileRelease(file);
}
```

此例中读取了两个节点，即根节点json和"hello"节点node。此段程序的输出为：

```
json->type=LIST
json->num=13
node->type=KEY_STRING
node->key=hello
node->string=world
```



读取节点可根据需要写出以下形式：

```c
node=mJSONRead(json,"t");
if(node!=NULL)
{
    if(node->type==JSON_KEY_BOOL)
        printf("t=%d\n",node->dataBool);
}

struct JSONNode f_node;
node=mJSONRead(json,"f",&f_node);
printf("f=%d\n",f_node.dataBool);

int i=*(int *)mJSONRead(json,"i");
printf("i=%d\n",i);

double *pi=(double *)mJSONRead(json,"pi");
printf("pi=%lf\n",*pi);
```

此段程序的输出为：

```
t=1
f=0
i=123
pi=3.141592
```



这里需要注意的是，在Morn中，nul被解释为空字符串，因此

```c
node = mJSONRead(json,"n");
printf("type=%s,nul=%p\n",jsontype[node->type],node->string);
```

输出为：

```
type=KEY_STRING,nul=0000000000000000
```



对于嵌套的多层节点，可以逐级读取，如：

```c
node=mJSONRead(json,"date");
struct JSONNode *year=mJSONRead(node,"year");
printf("date.year=%d,type=%s\n",year->dataS32,mJSONNodeType(year));
struct JSONNode *month=mJSONRead(node,"month");
printf("date.month=%s,type=%s\n",month->dataS32,mJSONNodeType(month));
struct JSONNode *day=mJSONRead(node,"day");
printf("date.day=%d,type=%s\n",day->dataS32,mJSONNodeType(day));
```

也可以越级读取，如：

```c
struct JSONNode *year=mJSONRead(json,"date.year");
printf("date.year=%d,type=%s\n",year->dataS32,mJSONNodeType(year));
struct JSONNode *month=mJSONRead(json,"date.month");
printf("date.month=%s,type=%s\n",month->dataS32,mJSONNodeType(month));
struct JSONNode *day=mJSONRead(json,"date.day");
printf("date.day=%d,type=%s\n",day->dataS32,mJSONNodeType(day));
```

以上两段程序的输出皆是：

```
date.year=2021,type=KEY_INT
date.month=June,type=KEY_STRING
date.day=5,type=KEY_INT
```



对于数组的读取，可以使用以下多种灵活的形式：

```c
struct JSONNode *p;
node=mJSONRead(json,"a1");
p = mJSONRead(node);
printf("a1[0]=%d\n",p->dataS32);
p = mJSONRead(node,1);
printf("a1[1]=%d\n",p->dataS32);
p = mJSONRead(node,"[2]");
printf("a1[2]=%d\n",p->dataS32);
p = mJSONRead(json,"a1[3]");
printf("a1[3]=%d\n",p->dataS32);
```

以上程序的输出为：

```
a1[0]=0
a1[1]=1
a1[2]=2
a1[3]=3
```



对于二维或多维数组的读取，可以越级读取如：

```c
node = mJSONRead(json,"a2[1][2]");
```

也可以逐级读取：

```c
struct JSONNode *a2=mJSONRead(json,"a2");
for(int j=0;j<a2->num;j++)
{
    struct JSONNode *p1=mJSONRead(a2,j);
    for(int i=0;i<p1->num;i++)
    {
        struct JSONNode *p2=mJSONRead(p1,i);
        printf("%02d,",p2->dataS32);
    }
    printf("\n");
}
```

以上程序的输出为：

```
00,01,02,03,
10,11,12,13,
20,21,22,23,
```



列表与数组的混合读取：

```c
node = mJSONRead(json,"province.Hebei[0]");
printf("%s\n",node->string);
node = mJSONRead(json,"province.Anhui[0]");
printf("%s\n",node->string);
node = mJSONRead(json,"province.Gansu"   );
printf("%s\n",node->string);
```

以上程序的输出为：

```
Shijiazhuang
Hefei
Lanzhou
```



### 性能

完整的测试程序参见：[test_JSON_file2.cpp](https://github.com/jingweizhanghuai/Morn/blob/master/test/test_JSON_file2.cpp)

性能测试中对比了Morn与几种常用的json解析库：[cjson](https://github.com/DaveGamble/cJSON)、[jsoncpp](https://github.com/open-source-parsers/jsoncpp)、[nlohmann](https://github.com/nlohmann/json)、[rapidjson](https://github.com/Tencent/rapidjson)、[yyjson](https://github.com/ibireme/yyjson)

#### 测试一：

解析[citm_catalog.json](https://github.com/miloyip/nativejson-benchmark/blob/master/data/citm_catalog.json)，并读取文件中的areaId，测试解析和读取速度。测试程序如下（仅节选Morn的测试程序）：

```c
int Morn_test1()
{
    MObject *jsondata=mObjectCreate();
    mFile(jsondata,"./citm_catalog.json");
    
    mTimerBegin("Morn Json");
    struct JSONNode *json = mJSONLoad(jsondata);
    int n=0;
    struct JSONNode *performances_array = mJSONRead(json,"performances");
    for(int i=0;i<performances_array->num;i++)
    {
        struct JSONNode *performances = mJSONRead(performances_array,i);
        struct JSONNode *seatCategories_array = mJSONRead(performances,"seatCategories");
        for(int j=0;j<seatCategories_array->num;j++)
        {
            struct JSONNode *seatCategories = mJSONRead(seatCategories_array,j);
            struct JSONNode *areas_array = mJSONRead(seatCategories,"areas");
            for(int k=0;k<areas_array->num;k++)
            {
                struct JSONNode *areas = mJSONRead(areas_array,k);
                struct JSONNode *areaId=mJSONRead(areas,"areaId");
                int id=areaId->dataS32;
                n++;
                // printf("id=%d\n",id);
            }
        }
    }
    mTimerEnd("Morn Json");

    mObjectRelease(jsondata);
    return n;
}

int test1()
{
    int n=Morn_test1();
    printf("get %d areaId\n\n",n);
}
```

其测试结果如下：

[![5pkJSA.png](https://z3.ax1x.com/2021/10/07/5pkJSA.png)](https://imgtu.com/i/5pkJSA)



#### 测试二

解析[canada.json](https://github.com/miloyip/nativejson-benchmark/blob/master/data/canada.json)，并读取文件中的坐标值，测试解析和读取速度。测试程序如下（仅节选Morn的测试程序）：

```c
int Morn_test2()
{
    MObject *jsondata=mObjectCreate();
    mFile(jsondata,"./canada.json");
    
    mTimerBegin("Morn json");
    struct JSONNode *json=mJSONLoad(jsondata);
    int n=0;
    struct JSONNode *coordinates0=mJSONRead(json,"features[0].geometry.coordinates");
    for (int j=0;j<coordinates0->num;j++)
    {
        struct JSONNode *coordinates1 = mJSONRead(coordinates0,j);
        for (int i=0;i<coordinates1->num;i++)
        {
            struct JSONNode *coordinates2 = mJSONRead(coordinates1,i);
            double x=mJSONRead(coordinates2,0)->dataD64;
            double y=mJSONRead(coordinates2,1)->dataD64;
            n++;
            // printf("x=%f,y=%f\n",x,y);
        }
    }
    mTimerEnd("Morn json");
    
    mObjectRelease(jsondata);
    return n;
}

void test2()
{
    int n=Morn_test2();
    printf("get %d coordinates\n\n",n);
}
```

其测试结果如下：

[![5puZqA.png](https://z3.ax1x.com/2021/10/07/5puZqA.png)](https://imgtu.com/i/5puZqA)

以上测试可见：rapidjson、yyjson、Morn的速度远快于其它（cjson在测试一速度尚可，但测试二速度最慢），而yyjson和Morn又显著的快于rapidjson。



#### 测试三

比较Morn与rapidjson和yyjson，在多种json文件中的解析速度。后两者都是以快速解析而著称。

测试解析的文件包括：canada.json、citm_catalog.json、[twitter.json](https://github.com/chadaustin/sajson/blob/master/testdata/twitter.json)、[github_events.json](https://github.com/chadaustin/sajson/blob/master/testdata/github_events.json)、[apache_builds.json](https://github.com/chadaustin/sajson/blob/master/testdata/apache_builds.json)、[mesh.json](https://github.com/chadaustin/sajson/blob/master/testdata/mesh.json)、[mesh.pretty.json](https://github.com/chadaustin/sajson/blob/master/testdata/mesh.pretty.json)、[update-center.json](https://github.com/chadaustin/sajson/blob/master/testdata/update-center.json)

程序中对上述文件分别进行了100次解析，并计时。

测试程序如下：

```c
void rapidjson_test3(const char *filename,int n)
{
    MString *jsondata=mObjectCreate();
    mTimerBegin("rapidjson");
    for(int i=0;i<n;i++)
    {
        mFile(jsondata,filename);
        rapidjson::Document doc;
        doc.Parse(jsondata->string);
    }
    mTimerEnd("rapidjson");
    mObjectRelease(jsondata);
}

void yyjson_test3(const char *filename,int n)
{
    MString *jsondata=mObjectCreate();
    mTimerBegin("yyjson");
    for(int i=0;i<n;i++)
    {
        mFile(jsondata,filename);
        yyjson_doc_get_root(yyjson_read(jsondata->string,jsondata->size-1,0));
    }
    mTimerEnd("yyjson");
    mObjectRelease(jsondata);
}

void Morn_test3(const char *filename,int n)
{
    MString *jsondata=mObjectCreate();
    mTimerBegin("Morn json");
    for(int i=0;i<n;i++)
    {
        mFile(jsondata,filename);
        mJSONLoad(jsondata);
    }
    mTimerEnd("Morn json");
    mObjectRelease(jsondata);
}

void test3()
{
    const char *filename;

    filename = "./canada.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./citm_catalog.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./testdata/twitter.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./testdata/github_events.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./testdata/apache_builds.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./testdata/mesh.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./testdata/mesh.pretty.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);

    filename = "./testdata/update-center.json";
    printf("\nfor %s:\n",filename);
    rapidjson_test3(filename,100);
    yyjson_test3(filename,100);
    Morn_test3(filename,100);
}
```

测试结果如下：

[![45IDJA.png](https://z3.ax1x.com/2021/09/29/45IDJA.png)](https://imgtu.com/i/45IDJA)

可见：**Morn和yyjson显著快于rapidjson（2至4倍），Morn与yyjson速度相当。**

