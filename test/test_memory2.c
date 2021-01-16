/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//build: gcc -O2 -fopenmp test_memory2.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_memory2.exe

#define DEBUG
#include "morn_util.h"

void test1()
{
    int *data1 = mMalloc(10*sizeof(int));
    int *data2 = mMalloc( 8*sizeof(int));
    mFree(data1);
    int *data3 = mMalloc(16*sizeof(int));
    mFree(data3);
    MemoryListPrint(1);
}

int test2()
{
    int *data1 = mMalloc(10*sizeof(int));
    int *data2 = mMalloc( 8*sizeof(int));
    mFree(data1);
    int *data3 = data2;
    mFree(data2);
    mFree(data1);
}

void test3()
{
    int *data = mMalloc(10*sizeof(int));
    for(int i=0;i<12;i++)
        data[i]=i;
    mFree(data);
}

int main()
{
    test1();
    test2();
    test3();
    return 0;
}

