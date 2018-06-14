#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <stdarg.h>
char udpRecBuf [1024]={0};
char udpSentBuf [1024]={0};

#define DEBUG_STRING_MAX_LEN 1024
typedef struct
{
	unsigned int  ModuleId;
    unsigned int length;
    unsigned char   data[DEBUG_STRING_MAX_LEN];

}S_DebugMsg;

enum
{
    noDebug=0,
    serialDebug,
    udpDebug,
}debugWhere;

char g_debugWhere=serialDebug;
struct sockaddr_in g_localAddr;
struct sockaddr_in g_remoteAddr;
struct sockaddr_in g_debugAddr;
int g_sockfd;
void DebugPrint(char *fmt, ...);

void *udpServerThread(void)
{
	 g_sockfd=socket(AF_INET,SOCK_DGRAM,0);

	    //创建网络通信对象
//	    struct sockaddr_in addr;
	 g_localAddr.sin_family =AF_INET;
	 g_localAddr.sin_port =htons(5211);
	    //addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	 g_localAddr.sin_addr.s_addr=INADDR_ANY;
	    //INADDR_ANY

	    //绑定socket对象与通信链接
	 int ret =-1;
	 while(ret ==-1)
	 {
		ret =bind(g_sockfd,(struct sockaddr*)&g_localAddr,sizeof(g_localAddr));
		if(0>ret)
		{
			printf("bind erro\n");
			sleep(3);
		}
	 }
	    printf("bind ok\n");
//	    struct sockaddr_in cli;
	    socklen_t len=sizeof(g_remoteAddr);

	    while(1)
	    {
	        printf("udp wait for recv\n");

	        recvfrom(g_sockfd,&udpRecBuf,sizeof(udpRecBuf),0,(struct sockaddr*)&g_remoteAddr,&len);
	        printf("recv num =%s\n",udpRecBuf);
	        if(!strcmp(udpRecBuf,"debug"))
	        {
	        	g_debugWhere=udpDebug;
	        	g_debugAddr =g_remoteAddr;

	        }
//	        for(int i=0;i<1024;i++)udpSentBuf[i]=i;
//	        sendto(g_sockfd,&udpSentBuf,sizeof(udpSentBuf),0,(struct sockaddr*)&g_remoteAddr,sizeof(g_remoteAddr));

	    }
	    close(g_sockfd);
}

void DebugPrint( char *fmt, ...)
{
    S_DebugMsg debugMsg;
    int printStrLen =0;

    if(noDebug == g_debugWhere)return;
    va_list ap;
    va_start(ap, fmt);
    printStrLen = vsnprintf((char*)&debugMsg.data[0], DEBUG_STRING_MAX_LEN, fmt, ap);
    va_end(ap);


    if(g_debugWhere==serialDebug)
    {
        printf("%s", debugMsg.data);
//        if(FILE_OUTPUT_DEBUG == (level & 0xFFFF0000))
//        {
//            dbgFileOut.open(DEBUG_FILE_OUTPUT_PATH);
//            dbgFileOut << debugMsg.data;
//            dbgFileOut.close();
//        }
        return;
    }
    if(g_debugWhere==udpDebug)
    {
		debugMsg.length = htons(printStrLen);
		sendto(g_sockfd, &debugMsg.data, printStrLen, 0,(struct sockaddr *)&g_debugAddr, sizeof(struct sockaddr_in));
    }
}
