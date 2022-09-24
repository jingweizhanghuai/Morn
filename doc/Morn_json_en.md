## JSON

 Morn provides library for parsing JSON files. JSON parsing with Morn is **simple** (with only two interfaces) and **fast** (much faster than RapidJSON) .   

This is a typical JSON file (which we will take as an example) :

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

A JSON string is formed by JSON nodes which combined  according rules. The JSON node is the basic structure in Morn, These nodes have different types, such as  key-value or single value. in Morn we define these types as:

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

JSON Node is defined as:

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



### Interface

#### Load and Parse JSON

```c
struct JSONNode *mJSONLoad(MFile *jsonfile);
struct JSONNode *mJSONLoad(MString *jsondata);
```

With the input is a JSON file or a JSON string, the output is the parsed top-level JSON node.

This is a simple example：

```c
MFile *file = mFileCreate("./test_json.json");
struct JSONNode *json=mJSONLoad(file);
...
mFileRelease(file);
```

or:

```c
MString *string = mStringCreate("{\"hello\":\"world\",\"t\":true,\"i\":123}");
struct JSONNode *json=mJSONLoad(string);
...
mStringRelease(string);
```

 For file parsing, you can use `mJSONLoad` directly, or you can read the file and parse as string.   



#### Read JSON Node

```c
struct JSONNode *mJSONRead(struct JSONNode *node);
struct JSONNode *mJSONRead(struct JSONNode *node,int n);
struct JSONNode *mJSONRead(struct JSONNode *node,const char *key);
struct JSONNode *mJSONRead(struct JSONNode *node,struct JSONNode *dst);
struct JSONNode *mJSONRead(struct JSONNode *node,int n,struct JSONNode *dst);
struct JSONNode *mJSONRead(struct JSONNode *node,const char *key,struct JSONNode *dst);
```

The input node must with type of list(`JSON_LIST` / `JSON_KEY_LIST`）or array(`JSON_ARRAY `/`JSON_KEY_ARRAY`)，The return value is NULL on read failure.

This interface has three forms:

```c
struct JSONNode *child;
child = mJSONRead(mother);          //mother is list or array, read the first node of mother
child = mJSONRead(mother,5);        //mother is list or array, read the fifth node of mother
child = mJSONRead(mother,"[5]");    //mother is array, read the fifth node of mother
child = mJSONRead(mother,"child5"); //mother is list, read the node with key is "child5"
child = mJSONRead(mother,"a.b[3].c.d[6]");   //read further child node
```

or:

```c
struct JSONNode child;
mJSONRead(mother,&child);           //mother is list or array, read the first node of mother
mJSONRead(mother,5,&child);         //mother is list or array, read the fifth node of mother
mJSONRead(mother,"[5]",&child);     //mother is array, read the fifth node of mother
mJSONRead(mother,"child5",&child);  //mother is list, read the node with key is "child5"
mJSONRead(mother,"a.b[3].c.d[6]",&child);    //read further child node

```



### Example

The full example file is [test_JSON_file.c](https://github.com/jingweizhanghuai/Morn/blob/master/test/test_JSON_file.c)

Taking the JSON file at the beginning of this article as an example, you can read it using the following program:   

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

in this example two node has been read, root node  and "hello" node. the output is:

```
json->type=LIST
json->num=13
node->type=KEY_STRING
node->key=hello
node->string=world

```



Node read can be written in following forms as required:

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

the output is:

```
t=1
f=0
i=123
pi=3.141592

```



 Note here that  `nul`  will be understood as null string:

```c
node = mJSONRead(json,"n");
printf("type=%s,nul=%p\n",jsontype[node->type],node->string);

```

the output is:

```
type=KEY_STRING,nul=0000000000000000
```



For further child node, it can be read layer by layer, for example: 

```c
node=mJSONRead(json,"date");
struct JSONNode *year=mJSONRead(node,"year");
printf("date.year=%d,type=%s\n",year->dataS32,mJSONNodeType(year));
struct JSONNode *month=mJSONRead(node,"month");
printf("date.month=%s,type=%s\n",month->dataS32,mJSONNodeType(month));
struct JSONNode *day=mJSONRead(node,"day");
printf("date.day=%d,type=%s\n",day->dataS32,mJSONNodeType(day));

```

Or it can be read cross layers: 

```c
struct JSONNode *year=mJSONRead(json,"date.year");
printf("date.year=%d,type=%s\n",year->dataS32,mJSONNodeType(year));
struct JSONNode *month=mJSONRead(json,"date.month");
printf("date.month=%s,type=%s\n",month->dataS32,mJSONNodeType(month));
struct JSONNode *day=mJSONRead(json,"date.day");
printf("date.day=%d,type=%s\n",day->dataS32,mJSONNodeType(day));

```

Output of these above two programs is:

```
date.year=2021,type=KEY_INT
date.month=June,type=KEY_STRING
date.day=5,type=KEY_INT
```



There are several flexible forms for reading node from arrays:

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

Output is:

```
a1[0]=0
a1[1]=1
a1[2]=2
a1[3]=3
```



You can read multidimensional array as  further child with cross layers read:

```c
node = mJSONRead(json,"a2[1][2]");
```

And also can be read layer by layer:

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

Output is:

```
00,01,02,03,
10,11,12,13,
20,21,22,23,
```



Node can also be read from mixed list and array:

```c
node = mJSONRead(json,"province.Hebei[0]");
printf("%s\n",node->string);
node = mJSONRead(json,"province.Anhui[0]");
printf("%s\n",node->string);
node = mJSONRead(json,"province.Gansu"   );
printf("%s\n",node->string);

```

Output is:

```
Shijiazhuang
Hefei
Lanzhou
```



### Performance

 Complete test file is [test_JSON_file2.cpp](https://github.com/jingweizhanghuai/Morn/blob/master/test/test_JSON_file2.cpp)

 In this performance test, Morn is compared with: [cjson](https://github.com/DaveGamble/cJSON)、[jsoncpp](https://github.com/open-source-parsers/jsoncpp)、[nlohmann](https://github.com/nlohmann/json)、[rapidjson](https://github.com/Tencent/rapidjson)、[yyjson](https://github.com/ibireme/yyjson)

#### Test 1

Testing parse [citm_catalog.json](https://github.com/miloyip/nativejson-benchmark/blob/master/data/citm_catalog.json), and read the "areaId", then measure time-consume of parse and read. This is a part of the program (using Morn)：

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

Result is:

[![5KQ2Is.png](https://z3.ax1x.com/2021/10/13/5KQ2Is.png)](https://imgtu.com/i/5KQ2Is)



#### Test 2

Testing parse [canada.json](https://github.com/miloyip/nativejson-benchmark/blob/master/data/canada.json) and read all of coordinates, then measure time-consume of parse and read. This is a part of the program (using Morn)：

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

Result is:

[![5KQWin.png](https://z3.ax1x.com/2021/10/13/5KQWin.png)](https://imgtu.com/i/5KQWin)

Thus it can be seen: rapidjson/yyjson/Morn is much faster than other json library (cjson is OK in Test 1,but is slowest in test 2), and yyjson and Morn is faster than rapidjson.



#### Test 3

Comparing the performance of rapidjson yyjson and Morn with many different json file. rapidjson and yyjson are known for high performance JSON parse.

The testing file are: canada.json citm_catalog.json [twitter.json](https://github.com/chadaustin/sajson/blob/master/testdata/twitter.json) [github_events.json](https://github.com/chadaustin/sajson/blob/master/testdata/github_events.json) [apache_builds.json](https://github.com/chadaustin/sajson/blob/master/testdata/apache_builds.json) [mesh.json](https://github.com/chadaustin/sajson/blob/master/testdata/mesh.json) [mesh.pretty.json](https://github.com/chadaustin/sajson/blob/master/testdata/mesh.pretty.json) and [update-center.json](https://github.com/chadaustin/sajson/blob/master/testdata/update-center.json)

In the program we parse each of these files for 100 times and measure the time consume.

Testing program is:

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

Result is:

[![5KK5Yq.png](https://z3.ax1x.com/2021/10/13/5KK5Yq.png)](https://imgtu.com/i/5KK5Yq)

Thus it can be seen: Morn and yyjson are much faster then rapidjson with 2 to 5 times, in most cases Morn is faster then yyjson.

