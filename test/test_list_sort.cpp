/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： g++ -O2 -fopenmp test_list_sort.cpp -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_list_sort.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
    printf("student mean score sort by STL:\n");
    mTimerBegin();
    sort(vec.begin(),vec.end(),stl_student_compare);  
    mTimerEnd();
    
    printf("student mean score sort by Morn:\n");
    mTimerBegin();
    mListSort(list,morn_student_compare,NULL);
    mTimerEnd();
    
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
    
    printf("rect area sort by stl:\n");
    mTimerBegin();
    sort(vec.begin(),vec.end(),stl_rect_compare);  
    mTimerEnd();
    
    printf("rect area sort by morn:\n");
    mTimerBegin();
    mListSort(list,morn_rect_compare,NULL);
    mTimerEnd();
    
    // for(int i=50000;i<50010;i++) printf("%d,",mRectArea(&(vec.at(i)))); printf("\n");
    // for(int i=50000;i<50010;i++) printf("%d,",mRectArea(list->data[i])); printf("\n");
    
    mListRelease(list);
}

int main()
{
    test1();
    test2();
    return 0;
}
    