// g++ -O2 test_CRC2.cpp -o test_CRC2.exe -lmorn
#include "morn_util.h"
#include <boost/crc.hpp>

void test_boost()
{
    FILE *f=fopen("a_Q_zheng_zhuan.txt","rb");
    int size = fsize(f);
    char *data=(char *)malloc(size);
    size=fread(data,1,size,f);
    fclose(f);
    
    uint32_t rst;
    
    mTimerBegin("boost");
    for(int i=0;i<1000;i++)
    {
        boost::crc_32_type  crc32;
        crc32.process_bytes(data,size);
        rst=crc32.checksum();
    }
    mTimerEnd("boost");
    
    printf("crc=%x\n",rst);
    free(data);
}

void test_Morn()
{
    FILE *f=fopen("a_Q_zheng_zhuan.txt","rb");
    int size = fsize(f);
    char *data=(char *)malloc(size);
    size=fread(data,1,size,f);
    fclose(f);
    
    uint32_t rst;
    
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        rst=mCRC(data,size);
    mTimerEnd("Morn");
    
    printf("crc=%x\n",rst);
    free(data);
}

int main(int arc,char *argv[])
{
    if(strcmp(argv[1],"boost")==0) test_boost();
    else                           test_Morn();
    return 0;
}


