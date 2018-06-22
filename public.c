#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "public.h"

/* Table of CRC values for high-order byte */
static const INT8U table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const INT8U table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

int  get_oidValue(int oid)
{
	int i;
	for(i=0;i<g_tabLen;i++)
	{
		if(g_devDataTab[i].oid == oid)
			return g_devDataTab[i].valueInt;
	}
    return -1;

}
int  get_oidIdx(int oid)
{
	int i;
	for(i=0;i<g_tabLen;i++)
	{
		if(g_devDataTab[i].oid == oid)
			return i;
	}
    return -1;

}
int  get_typeValue(char *ssType)
{
	int i;
	for(i=0;i<g_tabLen;i++)
	{
		if(g_devDataTab[i].ssType == ssType)
			return g_devDataTab[i].valueInt;
	}
    return -1;

}


  
INT32U crc32_table[256];
  
void make_crc32_table(void)
{  
	INT32U c;
	INT32U i = 0;
    int bit = 0;  
      
    for(i = 0; i < 256; i++)  
    {  
        c  = (INT32U)i;
          
        for(bit = 0; bit < 8; bit++)  
        {  
            if(c&1)  
            {  
                c = (c >> 1)^(0xEDB88320);  
            }  
            else  
            {  
                c =  c >> 1;  
            }  
              
        }  
        crc32_table[i] = c;  
    }  
}  


  
INT32U make_crc(INT32U crc, unsigned char *string, INT32U size)
{  

    while(size--)  
        crc = (crc >> 8)^(crc32_table[(crc ^ *string++)&0xff]);  
  
    return crc;  
}  
INT32U caculate_crc(unsigned char *string, INT32U size)
{
	  INT32U crc = 0xFFFFFFFF;
	  make_crc32_table();
	  return make_crc(crc,string,size);
 }

void testCrc32(void)
{
//    int i;
    FILE *sp = NULL;
//    FILE *dp = NULL;
    INT32U count;
    INT32U crc = 0xFFFFFFFF;
   // INT32U crc = 0;
    unsigned char buf[1024];

    printf("--------enter crc32------------------------------------\n");
    make_crc32_table();

    sp = fopen("/home/ryan/share2win/jxdh01-v02.db", "rb");
    if(sp == NULL)
    {
        printf("open file error\n");
        return;
    }

    if(sp)
    {
        while(!feof(sp))
        {
            memset(buf, 0, sizeof(buf));
            count = fread(buf, 1, sizeof(buf), sp);
            crc = make_crc(crc, buf, count);
        }
    }

    printf("calculate crc is 0x%x\n", crc);
   // printf("calculate crc is %d\n", crc);
    fclose(sp);


}

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode( const char * base64, unsigned char * bindata )
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}
void HexToStr(INT8U *pbDest, INT8U *pbSrc, INT16U nLen)
{
	INT8U    ddl,ddh;
	INT16U i;

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}
void StrToHex(INT8U *pbDest, INT8U *pbSrc, INT16U nLen)
{
	INT8U h1,h2;
	INT8U s1,s2;
	INT16U i;

    for (i=0; i<nLen; i++)
    {
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];
		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
		s1 -= 7;
		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
		s2 -= 7;
		pbDest[i] = s1*16 + s2;
    }
}
void rmZfFun(INT8U *p,INT8U ch)
{
	INT8U  i,j;

	for(i=0;p[i]!='\0';i++)
	{
		if(p[i]==ch)
		{
			for(j=i;p[j]!='\0';j++) p[j]=p[j+1];
		}
		if(i>250)break;
	}
}

INT16U modbusCrc16(INT8U *buffer, INT16U buffer_length)
{
	INT8U crc_hi = 0xFF; /* high CRC byte initialized */
	INT8U crc_lo = 0xFF; /* low CRC byte initialized */
	INT16U i; /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}
//#define BUFSIZE     1024*4
//
//static unsigned int crc_table[256];
//static void init_crc_table(void)
//{
//    unsigned int c;
//    unsigned int i, j;
//
//    for (i = 0; i < 256; i++) {
//        c = (unsigned int)i;
//        for (j = 0; j < 8; j++) {
//            if (c & 1)
//                c = 0xedb88320L ^ (c >> 1);
//            else
//                c = c >> 1;
//        }
//        crc_table[i] = c;
//    }
//}
//static unsigned int crc32(unsigned int crc,unsigned char *buffer, unsigned int size)
//{
//    unsigned int i;
//    for (i = 0; i < size; i++) {
//        crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
//    }
//    return crc ;
//}
// void calc_img_crc(void)
//{
//    int fd;
//    int nread;
//    int ret;
//    unsigned char buf[BUFSIZE];
//    /*第一次传入的值需要固定,如果发送端使用该值计算crc校验码,
//    **那么接收端也同样需要使用该值进行计算*/
//    unsigned int crc = 0xffffffff;
//    init_crc_table();
//    //fopen("/home/ryan/share2win/jxdh01-v02.db", "rb");
//    fd = open("/home/ryan/share2win/jxdh01-v02.db", O_RDONLY);
//    if (fd < 0) {
//        printf("%d:open %s.\n", __LINE__, strerror(errno));
//        return ;
//    }
//
//    while ((nread = read(fd, buf, BUFSIZE)) > 0) {
//        crc = crc32(crc, buf, nread);
//    }
//    //printf(" new  calculate crc is 0x%x\n", crc);
//    printf(" new  calculate crc is %d\n", crc);
//    close(fd);
//
//    if (nread < 0) {
//        printf("%d:read %s.\n", __LINE__, strerror(errno));
//        return ;
//    }
//
//    return ;
//}
//void compare_crc(char *filename)
//{
//    FILE *sp = NULL;
//    uint32_t srcCrc ;
//    uint32_t calcuCrc = 0xffffffff;
//    unsigned char buf[1024];
//    uint32_t count;
//
//    if(filename == NULL)
//    {
//        printf("filename is null\n");
//        exit(1);
//    }
//
//    sp = fopen(filename, "rb");
//    if(sp == NULL)
//    {
//        printf("open file fail\n");
//        exit(1);
//    }
//
//    fread(&srcCrc, 1, 4, sp);
//    printf("In %s: src crc is 0x%x\n", __FUNCTION__, srcCrc);
//
//
//        if(sp)
//        {
//                while(!feof(sp))
//                {
//                        memset(buf, 0, sizeof(buf));
//                        count = fread(buf, 1, sizeof(buf), sp);
//                        calcuCrc = make_crc(calcuCrc, buf, count);
//                }
//        }
//    printf("In %s: calcuCrc is 0x%x\n", __FUNCTION__, calcuCrc);
//
//    fclose(sp);
//
//    if(srcCrc == calcuCrc)
//    {
//        printf("In %s: the calculate crc equal the src crc in file \n", __FUNCTION__);
//
//    }
//    else
//    {
//        printf("In %s: the calculate crc not equal the src crc in file \n", __FUNCTION__);
//    }
//
//}
  
  
//int main()
//{
//    int i;
//    FILE *sp = NULL;
//    FILE *dp = NULL;
//    uint32_t count;
//    uint32_t crc = 0xFFFFFFFF;
//    unsigned char buf[1024];
//
//
//    make_crc32_table();
//
//    sp = fopen("/home/user/work_soft/crc_check/bak/test.txt", "rb");
//    if(sp == NULL)
//    {
//        printf("open file error\n");
//        return -1;
//    }
//    dp = fopen("/home/user/work_soft/crc_check/bak/testcrc.txt", "wb");
//    if(dp == NULL)
//    {
//        printf("open file error\n");
//        return -1;
//    }
//
//    if(sp)
//    {
//        while(!feof(sp))
//        {
//            memset(buf, 0, sizeof(buf));
//            count = fread(buf, 1, sizeof(buf), sp);
//            crc = make_crc(crc, buf, count);
//        }
//    }
//
//    printf("In main: calculate crc is 0x%x\n", crc);
//    if(dp)
//    {
//        fwrite(&crc, 1, 4, dp);//write the crc into the file testcrc.txt
//        fseek(sp, 0, SEEK_SET);
//        while(!feof(sp))
//        {
//            memset(buf, 0, sizeof(buf));
//                        count = fread(buf, 1, sizeof(buf), sp);
//            fwrite(buf, 1, count, dp);
//        }
//
//        fclose(sp);
//        fclose(dp);
//    }
//
//
//    compare_crc("/home/user/work_soft/crc_check/bak/testcrc.txt");
//
//
//    return 0;
//
//}
