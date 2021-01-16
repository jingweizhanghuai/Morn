/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -O2 -fopenmp test_list_sort.cpp -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -o test_list_sort.exe

#include "morn_image.h"

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

#define N 100000

struct Student
{
    char name[64];
    int ID;
    int sex;
    int Grade;
    int Class;
    int score[16];
    int mean;
};

bool stl_student_compare(const struct Student& student1, const struct Student& student2)
{return (student1.mean<student2.mean);}

int morn_student_compare(void *student1,void *student2,void *para) 
{return (((const struct Student *)student1)->mean-((const struct Student *)student2)->mean);}  

void test1()
{
    vector<struct Student> vec;
    MList *list=mListCreate(DFLT,NULL);
    
    for(int i=0;i<N;i++)
    {
        struct Student student;
        for(int i=0;i<16;i++) {student.score[i]=mRand(0,10000);student.mean+=student.score[i];}
        student.mean=(student.mean+8)/16;
        
        vec.push_back(student);
        mListWrite(list,DFLT,&student,sizeof(struct Student));
    }
    
    mTimerBegin("stl");
    sort(vec.begin(),vec.end(),stl_student_compare);  
    mTimerEnd("stl");
    
    mTimerBegin("Morn");
    mListSort(list,(void *)morn_student_compare,NULL);
    mTimerEnd("Morn");
    
    // for(int i=50000;i<50010;i++) printf("%d,",(vec.at(i)).mean); printf("\n");
    // for(int i=50000;i<50010;i++) printf("%d,",((struct Student *)(list->data[i]))->mean); printf("\n");
    
    mListRelease(list);
}
    
bool stl_rect_compare(const MImageRect& rect1, const MImageRect& rect2) 
{return (mRectArea(&rect1)<mRectArea(&rect2));}

int morn_rect_compare(void *rect1,void *rect2,void *para) 
{return (mRectArea(rect1)-mRectArea(rect2));}

void test2()
{
    vector<MImageRect> vec;
    MList *list=mListCreate(DFLT,NULL);
    
    for(int i=0;i<N;i++)
    {
        MImageRect rect;
        rect.x1=mRand(0,10000);rect.y1=mRand(0,10000);
        rect.x2=rect.x1+mRand(0,10000);rect.y2=rect.y1+mRand(0,10000);
        
        vec.push_back(rect);
        
        mListWrite(list,DFLT,&rect,sizeof(MImageRect));
    }
    
    mTimerBegin("stl");
    sort(vec.begin(),vec.end(),stl_rect_compare);  
    mTimerEnd("stl");
    
    mTimerBegin("Morn");
    mListSort(list,(void *)morn_rect_compare,NULL);
    mTimerEnd("Morn");
    
    // for(int i=50000;i<50010;i++) printf("%d,",mRectArea(&(vec.at(i)))); printf("\n");
    // for(int i=50000;i<50010;i++) printf("%d,",mRectArea(list->data[i])); printf("\n");
    
    mListRelease(list);
}

int main()
{
    printf("student mean score sort:\n");
    test1();

    printf("rect area sort:\n");
    test2();
    
    return 0;
}
