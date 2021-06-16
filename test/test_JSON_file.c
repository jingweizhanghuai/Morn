/*
Copyright (C) 2019-2021 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this json except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//编译： gcc -O2 -fopenmp test_JSON_file.c -lmorn -o test_JSON_file.exe

#include "morn_util.h"

int main()
{
    MArray *json = mArrayCreate();
    JSONLoad(json,"./test_json.json");

    struct JSONNode *node;

    node=mJSONRead(json,"t");
    if(node->type==JSON_BOOL)
    {
        printf("t=%d\n",node->value_b);
    }

    node=mJSONRead(json,"f");
    if(node!=NULL) printf("f=%d\n",node->value_b);

    int i=*(int *)mJSONRead(json,"i");
    printf("i=%d\n",i);
    double *pi=(double *)mJSONRead(json,"pi");
    printf("pi=%f\n",*pi);
    

    node = mJSONRead(json,"n");
    printf("type=%d,nul=%p\n",node->type,node->string);

    node=mJSONRead(json,"date");
    struct JSONNode *year=mJSONRead(node,"year");
    printf("date.year=%d,type=%s\n",year->value_i,mJSONNodeType(year));
    struct JSONNode *month=mJSONRead(node,"month");
    printf("date.month=%s,type=%s\n",month->value_i,mJSONNodeType(month));
    struct JSONNode *day=mJSONRead(node,"day");
    printf("date.day=%d,type=%s\n",day->value_i,mJSONNodeType(day));

    year=mJSONRead(json,"date.year");
    printf("date.year=%d,type=%s\n",year->value_i,mJSONNodeType(year));
    month=mJSONRead(json,"date.month");
    printf("date.month=%s,type=%s\n",month->value_i,mJSONNodeType(month));
    day=mJSONRead(json,"date.day");
    printf("date.day=%d,type=%s\n",day->value_i,mJSONNodeType(day));

    node=mJSONRead(json,"date");
    MArray *date=mArrayCreate();
    mJSONArray(date,node);
    struct JSONNode *p=date->data;
    printf("date->num=%d\n",date->num);
    printf("p[0] key=%s, data=%d, type=%s\n",p[0].key,p[0].value_i,mJSONNodeType(p+0));
    printf("p[1] key=%s, data=%s, type=%s\n",p[1].key,p[1].string ,mJSONNodeType(p+1));
    printf("p[2] key=%s, data=%d, type=%s\n",p[2].key,p[2].value_i,mJSONNodeType(p+2));


    node = mJSONRead(json,"a1[0]");
    printf("a1[0]=%d\n",node->value_i);
    node = mJSONRead(json,"a1[1]");
    printf("a1[1]=%d\n",node->value_i);
    node = mJSONRead(json,"a1[2]");
    printf("a1[2]=%d\n",node->value_i);
    node = mJSONRead(json,"a1[3]");
    printf("a1[3]=%d\n",node->value_i);

    node=mJSONRead(json,"a1");
    p = mJSONRead(node,"[0]");
    printf("a1[0]=%d\n",p->value_i);
    p = mJSONRead(node,"[1]");
    printf("a1[1]=%d\n",p->value_i);
    p = mJSONRead(node,"[2]");
    printf("a1[2]=%d\n",p->value_i);
    p = mJSONRead(node,"[3]");
    printf("a1[3]=%d\n",p->value_i);
    
    node = mJSONRead(json,"a1");
    MArray *a1=mArrayCreate();
    mJSONArray(a1,node);
    p=a1->data;
    for(int i=0;i<a1->num;i++)
        printf("a1[%d]=%d\n",i,p[i].value_i);

    node = mJSONRead(json,"a2[1][2]");
    printf("a2[1][2]=%d\n",node->value_i);

    // node = mJSONRead(json,"a1");
    // MArray *a1=mArrayCreate();
    // mJSONArray(a1,node);
    // p=a1->data;
    // for(int i=0;i<a1->num;i++)

/*
    int *year = mJSONRead(date,"year");
    printf("year=%d\n",*year);
    int *day = mJSONRead(date,"day");
    printf("day=%d\n",*day);
    mArrayRelease(date);

    char *month = mJSONRead(json,"date.month");
    printf("month=%s\n",month);

    MArray *a1 = mArrayCreate();
    mJSONArray(json,a1,"a1");
    int *a1_1 = mJSONRead(a1,"[1]");
    printf("a1_1=%d\n",*a1_1);
    int *a1_2 = mJSONRead(a1,2);
    printf("a1_2=%d\n",*a1_2);
    
    int *a1_3 = mJSONRead(json,"a1[3]");
    printf("a1_3=%d\n",*a1_3);
    mArrayRelease(a1);

    ///////////////////////////////////////////////////////////
    int *a2_12 = mJSONRead(json,"a2[1][2]");
    printf("a2_12=%d\n",*a2_12);

    int *a3_210 = mJSONRead(json,"a3[2][1][0]");
    printf("a3_210=%d\n",*a3_210);

    ///////////////////////////////////////////////////////////
    int *a4_1 = mJSONRead(json,"a4[1].value");
    printf("a4_1=%d\n",*a4_1);

    char *str1 = mJSONRead(json,"city[0].Beijing[2]");
    printf("str1 = %s\n",str1);

    char *str2 = mJSONRead(json,"province.Anhui[0]");
    printf("str2 = %s\n",str2);
    
    ///////////////////////////////////////////////////////////
    

    type=JSON_INT64;
    int i;mJSONRead(json,"i",&i,&type);
    if(type==JSON_INT64) printf("i=%d\n",i);
    else                  printf("error");

    type=DFLT;
    double *pi = mJSONRead(json,"pi",&type);
    if(type==JSON_DOUBLE) printf("pi=%f\n",*pi);
    else                  printf("error");

    
    


    

    MArray *a1 = mJSONRead(json,"a1");
    int *a1_1 = mJSONRead(a1,"[1]");
    printf("a1_1=%d\n",*a1_1);

    int *a1_2 = mJSONRead(json,"a1[2]");
    printf("a1_2=%d\n",*a1_2);

    struct JSONArrayNode *p_a1 = a1->data;
    for(int i=0;i<a1->num;i++)
        printf("a1_%d=%d\n",i,p_a1[i].value_i);

    


    

    int day=6;
    day=*(int *)mJSONWrite(json,"date.day",&day,JSON_INT64);
    printf("day=%d\n",day);

    int data=40;
    int *a1_0=mJSONWrite(a1,"[0]",&data,JSON_INT64);
    printf("a1_0=%d\n",*a1_0);

    for(int i=1;i<a1->num;i++)
    {
        p_a1[i].value_i+=40;
        printf("a1_%d=%d\n",i,p_a1[i].value_i);
    }

    char *city = "Chongqing";
    mJSONWrite(json,"city[3]",city,JSON_STRING);
    char *str3 = mJSONRead(json,"city[3]");
    printf("str3 = %s\n",str3);

    mJSONWrite(json,"a2[3]",a1,JSON_ARRAY);

    MArray *gansu = mArrayCreate(3,sizeof(struct JSONArrayNode));
    struct JSONArrayNode *node = gansu->data;
    node[0].type = JSON_STRING; node[0].string="Lanzhou";
    node[1].type = JSON_STRING; node[1].string="Tianshui";
    node[2].type = JSON_STRING; node[2].string="Jiuquan";
    mJSONWrite(json,"province.Gansu",gansu,JSON_ARRAY);

    mJSONNodeDelete(json,"hello");
    mJSONNodeDelete(json,"a1[2]");
    mJSONNodeDelete(json,"a4");

    mJSONSave(json,"./test_json_out.json");
*/
    mArrayRelease(json);
}


    





// int main()
// {
//     MList *list = mListCreate();
//     mJSONLoad(list,"./china.json");
//     // printf("aaaaaaa %s\n",list->data[0]);

//     int type;
//     char *str = mJSONData(list,"type",&type);
//     printf("str=%s,type=%d\n",str,type);

//     str = mJSONData(list,"features[0].type",&type);
//     printf("str=%s,type=%d\n",str,type);

//     str = mJSONData(list,"features[1].properties.name",&type);
//     printf("str=%s,type=%d\n",str,type);

//     int n = *(int *)(mJSONData(list,"features[1].properties.childNum",&type));
//     printf("n=%d,type=%d\n",n,type);

//     MArray *coordinates = mJSONData(list,"features[0].geometry.coordinates[0][2]",&type);
//     printf("coordinates->num=%d,type=%d,tttt=%d\n",coordinates->num,type,coordinates->i_reserve);
//     for(int i=0;i<coordinates->num;i++) printf("%f\n",coordinates->dataD64[i]);
    
//     mListRelease(list);
// }

// int main()
// {
//     MTree *json=mTreeCreate();
//     mJSONLoad(json,"./test_JSON_json.json");

//     MTreeNode *node = json->treenode->child[0];
//     for(int i=0;i<node->child_num;i++)
//         printf("%s=%s\n",mJSONName(node->child[i]),mJSONValue(node->child[i]));
//     printf("\n");
    
//     MList *list = mListCreate(DFLT,NULL);
    
//     mJSONSearch(json,list,"班主任");
//     for(int i=0;i<list->num;i++) printf("班主任%d是%s\n",i,list->data[i]);
//     printf("\n");

//     mJSONSearch(json,list,"任课老师");
//     for(int i=0;i<list->num;i++) printf("任课老师%d是%s\n",i,list->data[i]);
//     printf("\n");

//     MList *name = mListCreate(DFLT,NULL);mJSONSearch(json,name,"学生.姓名");
//     MList *score= mListCreate(DFLT,NULL);mJSONSearch(json,score,"学生.成绩.数学");
//     for(int i=0;i<name->num;i++) printf("%s的数学成绩是%d\n",name->data[i],atoi(score->data[i]));
//     mListRelease(name);
//     mListRelease(score);
//     printf("\n");

//     struct Student
//     {
//         char *name;
//         char *sex;
//         char *course[3];
//         int score[3];
//     };
//     int func1(MTreeNode *ptr,void *para) {return (strcmp(mJSONName(ptr),para)==0);}
//     node = json->treenode;  // 从树根开始搜索
//     while(1)
//     {
//         struct Student student;
//         MTreeNode *student_node = mTreeSearch(node,func1,"学生",0);
//         if(student_node == NULL) break;
//         node = mTreeSearch(student_node,func1,"姓名",0);
//         student.name = mJSONValue(node);
//         node = mTreeSearch(student_node,func1,"性别",0);
//         student.sex  = mJSONValue(node);
//         node = mTreeSearch(student_node,func1,"成绩",0);
//         student.course[0] = mJSONName(node->child[0]);
//         student. score[0] = atoi(mJSONValue(node->child[0]));
//         student.course[1] = mJSONName(node->child[1]);
//         student. score[1] = atoi(mJSONValue(node->child[1]));
//         student.course[2] = mJSONName(node->child[2]);
//         student. score[2] = atoi(mJSONValue(node->child[2]));
//         printf("student name is %s,sex is %s,course0 is %s,score0 is %d,course1 is %s,score1 is %d,course2 is %s,score2 is %d\n",
//                 student.name,student.sex,student.course[0],student.score[0],student.course[1],student.score[1],student.course[2],student.score[2]);
//     }
//     printf("\n");

//     int func2(MTreeNode *ptr,void *para)
//     {return ((strcmp(mJSONName(ptr),"数学")==0)&&(atoi(mJSONValue(ptr))>=90));}
//     node = json->treenode;
//     while(1)
//     {
//         node = mTreeSearch(node,func2,NULL,0);
//         if(node == NULL) break;
//         printf("姓名%s\n",mJSONValue(node->parent->parent->child[0]));
//     }

//     mListRelease(list);
//     mTreeRelease(json);
//     return 0;
// }
    