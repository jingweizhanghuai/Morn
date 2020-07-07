/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_map.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -o test_map.exe
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
    p = mMapRead(map,"zero" ,DFLT,NULL,DFLT);if(p!=NULL)printf("zero = %d\n",*p);
    p = mMapRead(map,"one"  ,DFLT,NULL,DFLT);if(p!=NULL)printf("one  = %d\n",*p); 
    p = mMapRead(map,"two"  ,DFLT,NULL,DFLT);if(p!=NULL)printf("two  = %d\n",*p); 
    p = mMapRead(map,"three",DFLT,NULL,DFLT);if(p!=NULL)printf("three= %d\n",*p);
    p = mMapRead(map,"four" ,DFLT,NULL,DFLT);if(p!=NULL)printf("four = %d\n",*p);
    p = mMapRead(map,"five" ,DFLT,NULL,DFLT);if(p!=NULL)printf("five = %d\n",*p);
    p = mMapRead(map,"six"  ,DFLT,NULL,DFLT);if(p!=NULL)printf("six  = %d\n",*p);
    p = mMapRead(map,"seven",DFLT,NULL,DFLT);if(p!=NULL)printf("seven= %d\n",*p);
    p = mMapRead(map,"eight",DFLT,NULL,DFLT);if(p!=NULL)printf("eight= %d\n",*p);
    p = mMapRead(map,"nine" ,DFLT,NULL,DFLT);if(p!=NULL)printf("nine = %d\n",*p);
    p = mMapRead(map,"ten"  ,DFLT,NULL,DFLT);if(p!=NULL)printf("ten  = %d\n",*p);

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
