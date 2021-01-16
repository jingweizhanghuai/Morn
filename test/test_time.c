/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// gcc -O2 -fopenmp test_time.c -o test_time.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn
#include "morn_util.h"

int main()
{
    printf("%s\n",mTimeString());
    printf("%s\n",mTimeString(NULL));
    printf("%s\n",mTimeString("%Y.%02M.%02D %02H:%02m:%02S"));
    printf("%s\n",mTimeString(DFLT,"%Y.%02M.%02D %02H:%02m:%02S"));
    printf("%s\n",mTimeString(time(NULL),"%Y.%02M.%02D %02H:%02m:%02S"));
    
    printf("%s\n",mTimeString("%sM %D %Y %H:%m:%S %sW"          ));
    printf("%s\n",mTimeString("%Y %aM %D %aW %H:%m:%S"         ));
    printf("%s\n",mTimeString("%02H:%02m:%02S %Y.%02M.%02D %aW"));
    printf("%s\n",mTimeString("%CY年%CM月%CD日 星期%CW %CH时%Cm分%CS秒"));
    printf("\n");

    printf("%s\n",mTimeString(1218196800,NULL));
    printf("%s\n",mTimeString(1218196800,"%sM %D %Y %H:%m:%S %sW"          ));
    printf("%s\n",mTimeString(1218196800,"%Y %aM %D %aW %H:%m:%S"         ));
    printf("%s\n",mTimeString(1218196800,"%02H:%02m:%02S %Y.%02M.%02D %aW"));
    printf("%s\n",mTimeString(1218196800,"%CY年%CM月%CD日 星期%CW %CH时%Cm分%CS秒"));
    printf("\n");

    printf("%lld\n",mStringTime("2008.08.08 20:00:00"));
    printf("%lld\n",mStringTime("August 8 2008 20:0:0 Friday","%sM %D %Y %H:%m:%S %sW"));
    printf("%lld\n",mStringTime("2008 Aug 8 Sun 20:0:0","%Y %aM %D %aW %H:%m:%S"));
    printf("%lld\n",mStringTime("20:00:00 2008.08.08 Fri","%02H:%02m:%02S %Y.%02M.%02D %aW"));
    printf("%lld\n",mStringTime("二〇〇八年八月八日 星期五 二十时零分零秒","%CY年%CM月%CD日 星期%CW %CH时%Cm分%CS秒"));
    printf("\n");

    printf("今天是%s\n",mTimeString("%Y.%M.%D %aW"));
    printf("1000天后是%s\n",mTimeString(time(NULL)+1000*24*3600,"%Y.%M.%D %aW"));
    printf("今天已经建国%d天\n",(time(NULL)-mStringTime("1949.10.1","%Y.%M.%D"))/(24*3600));
    printf("距离2021年高考还有%d天\n",(mStringTime("2021.6.7","%Y.%M.%D")-time(NULL))/(24*3600));
    printf("汶川大地震发生在星期%s\n",mTimeString(mStringTime("2008.5.12","%Y.%M.%D"),"%CW"));
    
    return 0;
}