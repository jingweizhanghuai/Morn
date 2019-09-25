/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// g++ -O2 -fopenmp test_map2.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_map2.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

#include <map>
#include <string>
#include <iostream>

using namespace std;

#define TEST_NUM 100000
int main()
{
    int i,j;
    
    char *key=(char *)malloc(TEST_NUM*32*sizeof(char));
    for(i=0;i<TEST_NUM;i++)
    {
        int size = mRand(10,31);
        for(j=0;j<size;j++)
            key[i*32+j] = mRand('a','z');
        key[i*32+j]=0;
    }
    
    map<string,int> cpp_map;
    MChain *morn_map = mChainCreate();
    
    mLog(INFO,"STL Map写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        cpp_map.insert(pair<string,int>(&(key[i*32]),i));
    mTimerEnd();
    
    mLog(INFO,"Morn Map写入：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
        mMapWrite(morn_map,&(key[i*32]),DFLT,&i,sizeof(int));
    mTimerEnd();
    
    mLog(INFO,"STL Map读出：");
    map<string,int>::iterator iter;
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int index = mRand(0,TEST_NUM);
        iter = cpp_map.find(&(key[index*32]));
    }
    mTimerEnd();
    
    mLog(INFO,"Morn Map读出：");
    mTimerBegin();
    for(i=0;i<TEST_NUM;i++)
    {
        int index = mRand(0,TEST_NUM);
        int *value = (int *)mMapRead(morn_map,&(key[index*32]),DFLT,NULL,DFLT);
    }
    mTimerEnd();
    
    mChainRelease(morn_map);
    free(key);
    return 0;
}
