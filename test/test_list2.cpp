/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： g++ -O2 -fopenmp test_list2.cpp -I ..\include\ -L ..\lib\x64_mingw -lmorn -o test_list2.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

#define TEST_NUM 1000000

int main_int()
{
    int i;
    int *data=(int *)malloc(TEST_NUM*sizeof(int));
    for(i=0;i<TEST_NUM;i++) data[i]=mRand(DFLT,DFLT);

    vector<int> vec;
    MList *list = mListCreate();
    
    mLog(MORN_INFO,"STL vector写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(data[i]);
    mTimerEnd();
    
    mLog(MORN_INFO,"Morn MList写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,DFLT,data+i,sizeof(int));
    mTimerEnd();

    mLog(MORN_INFO,"STL vector读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        int v=vec.at(idx);
    }
    mTimerEnd();
    
    mLog(MORN_INFO,"Morn MList读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,list->num);
        int v=*(int *)(list->data[i]);
    }
    mTimerEnd();
    
    mListRelease(list);
    free(data);
    return 0;
}

struct Test
{
    int     data1;
    float   data2;
    double  data3;
    int    *data4;
    float  *data5;
    double *data6;
};

int main_struct()
{
    int i;
    struct Test *data=(struct Test *)malloc(TEST_NUM*sizeof(struct Test));
    for(i=0;i<TEST_NUM;i++)
    {
        data[i].data1=mRand(-10000,10000);
        data[i].data2=(float)mRand(-10000,10000)/100000.0f;
        data[i].data3=(double)mRand(-10000,10000)/100000.0f;
        data[i].data4=(int *)&(data[i].data1);
        data[i].data5=(float *)&(data[i].data2);
        data[i].data6=(double *)&(data[i].data3);
    }

    vector<struct Test> vec;
    MList *list = mListCreate();

    mLog(MORN_INFO,"STL vector写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(data[i]);
    mTimerEnd();
    
    mLog(MORN_INFO,"Morn MList写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,i,data+i,sizeof(struct Test));
    mTimerEnd();

    mLog(MORN_INFO,"STL vector读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        struct Test *v=&(vec.at(idx));
    }
    mTimerEnd();
    
    mLog(MORN_INFO,"Morn MList读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,list->num);
        struct Test *v=(struct Test *)(list->data[i]);
    }
    mTimerEnd();
    
    mListRelease(list);
    free(data);
    return 0;
}

int main_string()
{
    int i,j;
    char *data=(char *)malloc(TEST_NUM*128*sizeof(char));
    for(i=0;i<TEST_NUM;i++)
    {
        int size = mRand(1,127);
        for(j=0;j<size;j++)
            data[i*128+j] = mRand('a','z');
        data[i*128+j]=0;
    }
    
    vector<string> vec;
    MList *list = mListCreate(DFLT,NULL);
    
    mLog(MORN_INFO,"STL vector写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(&(data[i*32]));
    mTimerEnd();
    
    mLog(MORN_INFO,"Morn MList写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,DFLT,&(data[i*32]),DFLT);
    mTimerEnd();
    
    mLog(MORN_INFO,"STL vector读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        const char *p=vec.at(idx).data();
    }
    mTimerEnd();
    
    mLog(MORN_INFO,"Morn MList读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,list->num);
        const char *p=(const char *)(list->data[i]);
    }
    mTimerEnd();
    
    mListRelease(list);
    free(data);
    return 0;
}

int main()
{
    main_int();
    main_struct();
    main_string();
}


