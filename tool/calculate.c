/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_math.h"
#include "morn_help.h"

#ifdef _MSC_VER
#define stricmp _stricmp
#else
#define stricmp strcasecmp
#endif

int main(int argc,char *argv[])
{
    ToolHelp(infomation);
    mException((argc >1),EXIT,"invalid input");
    
    char str[2048];
    while(1)
    {
        printf(">");
        fgets(str,2048,stdin);
        if(str[0]=='\n') continue;
        if(stricmp(str,"exit\n")==0) break;
        printf("%.16f\n",mCalculate(str));
    }
    return 0;
}