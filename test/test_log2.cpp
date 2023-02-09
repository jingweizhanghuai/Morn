/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_SPDLOG -o test_log2_spdlog.exe -lmorn
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_LOG4CPLUS -o test_log2_log4cplus.exe -lmorn -llog4cplusS
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_G3LOG -o test_log2_g3log.exe -lg3log -lmorn -lpthread
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_EASYLOGGINGPP -o test_log2_easylogingpp.exe -lmorn
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_MORN -o test_log2_morn.exe -lmorn

#include "morn_ptc.h"

struct LogData
{
    int *datai;
    double *datad;
    char *datas;
    int N;
};

#ifdef USE_SPDLOG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
std::shared_ptr<spdlog::logger> spd_logger;
void log_init()
{
    spd_logger = spdlog::rotating_logger_mt("test_log", "test_log_spdlog.log",1024*1024,2);
    spdlog::set_pattern("[%Y.%m.%d %H:%M:%S thread%t %s,line%# function %!]%l: %v");
}
void log_test(struct LogData *p)
{
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        SPDLOG_LOGGER_INFO(spd_logger,"Hello world, datai={}, datad={}, datas={}",p->datai[i],p->datad[i],p->datas+i*32);
    }
}
#endif

#ifdef USE_LOG4CPLUS
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
log4cplus::Logger log4cplus_logger;
void log_init()
{
    log4cplus::SharedAppenderPtr appender(new log4cplus::RollingFileAppender("test_log_log4cplus.log",1024*1024,2));
    std::auto_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout("[%D thread%t %F line%L function %c]%p: %m%n"));
    appender->setLayout(layout);
    log4cplus_logger = log4cplus::Logger::getInstance("log_test");
    log4cplus_logger.addAppender(appender);
}
void log_test(struct LogData *p)
{
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        LOG4CPLUS_INFO(log4cplus_logger, "Hello world, datai"<<p->datai[i]<<", datad="<<p->datad[i]<<", datas=" <<p->datas+i*32);
    }
}
#endif

#ifdef USE_G3LOG
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
std::unique_ptr<g3::LogWorker> g3log_logger;
std::string my_format(const g3::LogMessage& msg) 
{
    return "["+msg.timestamp()+" thread"+msg.threadID()+" "+msg.file()+",line"+msg.line()+ " function "+msg.function()+"]"+ msg.level()+": ";
}
void log_init()
{
    g3log_logger = g3::LogWorker::createLogWorker();
    auto handle = g3log_logger->addDefaultLogger("test","");
    g3::initializeLogging(g3log_logger.get());
    auto changeFormatting = handle->call(&g3::FileSink::overrideLogDetails,my_format);
    auto changeHeader = handle->call(&g3::FileSink::overrideLogHeader, "\n");
    changeFormatting.wait();
    changeHeader.wait();
}
void log_test(struct LogData *p)
{
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        LOG(INFO) << "Hello world, datai=" << p->datai[i] << ",datad=" << p->datad[i] << ",datas=" << p->datas+i*32;
    }
}
#endif

#ifdef USE_EASYLOGGINGPP
#define ELPP_THREAD_SAFE
#include "easylogging++.h"
#include "easylogging++.cc"
INITIALIZE_EASYLOGGINGPP
el::Configurations easyloggingpp_logger;
void log_init()
{
    easyloggingpp_logger.setGlobally(el::ConfigurationType::Enabled, "true");
    easyloggingpp_logger.setGlobally(el::ConfigurationType::ToFile, "true");
    easyloggingpp_logger.setGlobally(el::ConfigurationType::Filename, "test_log_EasyloggingPP.log");
    easyloggingpp_logger.setGlobally(el::ConfigurationType::MaxLogFileSize, "1048576");
    
    easyloggingpp_logger.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
    easyloggingpp_logger.setGlobally(el::ConfigurationType::Format,"[%datetime thread%thread %file,line %line function %func]%level: %msg");
    el::Loggers::reconfigureAllLoggers(easyloggingpp_logger);
}
void log_test(struct LogData *p)
{
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        LOG(INFO) << "Hello world, datai=" << p->datai[i] << ",datad=" << p->datad[i] << ",datas=" << p->datas+i*32;
    }
}
#endif

#ifdef USE_MORN
void log_init()
{
    mPropertyWrite("Log","log_file","./test_log_Morn.log");
    int size=1024*1024;mPropertyWrite("Log","log_filesize"  ,&size,sizeof(int));
    int num=2;         mPropertyWrite("Log","log_filerotate",&num ,sizeof(int));
}
void log_test(struct LogData *p)
{
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        mLog(MORN_INFO,mLogFormat5("Hello world, datai=%d, datad=%f, datas=%s"),p->datai[i],p->datad[i],p->datas+i*32);
    }
}
#endif

void test(struct LogData *p,int thread_num)
{
    for(int i=1;i<thread_num;i++)
        mThread(log_test,p);
    log_test(p);
    mThreadJoin();
}

int main(int argc, char** argv)
{
    char datas[100][32];
    int datai[100];
    double datad[100];
    for(int i=0;i<100;i++)
    {
        datai[i]=mRand(DFLT,DFLT);
        datad[i]=(double)mRand(-1000000,1000000)/1234567.0;
        mRandString(&(datas[i][0]),31);
    }
    struct LogData data={.datai=datai,.datad=datad,.datas=(char *)datas};
    
    log_init();
    
    data.N=1000000;
    printf("thread_num=1:\n");
    mTimerBegin();
    test(&data,1);
    mTimerEnd();

    data.N=100000;
    printf("thread_num=10:\n");
    mTimerBegin();
    test(&data,10);
    mTimerEnd();
     
    data.N=10000;
    printf("thread_num=100:\n");
    mTimerBegin();
    test(&data,100);
    mTimerEnd();

    return 0;
}


