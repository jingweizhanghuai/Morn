// build: gcc -O2 test_process_topic.c -liceoryx_binding_c -liceoryx_posh -liceoryx_utils -liceoryx_platform -lstdc++ -lpthread -lrt -lmorn -lm -o test_process_topic.exe

#include "morn_ptc.h"

void publisher()
{
    char data[64];
    for(int i=0;i<100;i++)
    {
        mSleep(1000);
        mRandString(data,16,64);
        printf("string= %s\n",data);
        mProcTopicWrite("string",data);
    }
}

void subscriber()
{
    while(1)
    {
        mSleep(100);
        char *p=mProcTopicRead("string");
        if(p!=NULL) 
        {
            if(p[0]==0) return;
            printf("string= %s\n",p);
        }
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"publisher" )==0)  publisher();
    else if(strcmp(argv[1],"subscriber")==0) subscriber();
    else if(strcmp(argv[1],"exit"      )==0) {char data=0;mProcTopicWrite("string",&data,1);}
    else printf("run as: \"test_process_topic.exe publisher\", or \"test_process_topic.exe subscriber\"\n");
    return 0;
}
