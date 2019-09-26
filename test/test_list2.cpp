/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： g++ -O2 -fopenmp test_list2.cpp -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_list2.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

#define TEST_NUM 1000000
int main()
{
    int i,j;
    char *data=(char *)malloc(TEST_NUM*32*sizeof(char));
    for(i=0;i<TEST_NUM;i++)
    {
        int size = mRand(10,31);
        for(j=0;j<size;j++)
            data[i*32+j] = mRand('a','z');
        data[i*32+j]=0;
    }
    
    vector<string> vec;
    MList *list = mListCreate(DFLT,NULL);
    
    mLog(INFO,"STL vector写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        vec.push_back(&(data[i*32]));
    mTimerEnd();
    
    mLog(INFO,"Morn MList写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        mListWrite(list,DFLT,&(data[i*32]),DFLT);
    mTimerEnd();
    
    mLog(INFO,"STL vector读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,vec.size());
        const char *p=vec.at(idx).data();
    }
    mTimerEnd();
    
    mLog(INFO,"Morn MList读出：");
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
