// g++ -O2 test_array2.cpp -o test_array2.exe -lmorn
#include "morn_util.h"
#include <vector>

#define TEST_NUM 10000000
int main()
{
    std::vector<int> vec;
    mTimerBegin("STL vector");
    for(int i=0;i<TEST_NUM;i++) 
        vec.push_back(i);
    mTimerEnd("STL vector");

    MArray *array=mArrayCreate(sizeof(int));
    mTimerBegin("Morn");
    for(int i=0;i<TEST_NUM;i++)
        mArrayWrite(array,&i);
    mTimerEnd("Morn");
    mArrayRelease(array);
    
    return 0;
}