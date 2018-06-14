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
#include "sqlite3.h"
#include "circlebuff.h"
#include "public.h"
#include "cJSON.h"
char *getJson(int idx);
extern MQTT_SENT_BUFF_T mqBuff;
int pAlmOidIdx[500];
int *pReadOidIdx;
INT32U g_reportTimeCnt=0;
char g_currentTime[50]={0};
static char  sMqNewpacketFlag=0;
static char tmp[50] ={0};
char str[1500]={0};
void  getTime(char *temp);
void formJsonPacket(int idx);
extern void mq_circleBuff_WritePacket(INT8U *s, INT16U len, INT16U port);
void data_classification(void)
{
	int m=0;
	//int n=0;
	int i;

	for(i=0;i<g_tabLen;i++)
	{
		if(g_devDataTab[i].dataOption == ALM_20)m++;
	}
	//pAlmOidIdx= (int *)malloc(m+1);
	m =0;
	for(i=0;i<g_tabLen;i++)
	{
		if(g_devDataTab[i].dataOption == ALM_20)
		{
			pAlmOidIdx[m++]=i;
		}
	}
	pAlmOidIdx[m]=0xffff;
}

void dataReport_treat(void)
{
	int i;

	getTime(&g_currentTime[0]);
	for(i=0;i<g_tabLen;i++)
	{
		if(g_devDataTab[i].upSentPeriod ==0)continue;
		if((g_reportTimeCnt%g_devDataTab[i].upSentPeriod)!=0)continue;
		if(g_devDataTab[i].belongToOid>10)continue;
	    formJsonPacket(i);
	}
}
void formJsonPacket(int idx)
{
	char *s = getJson(idx);
	//printf("%s\n",s);
	strcpy(str,s);
    free(s);
	pthread_mutex_lock(&mqBuff.lock);
	if(strlen(str)>0)
	{
		mq_circleBuff_WritePacket((INT8U *)str,(INT16U)strlen(str),MQTP_REPORT);
		mqBuff.packetSum++;
		sMqNewpacketFlag=1;
	}
	pthread_mutex_unlock(&mqBuff.lock);
}


void  getTime(char *temp)
{
	struct   tm     *ptm;
	long ts;
	int   y,m,d,h,n,s;
	char str[50]= {0};
	char strTemp[10]= {0};

	ts   =   time(NULL);
	ptm   =   localtime(&ts);
	y   =   ptm-> tm_year+1900;         //年
	m   =   ptm-> tm_mon+1;             //月
	d   =   ptm-> tm_mday;              //日
	h   =   ptm-> tm_hour;              //时
	n   =   ptm-> tm_min;               //分
	s   =   ptm-> tm_sec;               //秒
	sprintf(strTemp, "%d", y);
	strcat(str,strTemp);
	strcat(str,"-");
	if(m<10)strcat(str,"0");
	sprintf(strTemp, "%d", m);
	strcat(str,strTemp);
	strcat(str,"-");
	if(d<10)strcat(str,"0");
	sprintf(strTemp, "%d", d);
	strcat(str,strTemp);
	strcat(str," ");
	if(h<10)strcat(str,"0");
	sprintf(strTemp, "%d", h);
	strcat(str,strTemp);
	strcat(str,":");
	if(n<10)strcat(str,"0");
	sprintf(strTemp, "%d", n);
	strcat(str,strTemp);
	strcat(str,":");
	if(s<10)strcat(str,"0");
	sprintf(strTemp, "%d", s);
	strcat(str,strTemp);
	strcpy(temp,str);
}
void dataInt2String(void)
{
	double dValue;
    int i;
	for(i=0;i<g_tabLen;i++)
	{
		dValue =(double)g_devDataTab[i].valueInt/g_devDataTab[i].radio;
		gcvt(dValue,8,g_devDataTab[i].valueString);
	//	printf("g_devDataTab[i].valueString=%s\n",g_devDataTab[i].valueString);
	}
}
void data4Test(void)
{
	int i;
	for(i=0;i<g_tabLen;i++)
	{
		 if(g_devDataTab[i].dataOption !=ALM_20 && g_devDataTab[i].dataOption !=SET_1 )
			g_devDataTab[i].valueInt =i;
	}
}
char *getJson(int idx)
{
	int m;

		cJSON *jsonRoot=NULL;
		cJSON *js_data =NULL;
		char *p = NULL;
		jsonRoot=cJSON_CreateObject();
		if (jsonRoot == NULL)goto end;
		cJSON_AddStringToObject(jsonRoot,"version",g_mqComVer);
		cJSON_AddStringToObject(jsonRoot, "type",g_devDataTab[idx].ssType);
		strcat(tmp,g_mqComId);
		strcat(tmp,g_devDataTab[idx].ssDevId);
		cJSON_AddStringToObject(jsonRoot, "id",tmp);
		tmp[0]=0;
		cJSON_AddStringToObject(jsonRoot, "time",g_currentTime);
		cJSON_AddItemToObject(jsonRoot, "data", js_data=cJSON_CreateObject());
		if(g_devDataTab[idx].belongToOid !=1)
		{
			cJSON_AddStringToObject(js_data, g_devDataTab[idx].ssDataType,g_devDataTab[idx].valueString);
		}
		else
		{
			cJSON_AddStringToObject(js_data, g_devDataTab[idx].ssDataType,g_devDataTab[idx].valueString);
			for(m=0;m<g_tabLen;m++)
	 		{
				if(g_devDataTab[m].upSentPeriod ==0)continue;
				if(g_devDataTab[idx].oid ==g_devDataTab[m].belongToOid)
				{
					cJSON_AddStringToObject(js_data, g_devDataTab[m].ssDataType,g_devDataTab[m].valueString);
				}
			}
    	}
		p = cJSON_PrintUnformatted(jsonRoot);

	end:
		cJSON_Delete(jsonRoot);
        return p;
}

void *sampleDataThread(void)
{
	data_classification();
	//testCrc32();
	while(1)
	{
			//g_reportTimeCnt++;
		// printf("enter sample process\n");
		//DebugPrint("----test udp debug----enter sample process---------------\n");
		g_reportTimeCnt=g_reportTimeCnt+10;
		data4Test();
		sleep(3);
		dataInt2String();
		dataReport_treat();
		pthread_mutex_lock(&mqBuff.lock);
		if(sMqNewpacketFlag !=0)
		{
			pthread_cond_signal(&mqBuff.newPacketFlag);
			sMqNewpacketFlag =0;
		}
		pthread_mutex_unlock(&mqBuff.lock);
	}
}
