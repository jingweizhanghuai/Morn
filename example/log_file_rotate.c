#include "morn_ptc.h"

int main()
{
                          mPropertyWrite("Log","log_file"      ,"./log_file_rotate.log");
    int filesize=64*1024; mPropertyWrite("Log","log_filesize"  ,&filesize  ,sizeof(int));
    int filerotate  =  5; mPropertyWrite("Log","log_filerotate",&filerotate,sizeof(int));
    
    for(int i=0;i<100000;i++)
    {
        mSleep(1);
        mLog(MORN_INFO,mLogFormat5("Hello world i=%d"),i);
    }
    
    return 0;
}
