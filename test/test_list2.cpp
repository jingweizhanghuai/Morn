/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//build£º g++ -O2 -DNDEBUG -fopenmp test_list2.cpp -o test_list2.exe -lmorn
#include "morn_util.h"

#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

#define TEST_NUM 10000000

void test_int()
{
    int i;
    int *data=(int *)malloc(TEST_NUM*sizeof(int));
    for(i=0;i<TEST_NUM;i++) data[i]=mRand();

    vector<int> vec;
    MList *list = mListCreate();
    
    mTimerBegin("STL vector write");
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(data[i]);
    mTimerEnd("STL vector write");
    
    mTimerBegin("Morn MList write");
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,DFLT,data+i,sizeof(int));
    mTimerEnd("Morn MList write");

    mTimerBegin("STL vector read");
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        int v=vec.at(idx);
    }
    mTimerEnd("STL vector read");
    
    mTimerBegin("Morn MList read");
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,list->num);
        int v=*(int *)(list->data[i]);
    }
    mTimerEnd("Morn MList read");
    
    mListRelease(list);
    free(data);
}

struct Test
{
    char    data0[32];
    int     data1;
    float   data2;
    double  data3;
    int    *data4;
    float  *data5;
    double *data6;
};

void test_struct()
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

    mTimerBegin("STL vector write");
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(data[i]);
    mTimerEnd("STL vector write");
    
    mTimerBegin("Morn MList write");
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,i,data+i,sizeof(struct Test));
    mTimerEnd("Morn MList write");

    mTimerBegin("STL vector read");
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        struct Test *v=&(vec.at(idx));
    }
    mTimerEnd("STL vector read");
    
    mTimerBegin("Morn MList read");
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,list->num);
        struct Test *v=(struct Test *)(list->data[i]);
    }
    mTimerEnd("Morn MList read");
    
    mListRelease(list);
    free(data);
}

void test_string()
{
    int i,j;
    char *data=(char *)malloc(TEST_NUM*128*sizeof(char));
    for(i=0;i<TEST_NUM;i++) mRandString(data+i*128,1,127);
    
    vector<string> vec;
    MList *list = mListCreate(DFLT,NULL);
    
    mTimerBegin("STL vector write");
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(&(data[i*32]));
    mTimerEnd("STL vector write");
    
    mTimerBegin("Morn MList write");
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,DFLT,&(data[i*32]),DFLT);
    mTimerEnd("Morn MList write");
    
    mTimerBegin("STL vector read");
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        const char *p=vec.at(idx).data();
    }
    mTimerEnd("STL vector read");
    
    mTimerBegin("Morn MList read");
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,list->num);
        const char *p=(const char *)(list->data[i]);
    }
    mTimerEnd("Morn MList read");
    
    mListRelease(list);
    free(data);
}

int main()
{
    printf("\ninteger test:\n");
    test_int();
    
    printf("\nstruct test:\n");
    test_struct();

    printf("\nstring test:\n");
    test_string();
}


