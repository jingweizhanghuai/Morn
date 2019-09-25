/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_INI_file.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_INI_file.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_util.h"

int main()
{
    int i,j;
    char *value;
    MFile *file = mFileCreate("./test_INI_file.ini");
    MList *student = mINISection(file);
    MList *score = mListCreate(DFLT,NULL);
    for(i=0;i<student->num;i++)
    {
        value = mINIRead(file,student->data[i],"年级");
        int grade = atoi(value);
        value = mINIRead(file,student->data[i],"班级");
        int class = atoi(value);
        char *label = mINIRead(file,student->data[i],"类别");
        float sum=0.0f;
        for(j=1;;j++)
        {
            char name[16];sprintf(name,"成绩%d",j);
            value = mINIRead(file,student->data[i],name);
            if(value==NULL) break;
            mStringSplit(value,"(",score);
            sum+=atof(score->data[1]);
        }
        printf("学生：%s %d年级%d班 %s 平均成绩:%f\n",
        student->data[i],grade,class,label,sum/(j-1));
    }
    mListRelease(score);
    mFileRelease(file);
}
