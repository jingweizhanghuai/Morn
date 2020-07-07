/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#define HELP_FILE "./help.txt"

#define fgets(Buff,Num,F) mException((fgets(Buff,Num,F)==NULL),EXIT,"file read error")

#define HELP_INFOMATION(name) {\
    char *arg_flag;\
    arg_flag=mStringArgument(argc,argv,"-help",NULL);\
    if(arg_flag!=NULL) {mToolHelp(name); return 1;}\
    arg_flag=mStringArgument(argc,argv,"v",NULL);\
    if(arg_flag!=NULL) {mToolHelp("version"); return 1;}\
}

void mToolHelp(const char *name)
{
    FILE *f;
    char buff[2048];
    int len;
    
    len = strlen(name);
    
    f = fopen(HELP_FILE,"r");
    
    while(!feof(f))
    {
        fgets(buff,2048,f);
        if(buff[0] == '[')
        {
            if(((int)strspn(buff+1,name)>=len)&&(buff[len+1] == ']'))
                break;
        }
    }
    
    fgets(buff,2048,f);
    while((buff[0] != '[')&&(!feof(f)))
    {
        printf("%s",buff);
        fgets(buff,2048,f);
    }
    
    fclose(f);
}
