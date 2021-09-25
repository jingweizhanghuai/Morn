#include "morn_util.h"

uint32_t mCRC32(void *in,int size)
{
    uint32_t crc = 0xEDB88320;
    uint8_t *p = (uint8_t *)in;
    uint32_t rst=p[0];//*((uint32_t *)(in));
    // printf("rst=%x\n",rst);

    for(int i=1;i<size+4;i++)
    {
        printf("aa rst=%x\n",rst);
        uint8_t data = (i<size)?p[i]:0;
        for(int j=0;j<8;j++)
        {
            printf("rst=%x\n",rst);
            if(rst>0x7FFFFFFF) rst=rst^crc;
            uint32_t d=((data&(0x80>>j))!=0);
            printf("b rst=%x,d=%d\n",rst,d);
            
            rst=(rst<<1)+d;
        }
    }
    return rst;
}

unsigned char test[16] ={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
unsigned char len = 16;
void CRCCaculate()
{
   unsigned int crc = 0;
   unsigned char i;
   unsigned char *ptr = test;
   while( len-- ) {
      for(i = 0x80; i != 0; i = i >> 1) {
        if((crc & 0x8000) != 0) {
           crc = crc << 1;
           crc = crc ^ 0x1021;
        }
        else {
           crc = crc << 1;
        }
        if((*ptr & i) != 0) {
          crc = crc ^ 0x1021;
        }
     }
     ptr++;
   }

   printf("0x%x ",crc);
}


uint32_t crc_table[256];

unsigned int CRC(unsigned char *ptr,int size)
{
    int bits=32;
    unsigned int crc = 0;
    for(int i=0;i<size;i++)
    {
        unsigned char buff = (unsigned char)(crc >>(bits-8));
        crc = crc << 8;
        crc = crc ^ crc_table[ buff ^ ptr[i]];
    }
    return crc;
}

// #define POLY        0x1021  
// uint16_t crc16(unsigned char *addr, int num, uint16_t crc)  
// {  
//     int i;  
//     for (; num > 0; num--)              /* Step through bytes in memory */  
//     {  
//         crc = crc ^ (*addr++ << 8);     /* Fetch byte from memory, XOR into CRC top byte*/  
//         for (i = 0; i < 8; i++)             /* Prepare to rotate 8 bits */  
//         {  
//             if (crc & 0x8000)            /* b15 is set... */  
//                 crc = (crc << 1) ^ POLY;    /* rotate and XOR with polynomic */  
//             else                          /* b15 is clear... */  
//                 crc <<= 1;                  /* just rotate */  
//         }                             /* Loop for 8 bits */  
//         crc &= 0xFFFF;                  /* Ensure CRC remains 16-bit value */  
//     }                               /* Loop until num=0 */  
//     return(crc);                    /* Return updated CRC */  
// }  

/*


110101001111
 1001
 0011100
    1001
    010111
      1001
      11101
       1001
       01001
        


11010000
 0011
 10010
  0011
  000100

111111110000
 0011
 11001
  0011
  10101
   0011
   011010
     0011
     10010
      0011
        0100

111100000000
 0011
 11010
  0011
  10010
   0011
    0010
  
 


00110000
   0011
   10110
    0011
    0101 

   
   0101
   1010


00110011
   0011
   10101
    0011
    011000
      0011
      10110
       0011
       01010

110011000000
 0011
 10101
  0011
   11000
    0011
    10110
     0011
      10100
       0011
        1110


101100111111
 1001
 11110
  1001
  011111
    1001
    011011
      1001
      001011
        

1010000111


11111101100110000
 1001
 011010
   1001
   0011110
      1001
      011101
        1001
        010010
          1001
          10110
           1001
           11110
            1001
            01110

0000
1001
1001

1111101100110000
 1001
 011001
   1001
   000010011
        1001
        10100
         1001
         11010
          1001
          001100
                

111100110000
 1001
 011101
   1001
   01001
    1001
    00000


1111001100000000
 1001
 011101
   1001
   010010
     1001
     10110
      1001
      11110
       1001
       011100
         1001
         010100
           1001
           11010
            1001
            0011
       
                    
1111000000010000
 1001
 011100
   1001
   010100
     1001
     11010
      1001
      0011010
         1001
         0011000
            1001
            0001
                    

1111
00010001

111000010000
 1001
 010100
   1001
   11011
    1001
    0010000
       1001
       10010
        1001
        1011
                            
                            
110100100000
 0011
 10010
  0011
  00011000
      0011
      10110
       0011
       01010
       
                            
*/
