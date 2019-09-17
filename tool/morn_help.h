
#define HELP_FILE "./help.txt"

#define fgets(Buff,Num,F) mException((fgets(Buff,Num,F)==NULL),EXIT,"file read error")

#define HELP_INFOMATION(name) {\
    int arg_flag;\
    mStringArgument(argc,argv,"-help",&arg_flag);\
    if(arg_flag) {mToolHelp(name); return 1;}\
    mStringArgument(argc,argv,"v",&arg_flag);\
    if(arg_flag) {mToolHelp("version"); return 1;}\
}

void mToolHelp(char *name)
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
            if((strspn(buff+1,name)>=len)&&(buff[len+1] == ']'))
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
