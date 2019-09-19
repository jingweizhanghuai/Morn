#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Util.h"
#include "morn_Help.h"

int main(int argc,char *argv[])
{
    HELP_INFOMATION("Crypt");
    
    char *file_in;int flag;
    {
        file_in = mStringArgument(argc,argv,"e",NULL);
        if(file_in!=NULL) flag=0;
    }
    if(file_in==NULL)
    {
        file_in = mStringArgument(argc,argv,"d",NULL);
        if(file_in!=NULL) flag=1;
    }
    mException((file_in==NULL),EXIT,"invalid input file");
    
    char *file_out = mStringArgument(argc,argv,"o",NULL);
    mException((file_in==NULL),EXIT,"invalid output file");
    
    uint64_t key=DFLT;
    char *p_key = mStringArgument(argc,argv,"k",NULL);
    if(p_key != NULL) {key=0;memcpy(&key,p_key,MIN(8,strlen(p_key)));}
    
    if(flag==0) mEncrypt(file_in,file_out,key);
    else        mDecrypt(file_in,file_out,key);
}
