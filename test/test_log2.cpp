/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_GLOG -o test_log2_glog.exe -lglog -lmorn
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_SPDLOG -o test_log2_spdlog.exe -lmorn
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_LOG4CPP -o test_log2_log4cpp.exe -llog4cpp -lmorn
// build: g++ -O2 -fopenmp test_log2.cpp -DUSE_MORN -o test_log2_morn.exe -lmorn

#include "morn_ptc.h"

struct LogData
{
    int *datai;
    double *datad;
    char *datas;
    int N;
};

#ifdef USE_GLOG
#include "glog/logging.h"
void test_glog(struct LogData *p)
{
    google::InitGoogleLogging("test_log");
    google::SetLogDestination(google::GLOG_INFO, "./test_log_glog.log");
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        LOG(INFO)<<": Hello glog, "<<"datai="<<p->datai[i]<<", datad="<<p->datad[i]<<", datas="<< p->datas+i*32;
    }
    google::ShutdownGoogleLogging();
}
#endif

#ifdef USE_SPDLOG
#include "spdlog/spdlog.h"
void test_spdlog(struct LogData *p)
{
    spdlog::set_async_mode(4096);
    auto logger = spdlog::daily_logger_st("test_log", "test_log_spdlog.log");
    spdlog::set_pattern("[%Y.%m.%d %H:%M:%S thread%t]%l: %v");
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        logger->info("[{} line {},function {}] Hello spdlog, datai={}, datad={}, datas={}",__FILE__,__LINE__,__FUNCTION__,p->datai[i],p->datad[i],p->datas+i*32);
    }
}
#endif

#ifdef USE_LOG4CPP
#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"
void test_log4cpp(struct LogData *p)
{
    log4cpp::FileAppender * appender = new log4cpp::FileAppender("appender","./test_log_log4cpp.log");
    log4cpp::PatternLayout* pLayout = new log4cpp::PatternLayout();
    pLayout->setConversionPattern("[%d thread%t %c]%p: %m%n");
    appender->setLayout(pLayout);
    log4cpp::Category& root =log4cpp::Category::getRoot();
    log4cpp::Category& infoCategory =root.getInstance("test_log");
    infoCategory.addAppender(appender);
    infoCategory.setPriority(log4cpp::Priority::INFO);
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        infoCategory.info("[%s,line %d,function %s] Hello log4cpp, datai=%d, datad=%f, datas=%s",__FILE__,__LINE__,__FUNCTION__,p->datai[i],p->datad[i],p->datas+i*32);
    }
    log4cpp::Category::shutdown();
}
#endif

#ifdef USE_MORN
void test_morn(struct LogData *p)
{
    mPropertyWrite("Log","log_file","./test_log_morn.log");
    for(int n=0;n<p->N;n++)
    {
        int i=n%100;
        mLog(MORN_INFO,mLogFormat5("Hello Morn, datai=%d, datad=%f, datas=%s"),p->datai[i],p->datad[i],p->datas+i*32);
    }
}
#endif

int main(int argc, char** argv)
{
    char datas[100][32];
    int datai[100];
    double datad[100];
    for(int i=0;i<100;i++)
    {
        datai[i]=mRand(DFLT,DFLT);
        datad[i]=(double)mRand(-1000000,1000000)/1000000.0;
        mRandString(&(datas[i][0]),15,31);
    }
    struct LogData data={.datai=datai,.datad=datad,.datas=(char *)datas,.N=1000000};
    
    #ifdef USE_GLOG
    mTimerBegin("glog");
    test_glog(&data);
    mTimerEnd("glog");
    #endif
    
    #ifdef USE_SPDLOG
    mTimerBegin("spdlog");
    test_spdlog(&data);
    mTimerEnd("spdlog");
    #endif

    #ifdef USE_LOG4CPP
    mTimerBegin("log4cpp");
    test_log4cpp(&data);
    mTimerEnd("log4cpp");
    #endif
    
    #ifdef USE_MORN
    mTimerBegin("Morn");
    test_morn(&data);
    mTimerEnd("Morn");
    #endif

    return 0;
}

