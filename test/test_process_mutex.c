// gcc -O2 -fopenmp test_process_mutex.c -o test_process_mutex.exe -I ../include/ -L ../lib/x64_gnu/ -lmorn -lm
#include <sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "morn_util.h"

void mProcMutexLock(const char *mutexname);
void mProcMutexUnlock(const char *mutexname);

int main()
{
    fork();//fork();
    
    int File = open("./test_process_mutex.txt",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    int n=write(File,&n,sizeof(int));
    int *num = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,File,0);
    close(File);
    
    *num=0;

    int ID = getpid();
    volatile int a;
    mTimerBegin();
    for(int i=0;i<100000;i++)
    {
        mProcMutexLock("proctest");
        
        a = *num;
        // printf("ID=%d,*num=%d\n",ID,a);
        *num=a+1;

        mProcMutexUnlock("proctest");
    }
    mTimerEnd();
    printf("process %d over. num=%d\n",getpid(),*num);

    munmap(num,sizeof(int));
    
    remove("./test_process_mutex.txt");
    mSleep(1000);
}
