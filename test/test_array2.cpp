// g++ -O2 test_array2.cpp -o test_array2.exe -lmorn
#include "morn_util.h"
#include <vector>

#define TEST_NUM 10000000
extern "C" void *_ArrayPushBack32(MArray *array,uint32_t data);
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
        _ArrayPushBack32(array,(uint32_t )i);
        // if((i%100000)==0) printf("array->dataS32=%p\n",array->dataS32);
    }
    mTimerEnd("Morn");
    mArrayRelease(array);

    int num=0;int capacity=256;
    int *data=(int*)malloc(capacity*sizeof(int));
    mTimerBegin("Morn");
    for(int i=0;i<TEST_NUM;i++)
    {
        if(i==capacity)
        {
            data=(int*)realloc(data,capacity*2*sizeof(int));
            // printf("pp=%p,buff=%p\n",pp,data);
            // printf("buff[10]=%d\n",data[10]);
            
            // int *buff=(int*)malloc(
            // memcpy(buff,data,capacity*sizeof(int));
            capacity+=capacity;
            // free(data);
            // data=buff;
        }
        data[num++]=i;
    }
    mTimerEnd("Morn");
    
    return 0;
}