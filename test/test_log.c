/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// gcc -O2 -fopenmp test_log.c -o test_log.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn
// cl.exe test_log.c ..\lib\x64_msvc\libmorn.lib ..\lib\x64_msvc\pthreadVC2.lib -I ..\include\ -I ..\lib\include\pthread\

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

#define LOG_FORMAT1(message) "[%s,line %d]Info: in function %s:"message,__FILE__,__LINE__,__FUNCTION__
#define LOG_FORMAT2(message) "%s:"message,mTimeString()
void test1()
{
    mLog(MORN_INFO,"this is a Morn log, num=%d\n",1);
    mLog(MORN_INFO,mLogFormat(1,"this is a Morn log, num=%d"),1);
    mLog(MORN_INFO,mLogFormat(2,"this is a Morn log, num=%d"),2);
    mLog(MORN_INFO,mLogFormat(3,"this is a Morn log, num=%d"),3);
    mLog(MORN_INFO,mLogFormat(4,"this is a Morn log, num=%d"),4);
    mLog(MORN_INFO,mLogFormat(5,"this is a Morn log, num=%d"),5);
}

void test2()
{
    mLogSet("./test_log.log");
    int i=1;
    mLog(MORN_INFO, "this is a Morn log\n");
    mLog(MORN_DEBUG,"i=%d\n",i);
    mLog(MORN_INFO, "[%s,line %d]Info: in function %s:this is a Morn log\n",__FILE__,__LINE__,__FUNCTION__);
    mLog(MORN_DEBUG,"[%s,line %d]Info: in function %s:i=%d\n",__FILE__,__LINE__,__FUNCTION__,i);
    mLog(MORN_INFO, LOG_FORMAT1("this is a Morn log\n"));
    mLog(MORN_DEBUG,LOG_FORMAT1("i=%d\n"),i);
    mLog(MORN_INFO, LOG_FORMAT2("this is a Morn log\n"));
    mLog(MORN_DEBUG,LOG_FORMAT2("i=%d\n"),i);
}

void test3()
{
    mLogSet(MORN_INFO,"./test_log.log");
    int i=1;
    mLog(MORN_INFO, "this is a Morn log\n");
    mLog(MORN_DEBUG,"i=%d\n",i);
    mLog(MORN_INFO, "[%s,line %d]Info: in function %s:this is a Morn log\n",__FILE__,__LINE__,__FUNCTION__);
    mLog(MORN_DEBUG,"[%s,line %d]Info: in function %s:i=%d\n",__FILE__,__LINE__,__FUNCTION__,i);
    mLog(MORN_INFO, LOG_FORMAT1("this is a Morn log\n"));
    mLog(MORN_DEBUG,LOG_FORMAT1("i=%d\n"),i);
    mLog(MORN_INFO, LOG_FORMAT2("this is a Morn log\n"));
    mLog(MORN_DEBUG,LOG_FORMAT2("i=%d\n"),i);
}

void log_thread(int *test_num)
{
    int data[100];for(int i=0;i<100;i++) data[i]=mRand(0,100);
    for(int i=0;i<*test_num;i++)
        mLog(MORN_INFO,mLogFormat(5,"data=%d"),data[i%100]);
}

void test4()
{
    mLogSet(DFLT,"./test_log.log");
    
    int test_num = 800;
    
    mTimerBegin();
    log_thread(&test_num);
    mTimerEnd();

    mLogSet(DFLT,"./test_log2.log");
    mTimerBegin();
    mThread((log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num));
    mTimerEnd();
}

int main()
{
    test1();
    test2();
    test3();
    test4();
}
