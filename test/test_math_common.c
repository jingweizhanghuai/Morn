#include "morn_math.h"

int main()
{
    for(int i=-300;i<300;i++)
    {
        printf("%d: ceil=%d,floor=%d,round=%d\n",i,mBinaryCeil(i),mBinaryFloor(i),mBinaryRound(i));
    }
    return 0;
}