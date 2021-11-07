/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this json except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//build: gcc -O2 test_JSON_file.c -lmorn -o test_JSON_file.exe

#include "morn_util.h"

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
    printf("pi=%f\n",*pi);

    node = mJSONRead(json,"minimum");
    printf("maximum node_type=%s,value=%f\n",jsontype[node->type],node->dataD64);
    node = mJSONRead(json,"maximum");
    printf("maximum node_type=%s,value=%f\n",jsontype[node->type],node->dataD64);
    node = mJSONRead(json,"not_a_number");
    printf("not_a_number node_type=%s,value=%f\n",jsontype[node->type],node->dataD64);
    
    node = mJSONRead(json,"n");
    printf("type=%s,nul=%p\n",jsontype[node->type],node->string);

    node = mJSONRead(json,"esc");
    printf("esc=%s\n",node->string);
    
    node = mJSONRead(json,"unicode");
    printf("unicode=%s\n",node->string);

    node = mJSONRead(json,"comment1");
    printf("comment1 = %s\n",node->string);
    node = mJSONRead(json,"comment2");
    printf("comment2 = %s\n",node->string);
    
    node=mJSONRead(json,"date");
    struct JSONNode *year=mJSONRead(node,"year");
    printf("date.year=%d,type=%s\n",year->dataS32,mJSONNodeType(year));
    struct JSONNode *month=mJSONRead(node,"month");
    printf("date.month=%d,type=%s\n",month->dataS32,mJSONNodeType(month));
    struct JSONNode *day=mJSONRead(node,"day");
    printf("date.day=%d,type=%s\n",day->dataS32,mJSONNodeType(day));

    year=mJSONRead(json,"date.year");
    printf("date.year=%d,type=%s\n",year->dataS32,mJSONNodeType(year));
    month=mJSONRead(json,"date.month");
    printf("date.month=%d,type=%s\n",month->dataS32,mJSONNodeType(month));
    day=mJSONRead(json,"date.day");
    printf("date.day=%d,type=%s\n",day->dataS32,mJSONNodeType(day));
    
    node = mJSONRead(json,"a1[0]");
    printf("a1[0]=%d\n",node->dataS32);
    node = mJSONRead(json,"a1[1]");
    printf("a1[1]=%d\n",node->dataS32);
    node = mJSONRead(json,"a1[2]");
    printf("a1[2]=%d\n",node->dataS32);
    node = mJSONRead(json,"a1[3]");
    printf("a1[3]=%d\n",node->dataS32);
    
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

    node = mJSONRead(json,"a2[1][2]");
    printf("a2[1][2]=%d\n",node->dataS32);

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
    
    node = mJSONRead(json,"province.Hebei[0]");printf("%s\n",node->string);
    node = mJSONRead(json,"province.Anhui[0]");printf("%s\n",node->string);
    node = mJSONRead(json,"province.Gansu"   );printf("%s\n",node->string);
    
    mFileRelease(file);
}

