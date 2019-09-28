/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//±‡“Î£∫gcc -O2 -fopenmp test_memory.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_memory.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_Util.h"

void morn_mem_test(int test_num)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));
    
    for(int i=0;i<test_num;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(1,1023);
            data[idx] = mMalloc(size);
        }
        else
        {
            mFree(data[idx]);
            data[idx] = NULL;
        }
    }
    
    for(int i=0;i<1024;i++)
    {
        if(data[i]!=NULL)
            mFree(data[i]);
    }
}

void c_mem_test(int test_num)
{
    int *data[1024];
    memset(data,0,1024*sizeof(int *));
    
    for(int i=0;i<test_num;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(1,1023);
            data[idx] = malloc(size);
        }
        else
        {
            free(data[idx]);
            data[idx] = NULL;
        }
    }
    
    for(int i=0;i<1024;i++)
    {
        if(data[i]!=NULL)
            free(data[i]);
    }
}

int main()
{
    mTimerBegin();
    for(int i=0;i<100;i++)
        c_mem_test(1000);
    mTimerEnd();
    
    mTimerBegin();
    for(int i=0;i<100;i++)
        morn_mem_test(1000);
    mTimerEnd();
}
