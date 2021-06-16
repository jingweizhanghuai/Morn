/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build_mingw: gcc -O2 -fopenmp test_message.c -o test_message.exe -lmorn -lm
// build_msvc: cl.exe -O2 -openmp -I ..\include\ ..\lib\x64_msvc\libmorn.lib .\test_message.c
#include "morn_ptc.h"

void writer()
{
    char data[64];
    for(int i=0;i<100;i++)
    {
        mRandString(data,32,64);
        mProcTopicWrite("mytest",data);
        printf("%2d: write %s\n",i,data);
        mSleep(100);
    }
}

void reader()
{
    // int wait_time=1000;mPropertyWrite("mytest","wait_time",&wait_time,sizeof(int));
    for(int i=0;i<100;i++)
    {
        char *p = mProcTopicRead("mytest");
        if(p!=NULL) printf("%2d: read %s\n",i,p);
        // else printf("%2d: read error\n",i);
    }
}

int main(int argc,char *argv[])
{
    if(strcmp(argv[1],"read" )==0) {reader();return 0;}
    if(strcmp(argv[1],"write")==0) {writer();return 0;}
    return 0;
}
