/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

#define TEST_NUM 10000000

#ifdef USE_LIBC
//build_x64_gnu: gcc -O2 -fopenmp test_memory.c -o test_memory_libc.exe -DUSE_LIBC -I ../include/ -L ../lib/x64_gnu -lmorn -lm
void test(void *para)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));

    mTimerBegin("libc");
    for(int i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(0,16384);
            data[idx] = malloc(size);
        }
        else
        {
            free(data[idx]);
            data[idx] = NULL;
        }
    }

    
    for(int i=0;i<1024;i++)
        if(data[i]!=NULL)
            free(data[i]);
    
    mTimerEnd("libc");
}
#endif

#ifdef USE_MORN
//build_x64_mingw: gcc -O2 -fopenmp test_memory.c -o test_memory_morn.exe -DUSE_MORN -lmorn
//build_x64_gnu  : gcc -O2 -fopenmp test_memory.c -o test_memory_morn.exe -DUSE_MORN -I ../include/ -L ../lib/x64_gnu -lmorn -lm
void test(void *para)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));

    mTimerBegin("Morn");
    
    for(int i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,1024);
        // if(i==63842) printf("data[%d]=%d\n",idx,data[idx]);
        if(data[idx]==NULL)
        {
            int size =mRand(0,16384);
            // if(i==63842) {size=99;printf("aaaaa1\n");}
            data[idx] = mMalloc(size);
            // if(i==63842) printf("aaaaa2\n");
        }
        else
        {
            // printf("bbbbbbb1\n");
            mFree(data[idx]);
            // printf("bbbbbbb2\n");
            data[idx] = NULL;
        }
    }

    for(int i=0;i<1024;i++)
        if(data[i]!=NULL)
            mFree(data[i]);
            
    mTimerEnd("Morn");
}
#endif

#ifdef USE_TCMALLOC
//build_x64_gnu: gcc -O2 -fopenmp test_memory.c -o test_memory_tcmalloc.exe -DUSE_TCMALLOC -I ../include/ -L ../lib/x64_gnu -lmorn -ltcmalloc -lm
#include "gperftools/tcmalloc.h"
void test(void *para)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));

    mTimerBegin("tcmalloc");
    for(int i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(0,16384);
            data[idx] = tc_malloc(size);
        }
        else
        {
            tc_free(data[idx]);
            data[idx] = NULL;
        }
    }

    for(int i=0;i<1024;i++)
        if(data[i]!=NULL)
            tc_free(data[i]);
    mTimerEnd("tcmalloc");
}
#endif

#ifdef USE_JEMALLOC
//build_x64_gnu: gcc -O2 -fopenmp test_memory.c -o test_memory_jemalloc.exe -DUSE_JEMALLOC -I ../include/ -L ../lib/x64_gnu -lmorn -ljemalloc -lm
#include "jemalloc/jemalloc.h"
void test(void *para)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));

    mTimerBegin("jemalloc");
    for(int i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(0,16384);
            data[idx] = malloc(size);
        }
        else
        {
            free(data[idx]);
            data[idx] = NULL;
        }
    }
    for(int i=0;i<1024;i++)
        if(data[i]!=NULL)
            free(data[i]);
    mTimerEnd("jemalloc");
}
#endif

#ifdef USE_MIMALLOC
//build_x64_mingw: gcc -O2 -fopenmp test_memory.c -o test_memory_mimalloc.exe -DUSE_MIMALLOC -lmorn -lmimalloc -lPsapi -lBcrypt
//build_x64_gnu:   gcc -O2 -fopenmp test_memory.c -o test_memory_mimalloc.exe -DUSE_MIMALLOC -I ../include/ -L ../lib/x64_gnu -lmorn -lmimalloc -lm
#include "mimalloc.h"
void test(void *para)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));

    mTimerBegin("mimalloc");
    for(int i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(0,16384);
            data[idx] = mi_malloc(size);
        }
        else
        {
            mi_free(data[idx]);
            data[idx] = NULL;
        }
    }
    for(int i=0;i<1024;i++)
        if(data[i]!=NULL)
            mi_free(data[i]);
    mTimerEnd("mimalloc");
}
#endif

int main()
{
    test(NULL);
}

// int mainw()
// {
//     int *a[50];
//     for(int i=0;i<50;i++)
//     {
//         a[i]=mMalloc(100);
//         printf("a[%d]=%p\n",i,a[i]);
//     }
//     for(int i=0;i<50;i++) mFree(a[i]);
// }

// int main()
// {
//     printf("aaaaaa\n");
//     free(NULL);
//     printf("aaaaaa\n");
// }

