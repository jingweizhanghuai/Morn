
#include "morn_util.h"
void test1()
{
    int n;
    n=0; mDictionaryWrite("zero" ,DFLT,&n,sizeof(int));
    n=1; mDictionaryWrite("one"  ,DFLT,&n,sizeof(int));
    n=2; mDictionaryWrite("two"  ,DFLT,&n,sizeof(int));
    n=3; mDictionaryWrite("three",DFLT,&n,sizeof(int));
    n=4; mDictionaryWrite("four" ,DFLT,&n,sizeof(int));
    n=5; mDictionaryWrite("five" ,DFLT,&n,sizeof(int));
    n=6; mDictionaryWrite("six"  ,DFLT,&n,sizeof(int));
    n=7; mDictionaryWrite("seven",DFLT,&n,sizeof(int));
    n=8; mDictionaryWrite("eight",DFLT,&n,sizeof(int));
    n=9; mDictionaryWrite("nine" ,DFLT,&n,sizeof(int));
    
    int *p;
    p = mDictionaryRead("zero" );if(p!=NULL)printf("zero = %d\n",*p);else printf("zero  not found\n");
    p = mDictionaryRead("one"  );if(p!=NULL)printf("one  = %d\n",*p);else printf("one   not found\n"); 
    p = mDictionaryRead("two"  );if(p!=NULL)printf("two  = %d\n",*p);else printf("two   not found\n"); 
    p = mDictionaryRead("three");if(p!=NULL)printf("three= %d\n",*p);else printf("three not found\n");
    p = mDictionaryRead("four" );if(p!=NULL)printf("four = %d\n",*p);else printf("four  not found\n");
    p = mDictionaryRead("five" );if(p!=NULL)printf("five = %d\n",*p);else printf("five  not found\n");
    p = mDictionaryRead("six"  );if(p!=NULL)printf("six  = %d\n",*p);else printf("six   not found\n");
    p = mDictionaryRead("seven");if(p!=NULL)printf("seven= %d\n",*p);else printf("seven not found\n");
    p = mDictionaryRead("eight");if(p!=NULL)printf("eight= %d\n",*p);else printf("eight not found\n");
    p = mDictionaryRead("nine" );if(p!=NULL)printf("nine = %d\n",*p);else printf("nine  not found\n");
    p = mDictionaryRead("ten"  );if(p!=NULL)printf("ten  = %d\n",*p);else printf("ten   not found\n");

    mDictionaryNodeDelete("zero" );
    mDictionaryNodeDelete("one"  );
    mDictionaryNodeDelete("two"  );
    mDictionaryNodeDelete("three");
    mDictionaryNodeDelete("four" );
    mDictionaryNodeDelete("five" );
    mDictionaryNodeDelete("six"  );
    mDictionaryNodeDelete("seven");
    mDictionaryNodeDelete("eight");
    mDictionaryNodeDelete("nine" );
    
    p = mDictionaryRead("zero" );if(p!=NULL)printf("zero = %d\n",*p);else printf("zero  deleted\n");
    p = mDictionaryRead("one"  );if(p!=NULL)printf("one  = %d\n",*p);else printf("one   deleted\n"); 
    p = mDictionaryRead("two"  );if(p!=NULL)printf("two  = %d\n",*p);else printf("two   deleted\n"); 
    p = mDictionaryRead("three");if(p!=NULL)printf("three= %d\n",*p);else printf("three deleted\n");
    p = mDictionaryRead("four" );if(p!=NULL)printf("four = %d\n",*p);else printf("four  deleted\n");
    p = mDictionaryRead("five" );if(p!=NULL)printf("five = %d\n",*p);else printf("five  deleted\n");
    p = mDictionaryRead("six"  );if(p!=NULL)printf("six  = %d\n",*p);else printf("six   deleted\n");
    p = mDictionaryRead("seven");if(p!=NULL)printf("seven= %d\n",*p);else printf("seven deleted\n");
    p = mDictionaryRead("eight");if(p!=NULL)printf("eight= %d\n",*p);else printf("eight deleted\n");
    p = mDictionaryRead("nine" );if(p!=NULL)printf("nine = %d\n",*p);else printf("nine  deleted\n");
}

void test2()
{
    int n;
    n=0; mDictionaryWrite(&n,sizeof(int),"zero" ,DFLT);
    n=1; mDictionaryWrite(&n,sizeof(int),"one"  ,DFLT);
    n=2; mDictionaryWrite(&n,sizeof(int),"two"  ,DFLT);
    n=3; mDictionaryWrite(&n,sizeof(int),"three",DFLT);
    n=4; mDictionaryWrite(&n,sizeof(int),"four" ,DFLT);
    n=5; mDictionaryWrite(&n,sizeof(int),"five" ,DFLT);
    n=6; mDictionaryWrite(&n,sizeof(int),"six"  ,DFLT);
    n=7; mDictionaryWrite(&n,sizeof(int),"seven",DFLT);
    n=8; mDictionaryWrite(&n,sizeof(int),"eight",DFLT);
    n=9; mDictionaryWrite(&n,sizeof(int),"nine" ,DFLT);
    
    char *p;
    n= 0;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 1;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n); 
    n= 2;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n); 
    n= 3;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 4;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 5;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 6;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 7;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 8;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n= 9;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    n=10;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d not found\n",n);
    
    n= 0;mDictionaryNodeDelete(&n,sizeof(int));
    n= 1;mDictionaryNodeDelete(&n,sizeof(int));
    n= 2;mDictionaryNodeDelete(&n,sizeof(int));
    n= 3;mDictionaryNodeDelete(&n,sizeof(int));
    n= 4;mDictionaryNodeDelete(&n,sizeof(int));
    n= 5;mDictionaryNodeDelete(&n,sizeof(int));
    n= 6;mDictionaryNodeDelete(&n,sizeof(int));
    n= 7;mDictionaryNodeDelete(&n,sizeof(int));
    n= 8;mDictionaryNodeDelete(&n,sizeof(int));
    n= 9;mDictionaryNodeDelete(&n,sizeof(int));
    
    n= 0;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 1;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n); 
    n= 2;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n); 
    n= 3;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 4;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 5;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 6;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 7;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 8;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
    n= 9;p=mDictionaryRead(&n,sizeof(int),NULL,NULL);if(p!=NULL)printf("%d = %s\n",n,p);else printf("%d deleted\n",n);
}

void test3()
{
    int idx;
    
    //key is pointer
    int *a=&idx;idx=1;
    mDictionaryWrite(&a,sizeof(int *),&idx,sizeof(int));

    //key is int
    int b=1;idx++;
    mDictionaryWrite(&b,sizeof(int),&idx,sizeof(int));

    //key is float
    float c=2;idx++;
    mDictionaryWrite(&c,sizeof(float),&idx,sizeof(int));

    //key is array
    double d[4]={1,9,4,9};idx++;
    mDictionaryWrite(d,(4*sizeof(double)),&idx,sizeof(int));

    //key is string
    char *e="test string";idx++;
    mDictionaryWrite(e,strlen(e),&idx,sizeof(int));

    //key is struct
    struct {int d1;float d2;double d3[3];} f;idx++;
    mDictionaryWrite(&f,sizeof(f),&idx,sizeof(int));

    printf("f idx=%d\n",*(int *)mDictionaryRead(&f,sizeof(f),NULL,NULL));
    printf("e idx=%d\n",*(int *)mDictionaryRead( e,strlen(e),NULL,NULL));
    printf("d idx=%d\n",*(int *)mDictionaryRead( d,sizeof(d),NULL,NULL));
    printf("c idx=%d\n",*(int *)mDictionaryRead(&c,sizeof(c),NULL,NULL));
    printf("b idx=%d\n",*(int *)mDictionaryRead(&b,sizeof(b),NULL,NULL));
    printf("a idx=%d\n",*(int *)mDictionaryRead(&a,sizeof(a),NULL,NULL));
}

int main()
{
    test1();
    test2();
    test3();
    return 0;
}