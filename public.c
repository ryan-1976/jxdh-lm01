#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "public.h"


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
