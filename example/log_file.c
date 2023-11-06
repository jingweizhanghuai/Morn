#include "morn_ptc.h"

int main()
{
    mPropertyWrite("Log","log_file","./log_file_example.log");
    
    for(int i=0;i<2000;i++)
    {
        mSleep(10);
        mLog(MORN_INFO,mLogFormat5("Hello world i=%d"),i);
    }
    
    return 0;
}
    

