/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译：gcc -O2 -fopenmp test_thread_pool.c -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -o test_thread_pool.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"
// void mThreadPool(MList *pool,void (*func)(void *),void *para,int *flag,float priority)

struct FuncPara
{
    int idx;
    int t;
};

void func(void *para)
{
    struct FuncPara *p = (struct FuncPara *)para;
    printf("func%03d run with thread %d,for %dms\n",p->idx,mThreadID(),p->t);
    mSleep(p->t);
}

int main()
{
    MList *pool = mListCreate(3);

    struct FuncPara para;
    for(int i=0;i<100;i++)
    {
        para.idx=i;para.t=mRand(0,100);
        mThreadPool(pool,func,&para,sizeof(struct FuncPara));
        mSleep(mRand(0,40));
    }

    mListRelease(pool);
}
