/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_math.h"
#include "morn_help.h"

int main(int argc,char *argv[])
{
    mException((argc != 2),EXIT,"invalid input");
    ToolHelp(infomation);
    
    unsigned int out = mHash(argv[1],DFLT);
    printf("0x%x\n",out);
}
