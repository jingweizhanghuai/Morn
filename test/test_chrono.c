/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

// build: gcc test_chrono.c -o test_chrono.exe -lmorn

#include "morn_util.h"

void test0()
{
    while(1)
    {
        mChrono();
        printf("time=%f\n",mTime()/1000.0);
    }
}

void test1()
{
    mChronoTask(500);
    while(1)
    {
        mChrono();
        printf("time=%fms\n",mTime());
    }
}

void test2()
{
    int ID0=mChronoTask(200,4);int i0=0;
    int ID1=mChronoTask(300,3);int i1=0;
    int ID2=mChronoTask(500,2);int i2=0;
    printf("begin: time=%fms\n",mTime());
    while(1)
    {
        int ID=mChrono();
        if(ID<0) break;
        if(ID==ID0) printf("loop %d in task %d,time=%fms\n",ID,i0++,mTime());
        if(ID==ID1) printf("loop %d in task %d,time=%fms\n",ID,i1++,mTime());
        if(ID==ID2) printf("loop %d in task %d,time=%fms\n",ID,i2++,mTime());
        // mSleep(mRand(0,50));
    }
}

void test3()
{
    int ID0 = mChronoTask(1000,10);
    int t=100;
    int ID1 = mChronoTask(t);
    printf("begin: time=%fms\n",mTime());
    while(1)
    {
        int ID = mChrono();
        if(ID==ID0)
        {
            mChronoDelete(ID1);
            t=t+100;if(t>500) break;
            ID1 = mChronoTask(t);
            printf("change cycle time: %dms\n",t);
        }
        else printf("time=%fms\n",mTime());
    }
}

int main()
{
    test3();
}