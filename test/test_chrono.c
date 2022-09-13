/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

// build: gcc test_chrono.c -o test_chrono.exe -lmorn

#include "morn_util.h"

void test0()
{
    int cycle=50;
    mPropertyWrite("Chrono","defalt_cycle",&cycle);
    
    double t0=mTime();
    for(int i=0;i<10;i++)
    {
        mChrono();
        printf("time=%fms\n",(mTime()-t0));
    }
}

void test1()
{
    int ID=mChronoTask(50);
    double t0=mTime();
    for(int i=0;i<10;i++)
    {
        ID=mChrono();
        printf("ID=%d,time=%fms\n",ID,(mTime()-t0));
    }
    mChronoDelete(ID);
}

void test2()
{
    int ID0=mChronoTask(200,5);
    int ID1=mChronoTask(300,3);
    int ID2=mChronoTask(500,2);
    double t0=mTime();
    while(1)
    {
        int ID=mChrono();
        if(ID<0) break;
        if(ID==ID0) printf("task %d,time=%fms\n",ID,mTime()-t0);
        if(ID==ID1) printf("task %d,time=%fms\n",ID,mTime()-t0);
        if(ID==ID2) printf("task %d,time=%fms\n",ID,mTime()-t0);
    }
}

void test3()
{
    int ID0 = mChronoTask(1000,10);
    int t=100;
    int ID1 = mChronoTask(t);
    double t0=mTime();
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
        else printf("time=%fms\n",mTime()-t0);
    }
    mChronoDelete(ID0);
}

int main()
{
    test0();printf("\n\n");
    test1();printf("\n\n");
    test2();printf("\n\n");
    test3();printf("\n\n");
}