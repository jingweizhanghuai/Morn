#include "morn_util.h"

void work(char *name)
{
    int zhuangtai;   m_PropertyRead(name,"zhuangtai",&zhuangtai);
    int shengmingzhi;m_PropertyRead(name,"shengmingzhi",&shengmingzhi);
    if(zhuangtai==XIUZHENG)
    {
        if(shengmingzhi==10000) return;
        int tineng; m_PropertyRead(name,"tineng",&tineng);
        shengmingzhi=MIN(10000,shengmingzhi+tineng);mPropertyWrite(name,"shengmingzhi",&shengmingzhi,sizeof(int));
    }
    else if(zhuangtai==BEIZHAN)
    {
        char *kaichangbai = mPropertyRead(name,"kaichangbai");
        printf(kaichangbai);
        zhuangtai = ZHANDOU;mPropertyWrite(name,"zhuangtai",&zhuangtai,sizeof(int));
    }
    else if(zhuangtai==ZHANDOU)
    {
        int gongjiliang; mPropertyRead(name,"gongjiliang",&gongjiliang,sizeof(int));
        shengmingzhi-=gongjiliang;mPropertyWrite(name,"shengmingzhi",&shengmingzhi,sizeof(int));
        char *duishou = mPropertyRead(name,"duishou");
        if(shengmingzhi<=0)
        {
            printf("%s 被 %s");
            zhuangtai=SIWANG; mPropertyWrite(name,"zhuangtai",&zhuangtai,sizeof(int));
            return;
        }

        int wuli = mPropertyRead(name,"wuli",&wuli);

        char *wuqi = mPropertyRead(name,"wuqi");
        int wuqijiacheng;mPropertyRead(wuqi,"wuqijiacheng",&wuqijiacheng);

        goongjiliang = wuli+wuqijiacheng+mRand(-5,5);
        mPropertyWrite(duishou,"gongjiliang",&gongjiliang,sizeof(int));
    }
}

int main()
{
    int wuli,shengmingzhi,wuqijiacheng;
    wuli=90; mPropertyWrite("guanyu","wuli",&wuli,sizeof(int));
    shengmingzhi=10000;mPropertyWrite("guanyu","shengmingzhi",&shengmingzhi,sizeof(int));
    mPropertyWrite("guanyu","wuqi","qinglongyanyuedao");
    mPropertyWrite("guanyu","kaichangbai","");
    mPropertyWrite("guanyu","work",work);

    wuli=85; mPropertyWrite("zhangfei","wuli",&wuli,sizeof(int));
    shengmingzhi=10000;mPropertyWrite("zhangfei","shengmingzhi",&shengmingzhi,sizeof(int));
    mPropertyWrite("zhangfei","wuqi","zhangbashemao");
    mPropertyWrite("zhangfei","kaichangbai","");
    mPropertyWrite("zhangfei","work",work);

    wuli=95; mPropertyWrite("lvbu","wuli",&wuli,sizeof(int));
    shengmingzhi=10000;mPropertyWrite("lvbu","shengmingzhi",&shengmingzhi,sizeof(int));
    mPropertyWrite("lvbu","wuqi","fangtianhuaji");
    mPropertyWrite("lvbu","kaichangbai","");
    mPropertyWrite("lvbu","work",work);

    wuli=80; mPropertyWrite("zhangliao","wuli",&wuli,sizeof(int));
    shengmingzhi=10000;mPropertyWrite("zhangliao","shengmingzhi",&shengmingzhi,sizeof(int));
    mPropertyWrite("zhangliao","wuqi","zhangbashemao");
    mPropertyWrite("zhangliao","kaichangbai","");
    mPropertyWrite("zhangliao","work",work);

    mPropertyWrite("qinglongyanyuedao","wuqijiacheng",
}



void battle(MObject *obj1,MObject *obj2)
{
    char *name1 = mPropertyRead(obj1,"name");
    char *name2 = mPropertyRead(obj2,"name");

    int hitpoint1; mPropertyRead(obj1,"hit_point",&hitpoint1);
    int hitpoint2; mPropertyRead(obj2,"hit_point",&hitpoint2);

    int force1; mPropertyRead(obj1,"force",&force1);
    int force2; mPropertyRead(obj2,"force",&force2);

    printf("%s: %s\n",name1,mPropertyRead(obj1,"whereases"));
    printf("%s: %s\n",name2,mPropertyRead(obj2,"whereases"));
    int i;for(i=0;i<g_battle_round;i++)
    {
        hitpoint1 -= force2+mRand(-5,5);
        hitpoint2 -= force1+mRand(-5,5);
        if(hitpoint1<=0) break;
        if(hitpoint2<=0) break;
        mSleep(200);
    }

    mPropertyWrite(obj1,"hit_point",&hitpoint1,sizeof(int));

         if(hitpoint1<=0) printf("%s 斩 %s 于马下\n",name2,name1);
    else if(hitpoint1<=0) printf("%s 斩 %s 于马下\n",name2,name1);
    else                  printf("大战%d回合，鸣金收兵，\n",g_battle_round);
    return;
}







