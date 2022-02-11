// g++ -O2 -DNDEBUG test_text2.cpp -o test_text2.exe -lmorn

#include "morn_util.h"
#include <string>

void test(const char *find)
{
    printf("\n%s\n",find);
    
    int num;
    MArray *text= mTextCreate();
    mFileText(text,"D:/Morn/test/test/a_Q_zheng_zhuan.txt");

    num=0;
    mTimerBegin("strstr");
    for(int i=0;i<10000;i++)
    {
        char *p_text = text->text;
        while(1)
        {
            p_text=strstr(p_text,find);
            if(p_text==NULL) break;
            num++;
            p_text=p_text+1;
        }
    }
    mTimerEnd("strstr");
    printf("num=%d\n",num);

    num=0;
    std::string string(text->text,text->num);
    mTimerBegin("stl");
    for(int i=0;i<10000;i++)
    {
        int pos=0;
        while(1)
        {
            pos=string.find(find,pos+1);
            if (pos == std::string::npos) break;
            num++;
        }
    }
    mTimerEnd("stl");
    printf("num=%d\n",num);

    num=0;
    MArray *str = mTextCreate(find);
    mTimerBegin("Morn");
    for(int i=0;i<10000;i++)
    {
        int pos=0;
        while(1)
        {
            pos = mTextFind(text,str,pos+1);
            if(pos<0) break;
            num++;
        }
    }
    mTimerEnd("Morn");
    printf("num=%d\n",num);

    mTextRelease(text);
    mTextRelease(str);
}

int main()
{
    test("阿Q");
    test("你那里配姓赵!");
    test("阿Q站了一刻，心里想，“我总算被儿子打了，现在的世界真不像样……”于是也心满意足的得胜的走了。");
    test("阿Q要画圆圈了，那手捏着笔却只是抖。于是那人替他将纸铺在地上，阿Q伏下去，使尽了平生的力画圆圈。他生怕被人笑话，立志要画得圆，但这可恶的笔不但很沉重，并且不听话，刚刚一抖一抖的几乎要合缝，却又向外一耸，画成瓜子模样了。");
    return 0;
}



