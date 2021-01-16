/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_math.h"

#ifdef _MSC_VER
#define stricmp _stricmp
#else
#define stricmp strcasecmp
#endif

struct CaculateFunction
{
    char name[32];
    union
    {
        double (*func1)(double);
        double (*func2)(double,double);
        double (*func3)(double,double,double);
    };
};
struct CaculateFunction morn_caculate_function[128];
int morn_caculate_function_num=0;
void m_CaculateFunction(const char *name,void *func)
{
    mException(func==NULL,EXIT,"invalid input");
    int num=morn_caculate_function_num;
    mException(num>128,EXIT,"caculate function register error");
    morn_caculate_function_num+=1;
    strncpy(morn_caculate_function[num].name,name,32);
    morn_caculate_function[num].func1 = (double (*)(double))func;
}

struct CaculateInfo
{
    char type;
    double value;
};

double GetValue(char **ptr);

double Caculate(char **ptr)
{
    MList *list = mListCreate(0,NULL);
    
    char *p = *ptr;
    struct CaculateInfo info;
    info.type=(*p=='-')?(p++,'-'):'+';
    info.value = GetValue(&p);
    mListWrite(list,DFLT,&info,sizeof(struct CaculateInfo));
    
    while((*p!=0)&&(*p!=')')&&(*p!=','))
    {
        if((*p!='+')&&(*p!='-')&&(*p!='*')&&(*p!='/')&&(*p!='%')&&(*p!='^')) return NAN;
        info.type=*p; p++;
        *ptr=p;info.value=GetValue(ptr);if(mIsNan(info.value)) return NAN;
        p=*ptr;
        mListWrite(list,DFLT,&info,sizeof(struct CaculateInfo));
    }
    
    for(int i=list->num-1;i>0;i--)
    {
        struct CaculateInfo *cal = (struct CaculateInfo *)(list->data[i  ]);
        struct CaculateInfo *pre = (struct CaculateInfo *)(list->data[i-1]);
        if(cal->type=='^') {cal->type='=';cal->value=pow(pre->value,cal->value);pre->value=cal->value;}
    }
    
    for(int i=1;i<list->num;i++)
    {
        struct CaculateInfo *cal = (struct CaculateInfo *)(list->data[i  ]);
        struct CaculateInfo *pre = (struct CaculateInfo *)(list->data[i-1]);
             if(cal->type=='=') {cal->value= pre->value;}
        else if(cal->type=='*') {cal->type='=';cal->value=pre->value*cal->value;}
        else if(cal->type=='/') {cal->type='=';cal->value=pre->value/cal->value;}
        else if(cal->type=='%') {cal->type='=';cal->value=fmod(pre->value,cal->value);}
    }
    
    for(int i=list->num-1;i>0;i--)
    {
        struct CaculateInfo *cal = (struct CaculateInfo *)(list->data[i  ]);
        struct CaculateInfo *pre = (struct CaculateInfo *)(list->data[i-1]);
        if(cal->type=='=') {pre->value=cal->value;}
    }
    
    double rst=0;
    for(int i=0;i<list->num;i++)
    {
        struct CaculateInfo *cal = (struct CaculateInfo *)(list->data[i]);
             if(cal->type=='+') rst+=cal->value;
        else if(cal->type=='-') rst-=cal->value;
    }
    
    mListRelease(list);
    *ptr = p;
    
    return rst;
}

double GetValue(char **ptr)
{
    char *p = *ptr;
    double value,v1,v2,v3;
    
    int sign=0;if(*p=='-') {sign=1;p++;} else if(*p=='+') {sign=0;p++;} 
    char *str=p;
    
    while((*p!=0)&&(*p!=')')&&(*p!=',')&&(*p!='+')&&(*p!='-')&&(*p!='*')&&(*p!='/')&&(*p!='%')&&(*p!='^'))
    {
        if(*p=='(')
        {
            p[0]=0;p=p+1;*ptr=p;
            
            if(p==str+1) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?value:NAN;}
            
            if(stricmp(str,"abs"  )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?  ABS(value):NAN;}
            if(stricmp(str,"sqrt" )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')? sqrt(value):NAN;}
            if(stricmp(str,"exp"  )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?  exp(value):NAN;}
            if(stricmp(str,"ln"   )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?  log(value):NAN;}
            if(stricmp(str,"log10")==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?log10(value):NAN;}
            if(stricmp(str,"sin"  )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?  sin(value):NAN;}
            if(stricmp(str,"cos"  )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?  cos(value):NAN;}
            if(stricmp(str,"tan"  )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?  tan(value):NAN;}
            if(stricmp(str,"asin" )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')? asin(value):NAN;}
            if(stricmp(str,"acos" )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')? acos(value):NAN;}
            if(stricmp(str,"atan" )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')? atan(value):NAN;}
            if(stricmp(str,"ceil" )==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')? ceil(value):NAN;}
            if(stricmp(str,"floor")==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?floor(value):NAN;}
            if(stricmp(str,"round")==0) {value=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?round(value):NAN;}
            
            if(stricmp(str,"min")==0) {v1=Caculate(ptr);p=*ptr;if(*p!=','){return NAN;}*ptr=p+1;v2=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?MIN(v1,v2):NAN;}
            if(stricmp(str,"max")==0) {v1=Caculate(ptr);p=*ptr;if(*p!=','){return NAN;}*ptr=p+1;v2=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?MAX(v1,v2):NAN;}
            if(stricmp(str,"pow")==0) {v1=Caculate(ptr);p=*ptr;if(*p!=','){return NAN;}*ptr=p+1;v2=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?pow(v1,v2):NAN;}
            if(stricmp(str,"log")==0) {v1=Caculate(ptr);p=*ptr;if(*p!=','){return NAN;}*ptr=p+1;v2=Caculate(ptr);p=*ptr;*ptr=p+1;return (*p==')')?(log(v2)/log(v1)):NAN;}

            for(int i=0;i<morn_caculate_function_num;i++)
            {
                if(stricmp(str,morn_caculate_function[i].name)==0)
                {
                    v1=Caculate(ptr);p=*ptr;*ptr=p+1; if(*p==')') return morn_caculate_function[i].func1(v1);
                    v2=Caculate(ptr);p=*ptr;*ptr=p+1; if(*p==')') return morn_caculate_function[i].func2(v1,v2);
                    v3=Caculate(ptr);p=*ptr;*ptr=p+1; if(*p==')') return morn_caculate_function[i].func3(v1,v2,v3);
                    return NAN;
                }
            }
            return NAN;
        }
        p++;*ptr=p;
    }
    if(((str[0]=='p')||(str[0]=='P'))&&((str[1]=='i')||(str[1]=='I'))) value=MORN_PI;
    else if((str[0]=='e')||(str[0]=='E')) value=MORN_E;
    else if((str[0]>='0')&&(str[0]<='9')) value = atof(str);
    else return NAN;
    
    return (sign==0)?value:(0-value);
}

double mCaculate(char *str)
{
    int l = strlen(str)+1; 
    char *buff = (char *)malloc(l*sizeof(char)); char *p=buff;
    int i=0,j=0;for(;i<l;i++) if(str[i]!=' ') {buff[j]=str[i];j++;}
    double rst = Caculate(&p);
    free(buff);
    return rst;
}
