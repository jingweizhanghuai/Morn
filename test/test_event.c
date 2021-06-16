#include "morn_util.h"

void mEvent(void (*func)(void *),void *para);

long g_t;
void clocker(long *t)
{
    long t0=clock();
    if(t0>=t)
    {
        printf("t=%d,t0=%d\n",*t,t0);
        *t+=1000;
        mEvent(clocker,t);
    }
}

int main()
{
    g_t = clock();
    mEvent(clocker,&t);
    mSleep(10000);
}