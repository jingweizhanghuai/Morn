//  gcc -O2 -fopenmp test_process_mutex.c -o test_process_mutex.exe -lmorn
#include <sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "morn_ptc.h"

void mProcMutexLock(const char *mutexname);
void mProcMutexUnlock(const char *mutexname);

int main()
{
    int File = open("./test_process_mutex.txt",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    int n=0;n=write(File,&n,sizeof(int));
    close(File);
    
    fork();fork();fork();
    
    File = open("./test_process_mutex.txt",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    int *num = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,File,0);
    close(File);

    int ID = getpid();
    volatile int a;
    for(int i=0;i<100000;i++)
    {
        mProcLockBegin("proctest");
        
        a = *num;
        *num=a+1;

        mProcLockEnd("proctest");
    }
    printf("process %d over. num=%d\n",getpid(),*num);
    munmap(num,sizeof(int));
}
