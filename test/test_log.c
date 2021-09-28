/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// gcc -O2 -fopenmp test_log.c -o test_log.exe -lmorn -lws2_32
// cl.exe test_log.c ..\lib\x64_msvc\libmorn.lib ..\lib\x64_msvc\pthreadVC2.lib -I ..\include\ -I ..\lib\include\pthread\

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

char *author = "JingWeiZhangHuai";
#define LOG_FORMAT1(message) "[%s,line %d]Info: in function %s:"message,__FILE__,__LINE__,__FUNCTION__
#define LOG_FORMAT2(message) "[%s author %s]: "message,mTimeString(),author
void test1()
{
    mLog(MORN_INFO,"this is a Morn log, num=%d\n",1);
}

void test2()
{
    mLog(MORN_INFO,"this is a Morn log, num=%d\n",1);
    mLog(MORN_INFO,mLogFormat1("this is a Morn log, format %d"),1);
    mLog(MORN_INFO,mLogFormat2("this is a Morn log, format %d"),2);
    mLog(MORN_INFO,mLogFormat3("this is a Morn log, format %d"),3);
    mLog(MORN_INFO,mLogFormat4("this is a Morn log, format %d"),4);
    mLog(MORN_INFO,mLogFormat5("this is a Morn log, format %d"),5);
    mLog(MORN_INFO,LOG_FORMAT1("this is a Morn log, format %d"),6);
    mLog(MORN_INFO,LOG_FORMAT2("this is a Morn log, format %d"),7);
}

void test3()
{
    mLog(MORN_INFO, "this is log No.1\n");
    
    mPropertyWrite("Log","log_file","./test_log.log");
    
    mLog(MORN_INFO, "this is log No.2\n");
    
    mPropertyWrite("Log","log_file","exit");
    
    mLog(MORN_INFO, "this is log No.3\n");

    mPropertyWrite("Log","log_file","./test_log2.log");
    
    mLog(MORN_INFO, "this is log No.4\n");

    int log_console = 1;
    mPropertyWrite("Log","log_console",&log_console,sizeof(int));
    
    mLog(MORN_INFO, "this is log No.5\n");

    log_console = 0;
    mPropertyWrite("Log","log_console",&log_console,sizeof(int));
    
    mLog(MORN_INFO, "this is log No.6\n");

    mPropertyWrite("Log","log_file","exit");

    mLog(MORN_INFO, "this is log No.7\n");
}

void test4()
{
    int log_level = MORN_INFO;
    mPropertyWrite("Log","log_level",&log_level,sizeof(int));
    
    mLog(MORN_DEBUG  ,"this is a debug log\n");
    mLog(MORN_INFO   ,"this is a info log\n");
    mLog(MORN_WARNING,"this is a warning log\n");
    mLog(MORN_ERROR  ,"this is a error log\n\n");

    #define REMARK MORN_INFO+1
    log_level = REMARK;
    mPropertyWrite("Log","log_level",&log_level,sizeof(int));

    mLog(MORN_DEBUG  , "this is a debug log\n");
    mLog(MORN_INFO   , "this is a info log\n");
    mLog(REMARK      , "this is a remark log\n");
    mLog(MORN_WARNING, "this is a warning log\n");
    mLog(MORN_ERROR  , "this is a error log\n\n");
}

void send_log(char *log,int size,char *addr)
{
    mUDPWrite(addr,log,size);
}

void test5()
{
    void *func=send_log;
    mPropertyWrite("Log","log_function",&func,sizeof(void *));
    
    char *para = "localhost:1234";
    mPropertyWrite("Log","log_func_para",&para,sizeof(char *));
    
    mLog(MORN_INFO, "this is a Morn log\n");
}



// void log_thread(int *test_num)
// {
//     int data[100];for(int i=0;i<100;i++) data[i]=mRand(0,100);
//     for(int i=0;i<*test_num;i++)
//         mLog(MORN_INFO,mLogFormat(5,"data=%d"),data[i%100]);
// }

// void test6()
// {
//     mLogSet("./test_log.log");
    
//     int test_num = 800;
    
//     mTimerBegin();
//     log_thread(&test_num);
//     mTimerEnd();

//     mLogSet("./test_log2.log");
//     mTimerBegin();
//     mThread((log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num),(log_thread,&test_num));
//     mTimerEnd();
// }

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}
