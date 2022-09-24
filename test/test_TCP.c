//build_mingw: gcc -O2 -fopenmp test_TCP.c -o test_TCP.exe -lmorn -lWs2_32
#include "morn_util.h"

void test_send()
{
    char addr[16];

    char data[64];
    for(int i=0;i<100;i++)
    {
        mRandString(data,32,64);
        // sprintf(addr,"localhost:%d",rand()%4+1000);
        mTCPClientWrite("192.168.5.43:100",data,DFLT);
        printf("send: size=%zd, data is %s\n",strlen(data),data);
        mSleep(100);
    }
}

void test_recive()
{
    char data[64];
    while(1)
    {
        int size=64;
        char *ip = mTCPServerRead(":100",data,&size);
        if(strcmp(data,"exit")==0) break;
        if(ip==NULL) mSleep(100);
        else printf("recive: size=%d, data is %s\n",size,data);
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"send"  )==0) test_send();
    else if(strcmp(argv[1],"recive")==0) test_recive();
    // else mUDPWrite("localhost:1234",argv[1],DFLT);
    return 0;
}

