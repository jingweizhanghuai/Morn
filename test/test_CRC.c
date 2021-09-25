#include "morn_util.h"

uint32_t mCRC32(void *in,int size);

// E8B7BE43
int main()
{
    uint32_t rst = mCRC32("a",1);
    printf("rst = %x\n",rst);
    return 0;
}