/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_JSON_file.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_JSON_file.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

int main()
{
    MTree *json=mTreeCreate();
    mJSONLoad(json,"./test_JSON_file.json");

    MTreeNode *node = json->treenode->child[0];
    for(int i=0;i<node->child_num;i++)
        printf("%s是%s\n",mJSONName(node->child[i]),mJSONValue(node->child[i]));
    printf("\n");
    
    MList *list = mListCreate(DFLT,NULL);
    
    mJSONSearch(json,list,"班主任");
    for(int i=0;i<list->num;i++) printf("班主任%d是%s\n",i,list->data[i]);
    printf("\n");

    mJSONSearch(json,list,"任课老师");
    for(int i=0;i<list->num;i++) printf("任课老师%d是%s\n",i,list->data[i]);
    printf("\n");

    MList *name = mListCreate(DFLT,NULL);mJSONSearch(json,name,"学生.姓名");
    MList *score= mListCreate(DFLT,NULL);mJSONSearch(json,score,"学生.成绩.数学");
    for(int i=0;i<name->num;i++) printf("%s的数学成绩是%d\n",name->data[i],atoi(score->data[i]));
    mListRelease(name);
    mListRelease(score);
    printf("\n");

    struct Student
    {
        char *name;
        char *sex;
        char *course[3];
        int score[3];
    };
    int func1(MTreeNode *ptr,void *para) {return (strcmp(mJSONName(ptr),para)==0);}
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
                student.name,student.sex,student.course[0],student.score[0],student.course[1],student.score[1],student.course[2],student.score[2]);
    }
    printf("\n");

    int func2(MTreeNode *ptr,void *para)
    {return ((strcmp(mJSONName(ptr),"数学")==0)&&(atoi(mJSONValue(ptr))>=90));}
    node = json->treenode;
    while(1)
    {
        node = mTreeSearch(node,func2,NULL,0);
        if(node == NULL) break;
        printf("姓名%s\n",mJSONValue(node->parent->parent->child[0]));
    }

    mListRelease(list);
    mTreeRelease(json);
    return 0;
}
    