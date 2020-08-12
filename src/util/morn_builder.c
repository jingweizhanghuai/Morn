/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

#define MORN_PC_Name             0
#define MORN_PC_Description      1
#define MORN_PC_URL              2
#define MORN_PC_Version          3
#define MORN_PC_Requires         4
#define MORN_PC_Requires_private 5
#define MORN_PC_Conflicts        6
#define MORN_PC_Cflags           7
#define MORN_PC_Libs             8
#define MORN_PC_Libs_private     9

void PCRequires(char *data,int flag,MList *out)
{
    char rst[8+32];rst[0]=flag;rst[1]=0;
    char name[32];char sign[4]={0,0,0,0};int v[6]={0,0,0,0,0,0};
    char *p=data;while(*p==' ') p++;
    while(*p!=0)
    {
        if((p[0]==' ')&&(p[1]!=' ')&&(p[1]!='>')&&(p[1]!='=')&&(p[1]!='<')&&
         (((p[1]>='A')&&(p[1]<='z'))||((p[2]>='A')&&(p[2]<='z'))))
        {
            p[0]=0;
            int n=sscanf(data,"%[^ ^>^=^<] %[>=<] %d.%d.%d.%d.%d.%d",name,sign,v,v+1,v+2,v+3,v+4,v+5);
            if(n>1)
            {
                if(strcmp(sign,">=")==0) rst[1]=1;
                if(strcmp(sign,">" )==0) rst[1]=2;
                if(strcmp(sign,"<=")==0) rst[1]=3;
                if(strcmp(sign,"<" )==0) rst[1]=4;
            }
            if(n>2) {rst[2]=v[0];rst[4]=v[1];rst[5]=v[2];rst[6]=v[3];rst[7]=v[4];rst[8]=v[5];}
            else memset(rst+2,0,6);
            strcpy(rst+8,name);
            mListWrite(out,DFLT,rst,40*sizeof(char));
            
            data=p+1;
        }
        p=p+1;
    }
}

void mPCLoad(char *filename,MList *out)
{
    FILE *f = fopen(filename,"r");
    fseek(f,0,SEEK_END);int size=ftell(f);fseek(f,0,SEEK_SET);
    char *data = (char *)mMalloc(size);char zero=0;
    fclose(f);

    mListClear(out);
    
    char buff[1024];int flag;
    MList *list = mListCreate(DFLT,NULL);

    char *info[4];
    info[0]=data;info[1]=&zero;info[2]=&zero;info[3]=&zero;
    for(char *p=data;p<data+size;p++)
    {
        if(flag==1)
        {
            if(p[0]=='#') {flag = 0;continue;}
            
                 if((p[0]=='=' )||(p[0]==':')) {p[0]=0;info[1]=p+1;}
            else if((p[0]=='$' )&&(p[1]=='{')) {p[0]=0;info[2]=p+2;}
            else if (p[0]=='}' )               {p[0]=0;info[3]=p+1;}
        }
        if(p[0]=='\n')
        {
            if(flag==1)
            {
                p[0]=0;
                mListWrite(list,DFLT,info,4*sizeof(char *));
            }
            flag=1;
            info[0]=p+1;info[1]=&zero;info[2]=&zero;info[3]=&zero;
        }
    }
    int n = list->num;
    for(int i=0;i<n;i++)
    {
        char **pinfo = (char **)(list->data[i]);
        if(pinfo[2]!=&zero)
        {
            char **info1;
            int j;for(j=0;j<i;j++) 
            {
                info1 = (char **)(list->data[j]);
                if(strcmp(info1[0],pinfo[2])==0) break;
            }
            mException((j==i),EXIT,"invalid pc file");
            sprintf(buff,"%s%s%s",pinfo[1],info1[1],pinfo[3]);
            pinfo[1] = mListWrite(list,DFLT,buff,DFLT);
        }

        if(memcmp(pinfo[0],"Name",4)==0)
        {
            sprintf(buff,"%c%s",MORN_PC_Name,pinfo[1]);
            mListWrite(out,DFLT,buff,DFLT);
        }
        else if(memcmp(pinfo[0],"Description",11)==0)
        {
            sprintf(buff,"%c%s",MORN_PC_Description,pinfo[1]);
            mListWrite(out,DFLT,buff,DFLT);
        }
        else if(memcmp(pinfo[0],"URL",3)==0)
        {
            sprintf(buff,"%c%s",MORN_PC_URL,pinfo[1]);
            mListWrite(out,DFLT,buff,DFLT);
        }
        else if(memcmp(pinfo[0],"Version",7)==0)
        {
            int v[6]={0,0,0,0,0,0};
            sscanf(pinfo[1],"%d.%d.%d.%d.%d.%d.%d",v,v+1,v+2,v+3,v+4,v+5,v+6);
            buff[0]=MORN_PC_Version;buff[1]=0;buff[2]=v[0];buff[3]=v[1];buff[4]=v[2];buff[5]=v[3];buff[6]=v[4];buff[7]=v[5];
            mListWrite(out,DFLT,buff,8);
        }
        else if(memcmp(pinfo[0],"Requires.private",16)==0)
            PCRequires(pinfo[1],MORN_PC_Requires_private,out);
        else if(memcmp(pinfo[0],"Requires",8)==0)
            PCRequires(pinfo[1],MORN_PC_Requires,out);
        else if(memcmp(pinfo[0],"Conflicts",9)==0)
            PCRequires(pinfo[1],MORN_PC_Conflicts,out);
        else if(memcmp(pinfo[0],"Cflags",6)==0)
        {
            char *p1;char *p2=pinfo[1];
            while(1)
            {
                p1=strstr(p2," ");if(p1!=NULL) {*p1=0;p1++;}
                sprintf(buff,"%c%s",MORN_PC_Cflags,p2);
                mListWrite(out,DFLT,buff,8);
                if(p1==NULL) break;
                p2=p1;
            }
        }
        else if(memcmp(pinfo[0],"Libs.private",12)==0)
        {
            char *p1;char *p2=pinfo[1];
            while(1)
            {
                p1=strstr(p2," ");if(p1!=NULL) {*p1=0;p1++;}
                sprintf(buff,"%c%s",MORN_PC_Libs_private,p2);
                mListWrite(out,DFLT,buff,8);
                if(p1==NULL) break;
                p2=p1;
            }
        }
        else if(memcmp(pinfo[0],"Libs",4)==0)
        {
            char *p1;char *p2=pinfo[1];
            while(1)
            {
                p1=strstr(p2," ");if(p1!=NULL) {*p1=0;p1++;}
                sprintf(buff,"%c%s",MORN_PC_Libs,p2);
                mListWrite(out,DFLT,buff,8);
                if(p1==NULL) break;
                p2=p1;
            }
        }
    }
}
