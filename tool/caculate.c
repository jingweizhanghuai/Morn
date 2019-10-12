/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_math.h"

#ifdef _MSC_VER
#define stricmp _stricmp
#else
#define stricmp strcasecmp
#endif

int main()
{   
    char str[2048];
    while(1)
    {
        printf(">");
        gets(str);
        if(str[0]==0) continue;
        if(stricmp(str,"exit")==0) break;
        printf("result is %lf\n",mCaculate(str));
    }
    return 0;
}