// gcc -O2 test_CRC.c -o test_CRC.exe -lmorn
#include "morn_util.h"

int main()
{
    char *data = "hello world";int size=strlen(data);
    char *type;
    
    type="CRC8";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data,size));
    
    type="ITU";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="ROHC";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="MAXIM-8";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="X25";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="CCITT";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="CCITT-FALSE";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="XMODEM";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="IBM";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="MAXIM-16";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="USB";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="MODEBUS";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="DNP";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="CRC32";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    type="MPEG2";mPropertyWrite("CRC","type",type);
    printf("type=%s,crc=0x%x\n",type,mCRC(data));
    
    return 0;
}
