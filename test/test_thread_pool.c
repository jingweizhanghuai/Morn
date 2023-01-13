/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// mingw_build: gcc -O2 -fopenmp test_thread_pool.c -o test_thread_pool.exe -lmorn
// msvc_build: cl.exe -O2 -openmp -I ../include .\test_thread_pool.c ..\lib\x64_msvc\libmorn.lib
#include "morn_ptc.h"

void func(char *str)
{
    printf("Thread %d input : %s\n",mThreadID(),str);
    mSleep(mRand(0,100));
    for(int i=0;str[i];i++)
    {
             if((str[i]>='a')&&(str[i]<='z')) str[i]+=('A'-'a');
        else if((str[i]>='A')&&(str[i]<='Z')) str[i]+=('a'-'A');
    }
    printf("Thread %d output: %s\n",mThreadID(),str);
}

void test_no_adjust()
{
    MList *list = mListCreate();
    mListPlace(list,NULL,1024,64);

    int thread_num =5;
    mPropertyWrite("ThreadPool","thread_num",&thread_num,sizeof(int));

    mPropertyRead("ThreadPool","thread_num",&thread_num);
    printf("thread_num=%d\n",thread_num);
    
    for(int i=0;i<1024;i++)
    {
        mSleep(mRand(0,30));
        mRandString(list->data[i],32,64);
        mThreadPool(func,list->data[i]);
    }

    mPropertyRead("ThreadPool","thread_num",&thread_num);
    mPropertyWrite("ThreadPool","exit");
    printf("thread_num=%d\n",thread_num);
    
    mListRelease(list);
    printf("finish\n");
}

void test_adjust()
{
    MList *list = mListCreate();
    mListPlace(list,NULL,1024,64);

    int thread_num =2;
    mPropertyWrite("ThreadPool","thread_num",&thread_num,sizeof(int));
    int thread_adjust=1;
    mPropertyWrite("ThreadPool","thread_adjust",&thread_adjust,sizeof(int));
    int thread_max = 8;
    mPropertyWrite("ThreadPool","thread_max",&thread_max,sizeof(int));

    mPropertyRead("ThreadPool","thread_num",&thread_num);
    printf("thread_num=%d\n",thread_num);
    
    for(int i=0;i<1024;i++)
    {
        mSleep(mRand(0,30));
        mRandString(list->data[i],32,64);
        mThreadPool(func,list->data[i]);
    }

    mPropertyRead("ThreadPool","thread_num",&thread_num);
    
    mPropertyWrite("ThreadPool","exit");
    printf("thread_num=%d\n",thread_num);
    
    mListRelease(list);
    printf("finish\n");
}

int main(int argc,char *argv[])
{
    if(argc<2)                                  test_no_adjust();
    else if(strcmp(argv[1],"enable_adjust")==0) test_adjust();
    else                                        test_no_adjust();
    return 0;
}
