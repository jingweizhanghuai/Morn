/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//build: gcc -O2 -fopenmp test_INI_file.c -lmorn -o test_INI_file.exe

#include "morn_util.h"

int main()
{
    int i,j;
    MList *ini = mListCreate();
    mINILoad(ini,"./test_INI_file.ini");

    for(i=0;i<ini->num;i++)
    {
        int grade;int class;
        mINIRead(ini,ini->data[i],"年级","%d",&grade);
        mINIRead(ini,ini->data[i],"班级","%d",&class);
        char *label = mINIRead(ini,ini->data[i],"类别");
        float sum=0.0f;
        for(j=1;;j++)
        {
            char name[16];sprintf(name,"成绩%d",j);float score;
            if(mINIRead(ini,ini->data[i],name,"%[^(](%f)",name,&score)==NULL) break;
            sum+=score;
        }
        printf("学生：%s %d年级%d班 %s 平均成绩:%f\n",(char *)(ini->data[i]),grade,class,label,sum/(j-1));
    }

    mINIWrite(ini,"张三","成绩1","%s(%d)","数学",0);
    mINIWrite(ini,"张三","成绩5","%s(%d)","生物",75);
    
    mINIWrite(ini,"泾渭漳淮","班级","%d",5);
    mINIWrite(ini,"泾渭漳淮","年级","%d",6);
    mINIWrite(ini,"泾渭漳淮","类别","理科");
    mINIWrite(ini,"泾渭漳淮","成绩1","%s(%d)","数学",100);
    mINIWrite(ini,"泾渭漳淮","成绩2","%s(%d)","计算机",100);
    

    mINIDelete(ini,"李四","成绩5");
    mINIDelete(ini,"王二麻");
    
    mINISave(ini,"./test_INI_file_out.ini");
    
    mListRelease(ini);
}
