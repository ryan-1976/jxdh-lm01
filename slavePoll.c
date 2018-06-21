#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "modbus.h"
#include "public.h"
char dest[256];
void threadcreate(void);
void * mbPollThread(void *arg);

//static char judgeMbPollIsStd(const char * name);
static void pollThreadcreate(void);
static const int idx[7]={0,1,2,3,4,5,6};


extern int nativeUartTreat(void);
extern void mbPollUartTreat(INT8U idx);
void *slavePollThread(void)
{
	pollThreadcreate();
    sleep(1);
    return 0;
}
void * mbPollThread(void *arg)
{
    INT8U num=*(INT8U *)arg;


    	printf("num is %d\n",num);
    	if(num<7) mbPollUartTreat(num);
    	printf("num thread exit is %d\n",num);
    	pthread_exit((void *)0);
    	//printf("serialPath is %s\n",serialPath[num]);
    	//if(num==4) pthread_exit((void *)0);

    //return NULL;
}

void * nativeSlavePollThread(void *arg)
{
    int num=*(int *)arg;
	printf("num is %d\n",num);
	//printf("native slave poll serialPath is %s\n",serialPath[num]);
	nativeUartTreat();

	pthread_exit((void *)0);
}
static void pollThreadcreate(void)
{
	int i;
	char rc;
    pthread_t thr[7];
	if(pthread_create(&thr[6],NULL,nativeSlavePollThread,&idx[6])!=0)
	{
		printf("create thread 6 failed!\n");
		return;
	}
     for(i=0;i<6;i++)
     {
			if(pthread_create(&thr[i],NULL,mbPollThread,&idx[i])!=0)
			{
				printf("create thread %d failed!\n",i);
				return;
			}
//    	 rc=judgeMbPollIsStd(rs485[i]);
//    	 if(rc==1)
//    	 {
//    		 if(pthread_create(&thr[i],NULL,mbPollThread,&idx[i])!=0)
//			{
//				printf("create thread 0 failed!\n");
//				return;
//			}
//    	 }
//    	 else  if(rc==0)
//    	 {
//			if(pthread_create(&thr[i],NULL,nonStdMbThread,&idx[i])!=0)
//			{
//				printf("create thread 0 failed!\n");
//				return;
//			}
//    	 }
     }
}
//static char judgeMbPollIsStd(const char * name)
//{
//	int i;
//	for(i=0;i<g_devHardCfgLen;i++)
//	{
//		if(!strcmp(name,g_devHardCfg[i].name))
//		{
//			if(!strcmp("Y",g_devHardCfg[i].option5))return 1;
//			else return 0;
//		}
//	}
//	return -1;
//}

