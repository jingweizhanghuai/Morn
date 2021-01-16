/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// g++ -O2 -fopenmp test_log2.cpp -o test_log2.exe -I C:\ProgramFiles\CPackage\glog\include -I C:\ProgramFiles\CPackage\spdlog\include -I C:\ProgramFiles\CPackage\log4cpp\include -I ..\include\ -L C:\ProgramFiles\CPackage\glog\lib_x64_mingw -L C:\ProgramFiles\CPackage\log4cpp\lib_x64_mingw -L ..\lib\x64_mingw\ -lmorn -lglog -ldbghelp -llog4cpp
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<iostream>

#include "glog/logging.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"

#include "morn_util.h"

int main(int argc, char** argv)
{
    char datas[100][32];
    int datai[100];
    double datad[100];
    for(int i=0;i<100;i++)
    {
        //生成随机整数
        datai[i]=mRand(DFLT,DFLT);
        //生成随机浮点数
        datad[i]=(double)mRand(-1000000,1000000)/1000000.0;
        //生成随机字符串
        mRandString(&(datas[i][0]),15,31);
    }
    
    mTimerBegin("glog");
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::GLOG_INFO, "./test_log_glog.log");
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        LOG(INFO)<<": Hello glog, "<<"datai="<<datai[i]<<", datad="<<datad[i]<<", datas="<<&(datas[i][0]);
    }
    google::ShutdownGoogleLogging();
    mTimerEnd("glog");

    mTimerBegin("spdlog");
    auto console2 = spdlog::basic_logger_mt(argv[0],"./test_log_spdlog.log");
    spdlog::set_pattern("[%Y.%m.%d %H:%M:%S thread%t]%l: %v");
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        console2->info("[{} line {},function {}] Hello spdlog, datai={}, datad={}, datas={}",__FILE__,__LINE__,__FUNCTION__,datai[i],datad[i],&(datas[i][0]));
    }
    mTimerEnd("spdlog");

    mTimerBegin("log4cpp");
    log4cpp::FileAppender * appender = new log4cpp::FileAppender("appender","./test_log_log4cpp.log");
    log4cpp::PatternLayout* pLayout = new log4cpp::PatternLayout();
    pLayout->setConversionPattern("[%d thread%t %c]%p: %m%n");
    appender->setLayout(pLayout);
    log4cpp::Category& root =log4cpp::Category::getRoot();
    log4cpp::Category& infoCategory =root.getInstance(argv[0]);
    infoCategory.addAppender(appender);
    infoCategory.setPriority(log4cpp::Priority::INFO);
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        infoCategory.info("[%s,line %d,function %s] Hello log4cpp, datai=%d, datad=%f, datas=%s",__FILE__,__LINE__,__FUNCTION__,datai[i],datad[i],&(datas[i][0]));
    }
    log4cpp::Category::shutdown();
    mTimerEnd("log4cpp");

    mTimerBegin("Morn");
    mLogSet(DFLT,DFLT,"./test_log_morn.log");
    for(int n=0;n<1000000;n++)
    {
        int i=n%100;
        mLog(MORN_INFO,mLogFormat5("Hello Morn, datai=%d, datad=%f, datas=%s"),datai[i],datad[i],&(datas[i][0]));
    }
    mTimerEnd("Morn");

    return 0;
}
