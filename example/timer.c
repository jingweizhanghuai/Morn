#include "morn_util.h"

int main()
{
    double t1=mTimer();
    mSleep(100);
    double t2=mTimer();
    printf("t2-t1=%f ms\n",t2-t1);
}
