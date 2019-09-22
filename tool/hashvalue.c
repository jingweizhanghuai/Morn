#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_Math.h"
#include "morn_Help.h"

int main(int argc,char *argv[])
{
    HELP_INFOMATION("hashvalue");
    mException((argc != 2),"invalid input",EXIT);
    
    unsigned int out = mHash(argv[1],DFLT);
    printf("0x%x\n",out);
}
