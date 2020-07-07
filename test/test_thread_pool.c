/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译：gcc -O2 -fopenmp test_thread_pool.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_thread_pool.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

void func(void *para)
{
    int ID;PthreadOrder(ID);
    int *t = para;
    printf("func run on thread %d,sleep %dms\n",ID,*t);
    mSleep(*t);
}

int main()
{
    //创建一个有四个线程的线程池
    MList *pool = mListCreate(4,NULL);
    
    int t[100];
    for(int i=0;i<100;i++)
    {
        t[i]=mRand(10,120);
        // 把函数扔进线程池
        mThreadPool(pool,func,t+i,NULL,0);
        mSleep(mRand(5,30));
    }

    // 释放线程池
    mListRelease(pool);
}
