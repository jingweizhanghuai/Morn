/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"
#include "morn_help.h"

int main(int argc,char *argv[])
{
    ToolHelp(crypt);
    
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
