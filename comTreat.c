#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "public.h"
#include "cJSON.h"

DOWNLOAD g_dlState;
unsigned char *g_fileTmp=NULL;
unsigned char *g_filePkOk=NULL;
FILE *dlfp;

int parseJson (unsigned char *strJson )
{
  cJSON * pRoot,*pSubData ;
//  int iCount=0;
  int i;
  pRoot =NULL;
  if(NULL == strJson)goto end;
  pRoot = cJSON_Parse((const char *)strJson);  /* 解析 json 放入 pJson*/
  if(NULL == pRoot)goto end;
  cJSON*item=cJSON_GetObjectItem(pRoot,"type");
  pSubData =cJSON_GetObjectItem(pRoot, "data");

  if(NULL==item)goto end;
  printf("传入type=%s\n",item->valuestring);
	if(0==strcmp(item->valuestring,"filestart")&&g_dlState.curState==0)
	{
		    cJSON*item=cJSON_GetObjectItem(pSubData,"filesize");
			g_dlState.fileLen =item->valueint;
			item=cJSON_GetObjectItem(pSubData,"filecrc");
			g_dlState.pkCrc =item->valueint;
			item=cJSON_GetObjectItem(pSubData,"blockcount");
			g_dlState.pkSum =item->valueint;
			item=cJSON_GetObjectItem(pSubData,"maxblocklen");
			g_dlState.pkMaxLen =item->valueint;
			g_dlState.curPkIndx =0;
			g_dlState.curState =1;
			item=cJSON_GetObjectItem(pSubData,"fileName");
			strcpy(g_dlState.dlName,item->valuestring);
			g_fileTmp = (unsigned char *)malloc(g_dlState.fileLen+1);
			g_filePkOk = (unsigned char *)malloc(g_dlState.pkSum+1);
			if (!g_fileTmp ||!g_filePkOk)
			{
				printf("malloc download ram   error\n");
				exit(EXIT_FAILURE);
			}

//			memset(g_fileTmp,0,sizeof(g_fileTmp));
//			memset(g_filePkOk,0,sizeof(g_filePkOk));
			memset(g_fileTmp,0,g_dlState.fileLen+1);
			memset(g_filePkOk,0,g_dlState.pkSum+1);
			printf("g_dlState.fileLen=%d\n",g_dlState.fileLen);
			printf("g_dlState.pkCrc=%d\n",g_dlState.pkCrc);
			printf("g_dlState.pkSum=%d\n",g_dlState.pkSum);
			printf("g_dlState.pkMaxLen=%d\n",g_dlState.pkMaxLen);
			printf("g_dlState.dlName=%s\n",g_dlState.dlName);

	}
	else if(0==strcmp(item->valuestring,"fileData")&&g_dlState.curState==1)
	{
		item=cJSON_GetObjectItem(pSubData,"seqnum");
		g_dlState.curPkIndx =item->valueint;
		g_filePkOk[g_dlState.curPkIndx]=1;

		int temp_idx =g_dlState.curPkIndx*g_dlState.pkMaxLen;
		item=cJSON_GetObjectItem(pSubData,"filedata");
		//printf("rec base64=%s\n",item->valuestring);
		base64_decode(item->valuestring,&g_fileTmp[temp_idx]);
		//printf("rec trans=%s\n",&g_fileTmp[temp_idx]);
		if(g_dlState.curPkIndx == g_dlState.pkSum-1)
		{
			printf("g_filePk=");
			int temp=0;
			for(i=0;i<g_dlState.pkSum;i++)
			{
				if(g_filePkOk[i]!=1)temp=1;
				printf("%d",g_filePkOk[i]);
			}
			printf("\n");
			//printf("%s\n",g_fileTmp);
           if(!temp)
           {
        	   g_dlState.curState =2;
        	   int crc32=caculate_crc(g_fileTmp,g_dlState.fileLen);
        	   printf("crc32=%d;g_dlState.pkCrc=%d\n",crc32,g_dlState.pkCrc);

        	   dlfp=fopen(g_dlState.dlName,"w+");
        	   fwrite(g_fileTmp,1,g_dlState.fileLen,dlfp);
        	   fclose(dlfp);
        	   free(g_fileTmp);
        	   free(g_filePkOk);
        	   sleep(2);
        	   exit(0);
        	   printf("---download ok---------------------\n");
           }
		}
	}
	else
	{

	}



  end:
  cJSON_Delete(pRoot);
  return 0;
}


void com_treat(void)
{
  //  printf("RecvBuff4treat.scrFlag=%d\n",RecvBuff4treat.scrFlag);
	switch(RecvBuff4treat.scrFlag)
	{
		case MQTPC2DTU:
			parseJson(RecvBuff4treat.data);
			break;
		default:
			break;
	}
}
void *comThread(void)
{

	 while (1)
	 {
		 pthread_mutex_lock(&RecvBuff4treat.lock);
		 pthread_cond_wait(&RecvBuff4treat.newPacketFlag, &RecvBuff4treat.lock);
		 printf("-----enter--com_Treat\n");
		 com_treat();
		 pthread_mutex_unlock(&RecvBuff4treat.lock);

	 }
}
