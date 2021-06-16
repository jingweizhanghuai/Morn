// g++ -O2 test_array2.cpp -o test_array2.exe -lmorn
#include "morn_util.h"
#include <vector>

#define TEST_NUM 10000000

int main()
{
    std::vector<int> vec;
    mTimerBegin("STL vector");
    for(int i=0;i<TEST_NUM;i++) 
    {
        vec.push_back(i);
        // if((i%100000)==0) printf("vec[0]=%p\n",&(vec[0]));
    }
    mTimerEnd("STL vector");
    
    MArray *array=mArrayCreate(sizeof(int));
    mTimerBegin("Morn");
    for(int i=0;i<TEST_NUM;i++)
    {
        _ArrayPushBack(32,array,&i);
        // if((i%100000)==0) printf("array->dataS32=%p\n",array->dataS32);
    }
    mTimerEnd("Morn");
    mArrayRelease(array);
    
    return 0;
}