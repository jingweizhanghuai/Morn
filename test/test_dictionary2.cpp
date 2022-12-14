/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -O2 -DNDEBUG test_dictionary2.cpp -o test_dictionary2.exe -lmorn -labsl_hash -labsl_city -labsl_low_level_hash -labsl_raw_hash_set

#include "morn_util.h"

#include <unordered_map>
#include <string>
#include <iostream>

#include "absl/container/flat_hash_map.h"

extern "C" void *DictionaryInit();

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

void stl_test1(int number)
{
    printf("\n%d node test for key is string and value is integer:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);
    
    std::unordered_map<std::string,int> stl_unorderedmap;

    printf("write for %d times\n",number);
    mTimerBegin("STL unordered_map write");
    for(int i=0;i<number;i++) stl_unorderedmap[data[i].data_s]=data[i].data_i;
    mTimerEnd("STL unordered_map write");

    printf("read for %d times\n",number);
    mTimerBegin("STL unordered_map read");
    for(int i=0;i<number;i++) int data_i = stl_unorderedmap.find(data[i].data_s)->second;
    mTimerEnd("STL unordered_map read");

    printf("delete for %d times\n",number);
    mTimerBegin("STL unordered_map erase");
    for(int i=0;i<number;i++) stl_unorderedmap.erase(data[i].data_s);
    mTimerEnd("STL unordered_map erase");

    free(data);
}

void absl_test1(int number)
{
    printf("\n%d node test for key is string and value is integer:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);
    
    absl::flat_hash_map<std::string,int> absl_hash_map;

    printf("write for %d times\n",number);
    mTimerBegin("Abseil hash_map write");
    for(int i=0;i<number;i++) absl_hash_map[data[i].data_s]=data[i].data_i;
    mTimerEnd("Abseil hash_map write");

    printf("read for %d times\n",number);
    mTimerBegin("Abseil hash_map read");
    for(int i=0;i<number;i++) int data_i = absl_hash_map.find(data[i].data_s)->second;
    mTimerEnd("Abseil hash_map read");

    printf("delete for %d times\n",number);
    mTimerBegin("Abseil hash_map erase");
    for(int i=0;i<number;i++) absl_hash_map.erase(data[i].data_s);
    mTimerEnd("Abseil hash_map erase");

    free(data);
}

void Morn_test1(int number)
{
    printf("\n%d node test for key is string and value is integer:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);
    
    DictionaryInit();
    
    printf("write for %d times\n",number);
    mTimerBegin("Morn dictionary write");
    for(int i=0;i<number;i++) mDictionaryWrite(data[i].data_s,DFLT,&(data[i].data_i),sizeof(int));
    mTimerEnd("Morn dictionary write");

    printf("read for %d times\n",number);
    mTimerBegin("Morn dictionary read");
    for(int i=0;i<number;i++) int *data_i = (int *)mDictionaryRead(data[i].data_s);
    mTimerEnd("Morn dictionary read");

    printf("delete for %d times\n",number);
    mTimerBegin("Morn dictionary delete");
    for(int i=0;i<number;i++) mDictionaryNodeDelete(data[i].data_s);
    mTimerEnd("Morn dictionary delete");

    free(data);
}

void stl_test2(int number)
{
    printf("\n%d node test for key is integer and value is string:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);

    std::unordered_map<int,std::string> stl_unorderedmap;

    printf("write for %d times\n",number);
    mTimerBegin("STL unordered_map write");
    for(int i=0;i<number;i++) stl_unorderedmap[data[i].data_i]=data[i].data_s;
    mTimerEnd("STL unordered_map write");

    printf("read for %d times\n",number);
    mTimerBegin("STL unordered_map read");
    for(int i=0;i<number;i++) std::string data_s = stl_unorderedmap.find(data[i].data_i)->second;
    mTimerEnd("STL unordered_map read");

    printf("delete for %d times\n",number);
    mTimerBegin("STL unordered_map erase");
    for(int i=0;i<number;i++) stl_unorderedmap.erase(data[i].data_i);
    mTimerEnd("STL unordered_map erase");

    free(data);
}

void absl_test2(int number)
{
    printf("\n%d node test for key is integer and value is string:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);

    absl::flat_hash_map<int,std::string> absl_hash_map;

    printf("write for %d times\n",number);
    mTimerBegin("Abseil hash_map write");
    for(int i=0;i<number;i++) absl_hash_map[data[i].data_i]=data[i].data_s;
    mTimerEnd("Abseil hash_map write");

    printf("read for %d times\n",number);
    mTimerBegin("Abseil hash_map read");
    for(int i=0;i<number;i++) std::string data_s = absl_hash_map.find(data[i].data_i)->second;
    mTimerEnd("Abseil hash_map read");

    printf("delete for %d times\n",number);
    mTimerBegin("Abseil hash_map erase");
    for(int i=0;i<number;i++) absl_hash_map.erase(data[i].data_i);
    mTimerEnd("Abseil hash_map erase");

    free(data);
}

void Morn_test2(int number)
{
    printf("\n%d node test for key is integer and value is string:\n",number);
    struct TestData *data = (struct TestData *)malloc(number*sizeof(struct TestData));
    data_gerenate(data,number);
    
    DictionaryInit();

    printf("write for %d times\n",number);
    mTimerBegin("Morn dictionary write");
    for(int i=0;i<number;i++) mDictionaryWrite(&(data[i].data_i),sizeof(int),data[i].data_s,DFLT);
    mTimerEnd("Morn dictionary write");

    printf("read for %d times\n",number);
    mTimerBegin("Morn dictionary read");
    for(int i=0;i<number;i++) char *data_s = (char *)mDictionaryRead(&(data[i].data_i),sizeof(int),NULL,NULL);
    mTimerEnd("Morn dictionary read");

    printf("delete for %d times\n",number);
    mTimerBegin("Morn dictionary delete");
    for(int i=0;i<number;i++) mDictionaryNodeDelete(&(data[i].data_i),sizeof(int));
    mTimerEnd("Morn dictionary delete");

    free(data);
}

int main(int argc,char *argv[])
{
    if(argc<2) {printf("example: test_dictionary2.exe test1\n");return 1;}
    
    if(strcmp(argv[1],"stl_test1")==0)
    {
        stl_test1(10000);
        stl_test1(100000);
        stl_test1(1000000);
        stl_test1(10000000);
        return 0;
    }
    
    if(strcmp(argv[1],"absl_test1")==0)
    {
        absl_test1(10000);
        absl_test1(100000);
        absl_test1(1000000);
        absl_test1(10000000);
        return 0;
    }
    
    if(strcmp(argv[1],"Morn_test1")==0)
    {
        Morn_test1(10000);
        Morn_test1(100000);
        Morn_test1(1000000);
        Morn_test1(10000000);
        return 0;
    }

    if(strcmp(argv[1],"stl_test2")==0)
    {
        stl_test2(10000);
        stl_test2(100000);
        stl_test2(1000000);
        stl_test2(10000000);
        return 0;
    }
    
    if(strcmp(argv[1],"absl_test2")==0)
    {
        absl_test2(10000);
        absl_test2(100000);
        absl_test2(1000000);
        absl_test2(10000000);
        return 0;
    }
    
    if(strcmp(argv[1],"Morn_test2")==0)
    {
        Morn_test2(10000);
        Morn_test2(100000);
        Morn_test2(1000000);
        Morn_test2(10000000);
        return 0;
    }

    return 0;
}
