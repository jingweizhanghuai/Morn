/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//编译： gcc -O2 -fopenmp test_map.c -I ..\include\ -L ..\lib\x64_mingw -lmorn -o test_map.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

void test1()
{
    MChain *map = mChainCreate();
    
    int n;
    n=0; mMapWrite(map,"zero" ,DFLT,&n,sizeof(int));
    n=1; mMapWrite(map,"one"  ,DFLT,&n,sizeof(int));
    n=2; mMapWrite(map,"two"  ,DFLT,&n,sizeof(int));
    n=3; mMapWrite(map,"three",DFLT,&n,sizeof(int));
    n=4; mMapWrite(map,"four" ,DFLT,&n,sizeof(int));
    n=5; mMapWrite(map,"five" ,DFLT,&n,sizeof(int));
    n=6; mMapWrite(map,"six"  ,DFLT,&n,sizeof(int));
    n=7; mMapWrite(map,"seven",DFLT,&n,sizeof(int));
    n=8; mMapWrite(map,"eight",DFLT,&n,sizeof(int));
    n=9; mMapWrite(map,"nine" ,DFLT,&n,sizeof(int));
    
    int *p;
    p = mMapRead(map,"zero" );if(p!=NULL)printf("zero = %d\n",*p);
    p = mMapRead(map,"one"  );if(p!=NULL)printf("one  = %d\n",*p); 
    p = mMapRead(map,"two"  );if(p!=NULL)printf("two  = %d\n",*p); 
    p = mMapRead(map,"three");if(p!=NULL)printf("three= %d\n",*p);
    p = mMapRead(map,"four" );if(p!=NULL)printf("four = %d\n",*p);
    p = mMapRead(map,"five" );if(p!=NULL)printf("five = %d\n",*p);
    p = mMapRead(map,"six"  );if(p!=NULL)printf("six  = %d\n",*p);
    p = mMapRead(map,"seven");if(p!=NULL)printf("seven= %d\n",*p);
    p = mMapRead(map,"eight");if(p!=NULL)printf("eight= %d\n",*p);
    p = mMapRead(map,"nine" );if(p!=NULL)printf("nine = %d\n",*p);
    p = mMapRead(map,"ten"  );if(p!=NULL)printf("ten  = %d\n",*p);

    mChainRelease(map);
}

void test2()
{
    MChain *map = mChainCreate();
    
    int n;
    n=0; mMapWrite(map,&n,sizeof(int),"zero" ,DFLT);
    n=1; mMapWrite(map,&n,sizeof(int),"one"  ,DFLT);
    n=2; mMapWrite(map,&n,sizeof(int),"two"  ,DFLT);
    n=3; mMapWrite(map,&n,sizeof(int),"three",DFLT);
    n=4; mMapWrite(map,&n,sizeof(int),"four" ,DFLT);
    n=5; mMapWrite(map,&n,sizeof(int),"five" ,DFLT);
    n=6; mMapWrite(map,&n,sizeof(int),"six"  ,DFLT);
    n=7; mMapWrite(map,&n,sizeof(int),"seven",DFLT);
    n=8; mMapWrite(map,&n,sizeof(int),"eight",DFLT);
    n=9; mMapWrite(map,&n,sizeof(int),"nine" ,DFLT);
    
    char *p;
    n=0; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=1; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p); 
    n=2; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p); 
    n=3; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=4; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=5; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=6; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=7; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=8; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=9; p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    n=10;p = mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);
    
    mChainRelease(map);
}

int main()
{
    printf("test1:\n");
    test1();
    printf("test2:\n");
    test2();
}
