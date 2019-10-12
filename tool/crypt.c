/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Util.h"
#include "morn_Help.h"

int main(int argc,char *argv[])
{
    HELP_INFOMATION("crypt");
    
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
