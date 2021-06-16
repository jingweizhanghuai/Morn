/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build_mingw: gcc -O2 -fopenmp test_map.c -lmorn -o test_map.exe
// build_msvc: cl.exe -O2 -nologo -I ..\include\ test_map.c ..\lib\x64_msvc\libmorn.lib
#include "morn_util.h"

void test1()
{
    printf("\n\n");
    MObject *map = mMapCreate();
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
    p = mMapRead(map,"zero" );if(p!=NULL)printf("zero = %d\n",*p);else printf("zero  not found\n");
    p = mMapRead(map,"one"  );if(p!=NULL)printf("one  = %d\n",*p);else printf("one   not found\n"); 
    p = mMapRead(map,"two"  );if(p!=NULL)printf("two  = %d\n",*p);else printf("two   not found\n"); 
    p = mMapRead(map,"three");if(p!=NULL)printf("three= %d\n",*p);else printf("three not found\n");
    p = mMapRead(map,"four" );if(p!=NULL)printf("four = %d\n",*p);else printf("four  not found\n");
    p = mMapRead(map,"five" );if(p!=NULL)printf("five = %d\n",*p);else printf("five  not found\n");
    p = mMapRead(map,"six"  );if(p!=NULL)printf("six  = %d\n",*p);else printf("six   not found\n");
    p = mMapRead(map,"seven");if(p!=NULL)printf("seven= %d\n",*p);else printf("seven not found\n");
    p = mMapRead(map,"eight");if(p!=NULL)printf("eight= %d\n",*p);else printf("eight not found\n");
    p = mMapRead(map,"nine" );if(p!=NULL)printf("nine = %d\n",*p);else printf("nine  not found\n");
    p = mMapRead(map,"ten"  );if(p!=NULL)printf("ten  = %d\n",*p);else printf("ten   not found\n");
    printf("\n");

    mMapNodeDelete(map,"zero" ,DFLT);
    mMapNodeDelete(map,"one"  ,DFLT);
    mMapNodeDelete(map,"two"  ,DFLT);
    mMapNodeDelete(map,"three",DFLT);
    mMapNodeDelete(map,"four" ,DFLT);
    mMapNodeDelete(map,"five" ,DFLT);
    mMapNodeDelete(map,"six"  ,DFLT);
    mMapNodeDelete(map,"seven",DFLT);
    mMapNodeDelete(map,"eight",DFLT);
    mMapNodeDelete(map,"nine" ,DFLT);
    
    p = mMapRead(map,"zero" );if(p!=NULL)printf("zero = %d\n",*p);else printf("zero  deleted\n");
    p = mMapRead(map,"one"  );if(p!=NULL)printf("one  = %d\n",*p);else printf("one   deleted\n"); 
    p = mMapRead(map,"two"  );if(p!=NULL)printf("two  = %d\n",*p);else printf("two   deleted\n"); 
    p = mMapRead(map,"three");if(p!=NULL)printf("three= %d\n",*p);else printf("three deleted\n");
    p = mMapRead(map,"four" );if(p!=NULL)printf("four = %d\n",*p);else printf("four  deleted\n");
    p = mMapRead(map,"five" );if(p!=NULL)printf("five = %d\n",*p);else printf("five  deleted\n");
    p = mMapRead(map,"six"  );if(p!=NULL)printf("six  = %d\n",*p);else printf("six   deleted\n");
    p = mMapRead(map,"seven");if(p!=NULL)printf("seven= %d\n",*p);else printf("seven deleted\n");
    p = mMapRead(map,"eight");if(p!=NULL)printf("eight= %d\n",*p);else printf("eight deleted\n");
    p = mMapRead(map,"nine" );if(p!=NULL)printf("nine = %d\n",*p);else printf("nine  deleted\n");

    mMapRelease(map);
}

void test2()
{
    printf("\n\n");
    MChain *map = mMapCreate();
    
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
    n= 0;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 1;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n); 
    n= 2;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n); 
    n= 3;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 4;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 5;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 6;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 7;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 8;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 9;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n=10;p=mMapRead(map,&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    printf("\n");
    
    n= 0;mMapNodeDelete(map,&n,sizeof(int));
    n= 1;mMapNodeDelete(map,&n,sizeof(int));
    n= 2;mMapNodeDelete(map,&n,sizeof(int));
    n= 3;mMapNodeDelete(map,&n,sizeof(int));
    n= 4;mMapNodeDelete(map,&n,sizeof(int));
    n= 5;mMapNodeDelete(map,&n,sizeof(int));
    n= 6;mMapNodeDelete(map,&n,sizeof(int));
    n= 7;mMapNodeDelete(map,&n,sizeof(int));
    n= 8;mMapNodeDelete(map,&n,sizeof(int));
    n= 9;mMapNodeDelete(map,&n,sizeof(int));
    
    n= 0;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 1;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n); 
    n= 2;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n); 
    n= 3;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 4;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 5;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 6;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 7;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 8;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 9;p=mMapRead(map,&n,sizeof(int),NULL,DFLT);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);

    mMapRelease(map);
}

void test3()
{
    printf("\n\n");
    MChain *map = mMapCreate();

    //key is pointer
    int *a=NULL;int idx=1;
    mMapWrite(map,&a,sizeof(int *),&idx,sizeof(int));

    //key is int
    int b=1;idx++;
    mMapWrite(map,&b,sizeof(int),&idx,sizeof(int));

    //key is float
    float c=2;idx++;
    mMapWrite(map,&c,sizeof(float),&idx,sizeof(int));

    //key is array
    double d[4]={1,9,4,9};idx++;
    mMapWrite(map,d,(4*sizeof(double)),&idx,sizeof(int));

    //key is string
    char *e="test string";idx++;
    mMapWrite(map,e,strlen(e),&idx,sizeof(int));

    //key is struct
    struct {int d1;float d2;double d3[3];} f;idx++;
    mMapWrite(map,&f,sizeof(f),&idx,sizeof(int));

    printf("f idx=%d\n",*(int *)mMapRead(map,&f,sizeof(f),NULL,NULL));
    printf("e idx=%d\n",*(int *)mMapRead(map, e,strlen(e),NULL,NULL));
    printf("d idx=%d\n",*(int *)mMapRead(map, d,sizeof(d),NULL,NULL));
    printf("c idx=%d\n",*(int *)mMapRead(map,&c,sizeof(c),NULL,NULL));
    printf("b idx=%d\n",*(int *)mMapRead(map,&b,sizeof(b),NULL,NULL));
    printf("a idx=%d\n",*(int *)mMapRead(map,&a,sizeof(a),NULL,NULL));

    mMapRelease(map);
}

void test4()
{
    printf("\n\n");
    MChain *map = mMapCreate(NULL);
    
    char *a="abcd";int idx=1;
    mMapWrite(map,a,strlen(a),&idx,sizeof(int));
    
    unsigned char b[4]={97,98,99,100};idx=2;
    mMapWrite(map,b,sizeof(b),&idx,sizeof(int));
    
    int c = 1684234849;idx=3;
    mMapWrite(map,&c,sizeof(c),&idx,sizeof(int));
    
    float d = 16777999408082104000000.0f;idx=4;
    mMapWrite(map,&d,sizeof(d),&idx,sizeof(int));
    
    printf("a idx=%d\n",*(int *)mMapRead(map, a,strlen(a),NULL,NULL));
    printf("b idx=%d\n",*(int *)mMapRead(map, b,sizeof(b),NULL,NULL));
    printf("c idx=%d\n",*(int *)mMapRead(map,&c,sizeof(c),NULL,NULL));
    printf("d idx=%d\n",*(int *)mMapRead(map,&d,sizeof(d),NULL,NULL));
    
    mMapRelease(map);
}

int main()
{
    test1();
    test2();
    test3();
    test4();
}
