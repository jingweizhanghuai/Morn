/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build_mingw: g++ -O2 -fopenmp -DNDEBUG test_map2.cpp -lmorn -o test_map2.exe
// build_msvc: cl.exe -O2 -nologo -I ..\include\ test_map2.cpp ..\lib\x64_msvc\libmorn.lib
#include "morn_util.h"

#include <map>
#include <unordered_map>
#include <string>
#include <iostream>

struct TestData
{
    char data_s[32];
    int data_i;
};
void data_gerenate(struct TestData *data,int number)
{
    int i,j;
    for(i=0;i<number;i++)
    {
        mRandString(data[i].data_s,15,31);
        data[i].data_i = mRand();
    }
}

void test1()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    std::map<std::string,int> stl_map;
    std::unordered_map<std::string,int> stl_unorderedmap;
    MMap *morn_map = mMapCreate();

    printf("\n10000 times test with 100 node for key is string and value is integer:\n");
    mTimerBegin("STL map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<100;i++) int data_i = stl_map.find(data[i].data_s)->second;
        for(int i=0;i<100;i++) stl_map.erase(data[i].data_s);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
        for(int i=0;i<100;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
        for(int i=0;i<100;i++) stl_unorderedmap.erase(data[i].data_s);
    }
    mTimerEnd("STL unordered_map");
    
    mTimerBegin("Morn map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
        for(int i=0;i<100;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
        for(int i=0;i<100;i++) mMapNodeDelete(morn_map,data[i].data_s);
    }
    mTimerEnd("Morn map");

    printf("\n1000 times test with 1000 node for key is string and value is integer:\n");
    mTimerBegin("STL map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<1000;i++) int data_i = stl_map.find(data[i].data_s)->second;
        for(int i=0;i<1000;i++) stl_map.erase(data[i].data_s);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
        for(int i=0;i<1000;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
        for(int i=0;i<1000;i++) stl_unorderedmap.erase(data[i].data_s);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
        for(int i=0;i<1000;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
        for(int i=0;i<1000;i++) mMapNodeDelete(morn_map,data[i].data_s);
    }
    mTimerEnd("Morn map");

    printf("\n100 times test with 10000 node for key is string and value is integer:\n");
    mTimerBegin("STL map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<10000;i++) int data_i = stl_map.find(data[i].data_s)->second;
        for(int i=0;i<10000;i++) stl_map.erase(data[i].data_s);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
        for(int i=0;i<10000;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
        for(int i=0;i<10000;i++) stl_unorderedmap.erase(data[i].data_s);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
        for(int i=0;i<10000;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
        for(int i=0;i<10000;i++) mMapNodeDelete(morn_map,data[i].data_s);
    }
    mTimerEnd("Morn map");
    
    mMapRelease(morn_map);
}

void test2()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    std::map<int,std::string> stl_map;
    std::unordered_map<int,std::string> stl_unorderedmap;
    MMap *morn_map = mMapCreate();

    printf("\n10000 times test with 100 node for key is integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
        for(int i=0;i<100;i++) stl_map.erase(data[i].data_i);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
        for(int i=0;i<100;i++) stl_unorderedmap.erase(data[i].data_i);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
        for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("\n1000 times test with 1000 node for key is integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
        for(int i=0;i<1000;i++) stl_map.erase(data[i].data_i);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
        for(int i=0;i<1000;i++) stl_unorderedmap.erase(data[i].data_i);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<1000;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
        for(int i=0;i<1000;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("\n100 times test with 10000 node for key is integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
        for(int i=0;i<10000;i++) stl_map.erase(data[i].data_i);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
        for(int i=0;i<10000;i++) stl_unorderedmap.erase(data[i].data_i);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<10000;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
        for(int i=0;i<10000;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    }
    mTimerEnd("Morn map");
    
    mMapRelease(morn_map);
}

void test3()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    std::map<int,std::string> stl_map;
    std::unordered_map<int,std::string> stl_unorderedmap;
    MMap *morn_map = mMapCreate();

    printf("\n10000 times test with 100 node for key is orderly integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_map[i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = stl_map.find(i)->second;
        for(int i=0;i<100;i++) stl_map.erase(i);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_unorderedmap[i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = stl_unorderedmap.find(i)->second;
        for(int i=0;i<100;i++) stl_unorderedmap.erase(i);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,NULL);
        for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("\n1000 times test with 1000 node for key is orderly integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_map[i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = stl_map.find(i)->second;
        for(int i=0;i<1000;i++) stl_map.erase(i);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_unorderedmap[i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = stl_unorderedmap.find(i)->second;
        for(int i=0;i<1000;i++) stl_unorderedmap.erase(i);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<1000;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,NULL);
        for(int i=0;i<1000;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("\n100 times test with 10000 node for key is orderly integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_map[i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = stl_map.find(i)->second;
        for(int i=0;i<10000;i++) stl_map.erase(i);
    }
    mTimerEnd("STL map");

    mTimerBegin("STL unordered_map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_unorderedmap[i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = stl_unorderedmap.find(i)->second;
        for(int i=0;i<10000;i++) stl_unorderedmap.erase(i);
    }
    mTimerEnd("STL unordered_map");

    mTimerBegin("Morn map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<10000;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,NULL);
        for(int i=0;i<10000;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
    }
    mTimerEnd("Morn map");
    
    mMapRelease(morn_map);
}

void test4(int number)
{
    printf("\n%d node test for key is string and value is integer:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);
    
    std::map<std::string,int> stl_map;
    std::unordered_map<std::string,int> stl_unorderedmap;
    MMap *morn_map = mMapCreate();
    
    mTimerBegin("STL map write");
    for(int i=0;i<number;i++) stl_map[data[i].data_s]=data[i].data_i;
    mTimerEnd("STL map write");

    mTimerBegin("STL unordered_map write");
    for(int i=0;i<number;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
    mTimerEnd("STL unordered_map write");

    mTimerBegin("Morn map write");
    for(int i=0;i<number;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
    mTimerEnd("Morn map write");

    mTimerBegin("STL map read");
    for(int i=0;i<number;i++) int data_i = stl_map.find(data[i].data_s)->second;
    mTimerEnd("STL map read");

    mTimerBegin("STL unordered_map read");
    for(int i=0;i<number;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
    mTimerEnd("STL unordered_map read");

    mTimerBegin("Morn map read");
    for(int i=0;i<number;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
    mTimerEnd("Morn map read");

    mTimerBegin("STL map erase");
    for(int i=0;i<number;i++) stl_map.erase(data[i].data_s);
    mTimerEnd("STL map erase");

    mTimerBegin("STL unordered_map erase");
    for(int i=0;i<number;i++) stl_unorderedmap.erase(data[i].data_s);
    mTimerEnd("STL unordered_map erase");

    mTimerBegin("Morn map delete");
    for(int i=0;i<number;i++) mMapNodeDelete(morn_map,data[i].data_s);
    mTimerEnd("Morn map delete");

    mMapRelease(morn_map);
    free(data);
}

void test5(int number)
{
    printf("\n%d node test for key is integer and value is string:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);
    
    std::map<int,std::string> stl_map;
    std::unordered_map<int,std::string> stl_unorderedmap;
    MMap *morn_map = mMapCreate();
    
    mTimerBegin("STL map write");
    for(int i=0;i<number;i++) stl_map[data[i].data_i]=data[i].data_s;
    mTimerEnd("STL map write");

    mTimerBegin("STL unordered_map write");
    for(int i=0;i<number;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
    mTimerEnd("STL unordered_map write");
    
    mTimerBegin("Morn map write");
    for(int i=0;i<number;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,DFLT);
    mTimerEnd("Morn map write");
    
    mTimerBegin("STL map read");
    for(int i=0;i<number;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
    mTimerEnd("STL map read");

    mTimerBegin("STL unordered_map read");
    for(int i=0;i<number;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
    mTimerEnd("STL unordered_map read");
    
    mTimerBegin("Morn map read");
    for(int i=0;i<number;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
    mTimerEnd("Morn map read");

    mTimerBegin("STL map erase");
    for(int i=0;i<number;i++) stl_map.erase(data[i].data_i);
    mTimerEnd("STL map erase");

    mTimerBegin("STL unordered_map erase");
    for(int i=0;i<number;i++) stl_unorderedmap.erase(data[i].data_i);
    mTimerEnd("STL unordered_map erase");
    
    mTimerBegin("Morn map delete");
    for(int i=0;i<number;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    mTimerEnd("Morn map delete");

    mMapRelease(morn_map);
    free(data);
}

int main()
{
    test1();
    test2();
    test3();
    
    test4(100000);
    test4(1000000);
    
    test5(100000);
    test5(1000000);
    return 0;
}

