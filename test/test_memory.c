/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_ptc.h"

#ifdef USE_LIBC
//build: gcc -O2 -fopenmp test_memory.c -o test_memory_libc.exe -DUSE_LIBC -lmorn
#define my_malloc malloc
#define my_free free
#endif

#ifdef USE_MORN
//build: gcc -O2 -fopenmp test_memory.c -o test_memory_morn.exe -DUSE_MORN -lmorn
#define my_malloc mMalloc
#define my_free mFree
#endif

#ifdef USE_TCMALLOC
//build: gcc -O2 -fopenmp test_memory.c -o test_memory_tcmalloc.exe -DUSE_TCMALLOC -lmorn -ltcmalloc
#include "gperftools/tcmalloc.h"
#define my_malloc tc_malloc
#define my_free tc_free
#endif

#ifdef USE_JEMALLOC
//build: gcc -O2 -fopenmp test_memory.c -o test_memory_jemalloc.exe -DUSE_JEMALLOC -lmorn -ljemalloc
#include "jemalloc/jemalloc.h"
#define my_malloc malloc
#define my_free free
#endif

#ifdef USE_MIMALLOC
//build: gcc -O2 -fopenmp test_memory.c -o test_memory_mimalloc.exe -DUSE_MIMALLOC -lmorn -lmimalloc
#include "mimalloc/mimalloc.h"
#define my_malloc mi_malloc
#define my_free mi_free
#endif

#define TEST_NUM 10000000

void *data[1024]={NULL};
int flag[1024]={0};
void test()
{
    int count = 0;
    while(count<TEST_NUM)
    {
        int idx = mRand(0,1024);
        if(mAtomicCompare(flag+idx,0,DFLT))
        {
            data[idx] = my_malloc(mRand(0,16384));
            mAtomicSet(flag+idx,1);
            count++;
        }
        else if(mAtomicCompare(flag+idx,1,DFLT))
        {
            my_free(data[idx]);
            mAtomicSet(flag+idx,0);
        }
    }
}

void test_end()
{
    for(int i=0;i<1024;i++)
        if(flag[i]==1) my_free(data[i]);
}

void test_single_thread()
{
    mTimerBegin();
    test(data);
    test_end();
    mTimerEnd();
}

void test_multi_thread()
{
    mTimerBegin();
    mThread((test,NULL),(test,NULL),(test,NULL),(test,NULL),(test,NULL),(test,NULL),(test,NULL),(test,NULL));
    mTimerEnd();
    
    test_end();
}

int main(int argc,char *argv[])
{
    if(argc>1)
    {
        if(argv[1][0]=='m') test_multi_thread();
        else                test_single_thread();
    }
    else test_single_thread();
    return 0;
}

