#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_Math.h"
#include "morn_Help.h"

int main(int argc,char *argv[])
{
    unsigned int out;

    HELP_INFOMATION("hash_value");
    
    mException((argc != 2),"invalid input",EXIT);
    
    out = mHash(argv[1]);
    printf("0x%x\n",out);
}
