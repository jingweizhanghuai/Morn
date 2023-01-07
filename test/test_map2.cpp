/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -O2 -DNDEBUG test_map2.cpp -o test_map2.exe -lmorn -labsl_city

#include "morn_util.h"

#include <map>
#include <string>
#include <iostream>

#include "absl/container/btree_map.h"

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

void stl_test1()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    std::map<std::string,int> stl_map;

    printf("10000 times test with 100 node for: key is string and value is integer:\n");
    mTimerBegin("STL map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<100;i++) int data_i = stl_map.find(data[i].data_s)->second;
        for(int i=0;i<100;i++) stl_map.erase(data[i].data_s);
    }
    mTimerEnd("STL map");

    printf("1000 times test with 1000 node for: key is string and value is integer:\n");
    mTimerBegin("STL map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<1000;i++) int data_i = stl_map.find(data[i].data_s)->second;
        for(int i=0;i<1000;i++) stl_map.erase(data[i].data_s);
    }
    mTimerEnd("STL map");

    printf("100 times test with 10000 node for: key is string and value is integer:\n");
    mTimerBegin("STL map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<10000;i++) int data_i = stl_map.find(data[i].data_s)->second;
        for(int i=0;i<10000;i++) stl_map.erase(data[i].data_s);
    }
    mTimerEnd("STL map");

    free(data);
}

void absl_test1()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    absl::btree_map<std::string,int> absl_map;

    printf("10000 times test with 100 node for: key is string and value is integer:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) absl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<100;i++) int data_i = absl_map.find(data[i].data_s)->second;
        for(int i=0;i<100;i++) absl_map.erase(data[i].data_s);
    }
    mTimerEnd("Abseil map");

    printf("1000 times test with 1000 node for: key is string and value is integer:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) absl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<1000;i++) int data_i = absl_map.find(data[i].data_s)->second;
        for(int i=0;i<1000;i++) absl_map.erase(data[i].data_s);
    }
    mTimerEnd("Abseil map");

    printf("100 times test with 10000 node for: key is string and value is integer:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) absl_map[data[i].data_s]=data[i].data_i;
        for(int i=0;i<10000;i++) int data_i = absl_map.find(data[i].data_s)->second;
        for(int i=0;i<10000;i++) absl_map.erase(data[i].data_s);
    }
    mTimerEnd("Abseil map");
    
    free(data);
}

void Morn_test1()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    MMap *morn_map = mMapCreate();

    printf("10000 times test with 100 node for: key is string and value is integer:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
        for(int i=0;i<100;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
        for(int i=0;i<100;i++) mMapNodeDelete(morn_map,data[i].data_s);
    }
    mTimerEnd("Morn map");

    printf("1000 times test with 1000 node for: key is string and value is integer:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
        for(int i=0;i<1000;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
        for(int i=0;i<1000;i++) mMapNodeDelete(morn_map,data[i].data_s);
    }
    mTimerEnd("Morn map");

    printf("100 times test with 10000 node for: key is string and value is integer:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) mMapWrite(morn_map,data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
        for(int i=0;i<10000;i++) int *data_i = (int *)mMapRead(morn_map,data[i].data_s);
        for(int i=0;i<10000;i++) mMapNodeDelete(morn_map,data[i].data_s);
    }
    mTimerEnd("Morn map");
    
    mMapRelease(morn_map);
    free(data);
}

void stl_test2()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    std::map<int,std::string> stl_map;

    printf("10000 times test with 100 node for: key is integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
        for(int i=0;i<100;i++) stl_map.erase(data[i].data_i);
    }
    mTimerEnd("STL map");

    printf("1000 times test with 1000 node for: key is integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
        for(int i=0;i<1000;i++) stl_map.erase(data[i].data_i);
    }
    mTimerEnd("STL map");
    
    printf("100 times test with 10000 node for: key is integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = stl_map.find(data[i].data_i)->second;
        for(int i=0;i<10000;i++) stl_map.erase(data[i].data_i);
    }
    mTimerEnd("STL map");

    free(data);
}

void absl_test2()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    absl::btree_map<int,std::string> absl_map;

    printf("10000 times test with 100 node for: key is integer and value is string:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) absl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = absl_map.find(data[i].data_i)->second;
        for(int i=0;i<100;i++) absl_map.erase(data[i].data_i);
    }
    mTimerEnd("Abseil map");

    printf("1000 times test with 1000 node for: key is integer and value is string:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) absl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = absl_map.find(data[i].data_i)->second;
        for(int i=0;i<1000;i++) absl_map.erase(data[i].data_i);
    }
    mTimerEnd("Abseil map");

    printf("100 times test with 10000 node for: key is integer and value is string:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) absl_map[data[i].data_i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = absl_map.find(data[i].data_i)->second;
        for(int i=0;i<10000;i++) absl_map.erase(data[i].data_i);
    }
    mTimerEnd("Abseil map");
    
    free(data);
}

void Morn_test2()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    MMap *morn_map = mMapCreate();

    printf("10000 times test with 100 node for: key is integer and value is string:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
        for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("1000 times test with 1000 node for: key is integer and value is string:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<1000;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
        for(int i=0;i<1000;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("100 times test with 10000 node for: key is integer and value is string:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) mMapWrite(morn_map,&(data[i].data_i),sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<10000;i++) char *data_s = (char *)mMapRead(morn_map,&(data[i].data_i),sizeof(int),NULL,NULL);
        for(int i=0;i<10000;i++) mMapNodeDelete(morn_map,&(data[i].data_i),sizeof(int));
    }
    mTimerEnd("Morn map");
    
    mMapRelease(morn_map);
    free(data);
}

void stl_test3()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    std::map<int,std::string> stl_map;

    printf("10000 times test with 100 node for: key is orderly integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) stl_map[i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = stl_map.find(i)->second;
        for(int i=0;i<100;i++) stl_map.erase(i);
    }
    mTimerEnd("STL map");

    printf("1000 times test with 1000 node for: key is orderly integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) stl_map[i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = stl_map.find(i)->second;
        for(int i=0;i<1000;i++) stl_map.erase(i);
    }
    mTimerEnd("STL map");

    printf("100 times test with 10000 node for: key is orderly integer and value is string:\n");
    mTimerBegin("STL map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) stl_map[i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = stl_map.find(i)->second;
        for(int i=0;i<10000;i++) stl_map.erase(i);
    }
    mTimerEnd("STL map");

    free(data);
}

void absl_test3()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    absl::btree_map<int,std::string> absl_map;

    printf("10000 times test with 100 node for: key is orderly integer and value is string:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) absl_map[i]=data[i].data_s;
        for(int i=0;i<100;i++) std::string data_s = absl_map.find(i)->second;
        for(int i=0;i<100;i++) absl_map.erase(i);
    }
    mTimerEnd("Abseil map");

    printf("1000 times test with 1000 node for: key is orderly integer and value is string:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) absl_map[i]=data[i].data_s;
        for(int i=0;i<1000;i++) std::string data_s = absl_map.find(i)->second;
        for(int i=0;i<1000;i++) absl_map.erase(i);
    }
    mTimerEnd("Abseil map");

    printf("100 times test with 10000 node for: key is orderly integer and value is string:\n");
    mTimerBegin("Abseil map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) absl_map[i]=data[i].data_s;
        for(int i=0;i<10000;i++) std::string data_s = absl_map.find(i)->second;
        for(int i=0;i<10000;i++) absl_map.erase(i);
    }
    mTimerEnd("Abseil map");

    free(data);
}


void Morn_test3()
{
    struct TestData *data = (struct TestData *)malloc(10000*sizeof(struct TestData));
    data_gerenate(data,10000);
    
    MMap *morn_map = mMapCreate();

    printf("10000 times test with 100 node for: key is orderly integer and value is string:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<10000;n++)
    {
        for(int i=0;i<100;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<100;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,NULL);
        for(int i=0;i<100;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("1000 times test with 1000 node for: key is orderly integer and value is string:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<1000;n++)
    {
        for(int i=0;i<1000;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<1000;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,NULL);
        for(int i=0;i<1000;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
    }
    mTimerEnd("Morn map");

    printf("100 times test with 10000 node for: key is orderly integer and value is string:\n");
    mTimerBegin("Morn map");
    for(int n=0;n<100;n++)
    {
        for(int i=0;i<10000;i++) mMapWrite(morn_map,&i,sizeof(int),data[i].data_s,NULL);
        for(int i=0;i<10000;i++) char *data_s = (char *)mMapRead(morn_map,&i,sizeof(int),NULL,NULL);
        for(int i=0;i<10000;i++) mMapNodeDelete(morn_map,&i,sizeof(int));
    }
    mTimerEnd("Morn map");

    mMapRelease(morn_map);
    free(data);
}

int main(int argc,char *argv[])
{
    if(argc<2) {printf("example: test_map2.exe Morn_test1\n");return 1;}
    
         if(strcmp(argv[1], "stl_test1")==0)  stl_test1();
    else if(strcmp(argv[1],"absl_test1")==0) absl_test1();
    else if(strcmp(argv[1],"Morn_test1")==0) Morn_test1();
    
    else if(strcmp(argv[1], "stl_test2")==0)  stl_test2();
    else if(strcmp(argv[1],"absl_test2")==0) absl_test2();
    else if(strcmp(argv[1],"Morn_test2")==0) Morn_test2();

    else if(strcmp(argv[1], "stl_test3")==0)  stl_test3();
    else if(strcmp(argv[1],"absl_test3")==0) absl_test3();
    else if(strcmp(argv[1],"Morn_test3")==0) Morn_test3();

    return 0;
}



