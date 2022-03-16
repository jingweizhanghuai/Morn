#include "morn_util.h"

int main()
{
    MText *text= mTextCreate("dsedbcokjjhhdwabcrdfdacadebcddfg");
    MText *str = mTextCreate("abc");

    printf("%s\n",text->text);
    printf("%s\n",str->text);

    int n = mTextFind(text,str,0);
    printf("text->size=%d,n=%d\n",text->num,n);
    printf("%s\n",text->text+n);

    mTextRelease(text);
    mTextRelease(str);
}
