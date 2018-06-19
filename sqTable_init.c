#include <stdio.h>
#include "sqlite3.h"
#include <stdlib.h>
#include "public.h"
#include <string.h>

devDataTable *g_devDataTab;
devHardCfg *g_devHardCfg;
MBPOLLCFG *g_mbPollTab;
char *g_mqComVer =NULL;
char g_mqComId[50]={0};
char g_mqTopicReport[50]={0};
char g_mqTopicCtrl[50]={0};
char g_mqServer[30]={0};
char g_mqClientId[50]={0};

int  g_tabLen=0;
int  g_devHardCfgLen=0;
int  g_mbPollTabLen=0;
const char *dbPathName="./db-dhlm01.db";
static void getHardCfg(void);
static void getMbPollCfg(void);
void sqTable_init(void)
{
	sqlite3 *pdb = NULL;

	char** pResult;
	int nRow,nCol,nResult;
	char* errmsg;
	int i;
	//nResult = sqlite3_open_v2("jxdh01-v01.db",&pdb,SQLITE_OPEN_READWRITE,NULL);
	//nResult = sqlite3_open_v2("/home/ryan/share2win/jxdh01-v02.db",&pdb,SQLITE_OPEN_READWRITE,NULL);
	nResult = sqlite3_open_v2(dbPathName,&pdb,SQLITE_OPEN_READWRITE,NULL);

    if(nResult)
    {
            fprintf(stderr,"can't open database:%s\n",sqlite3_errmsg(pdb));
            sqlite3_close(pdb);
            //printf("open devCfg table err\n");
            exit(EXIT_FAILURE);
    }
    printf("open db ok\n");
 //---------------------------------------------------------------------
    char * strSql = "select * from devData";
    nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK)
	{
		sqlite3_close(pdb);
		sqlite3_free(errmsg);
		return ;
	}


  int nIndex = nCol;

  g_tabLen = nRow;

  printf("sqlite3_get_table ok g_tabLen=%d\n",g_tabLen);
	if ((g_devDataTab = (devDataTable *)malloc((g_tabLen) * sizeof(devDataTable))) == NULL)
	{

		printf("malloc g_devDataTab error\n");
		exit(EXIT_FAILURE);
	}

	for(i=0;i<g_tabLen;i++)
	{
		g_devDataTab[i].oid =atoi(pResult[nIndex]);
		strcpy(g_devDataTab[i].oidName,pResult[nIndex+1]);
		strcpy(g_devDataTab[i].ssType,pResult[nIndex+2]);
		strcpy(g_devDataTab[i].ssTypeCnName,pResult[nIndex+3]);
		strcpy(g_devDataTab[i].ssDataType,pResult[nIndex+4]);
		strcpy(g_devDataTab[i].ssDevId,pResult[nIndex+5]);

		g_devDataTab[i].valueInt = atoi(pResult[nIndex+6]);
		g_devDataTab[i].valueIntPre =g_devDataTab[i].valueInt;
		g_devDataTab[i].upSentPeriod =atoi( pResult[nIndex+7]);
		g_devDataTab[i].dataOption =atoi( pResult[nIndex+8]);

		g_devDataTab[i].belongToOid = atoi(pResult[nIndex+9]);
		g_devDataTab[i].belongOidIdx = atoi(pResult[nIndex+10]);
		g_devDataTab[i].radio = atoi(pResult[nIndex+11]);
		nIndex=nIndex+nCol;
	}
 // sqlite3_free_table(pResult);
  //----------------select devInfo-----------------------------------------------------
//  strSql[0] =0;
	strSql = "select content from devInfo WHERE cfgName = 'mqComVer'";
	nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK) { goto end; }
	g_mqComVer =(char *) pResult[nCol];
	printf("g_mqComVer =%s\n",g_mqComVer);

	strSql = "select content from devInfo WHERE cfgName = 'coLtdId'";
	nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK) { goto end; }
	char * g_coLtdId =(char *) pResult[nCol];
	printf("g_coLtdIdr =%s\n",g_coLtdId);

	strSql = "select content from devInfo WHERE cfgName = 'transRoomId'";
	nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK) { goto end; }
	char *g_transRoomId =(char *) pResult[nCol];
	printf("g_transRoomId =%s\n",g_transRoomId);

	strSql = "select content from devInfo WHERE cfgName = 'mqServer'";
	nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK) { goto end; }
	strcat(g_mqServer,pResult[nCol]);
	//g_mqServer = pResult[nCol];
	printf("g_mqServer =%s\n",g_mqServer);

	strSql = "select content from devInfo WHERE cfgName = 'clentId'";
	nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK) { goto end; }
	strcat(g_mqClientId,pResult[nCol]);
//	g_mqClientId =pResult[nCol];
	printf("g_mqClientId =%s\n",g_mqClientId);

	//g_mqComId =g_coLtdId;
	strcat(g_mqComId,g_coLtdId);
	strcat(g_mqComId,".");
	strcat(g_mqComId,g_transRoomId);
	strcat(g_mqComId,".");

	strcat(g_mqTopicReport,g_coLtdId);
	strcat(g_mqTopicReport,"/");
	strcat(g_mqTopicReport,g_transRoomId);
	strcat(g_mqTopicReport,"/report");

	strcat(g_mqTopicCtrl,g_coLtdId);
	strcat(g_mqTopicCtrl,"/");
	strcat(g_mqTopicCtrl,g_transRoomId);
	strcat(g_mqTopicCtrl,"/control");

	printf("g_mqComId =%s\n",g_mqComId);
	printf("g_mqTopicReport =%s\n",g_mqTopicReport);
	printf("g_mqTopicCtrl =%s\n",g_mqTopicCtrl);
  //---------------------------------------------------------------------




end:
  sqlite3_close(pdb);
  sqlite3_free(errmsg);
  getHardCfg();
  getMbPollCfg();

}

static void getHardCfg(void)
{
	char** pResult;
	int nRow,nCol,nResult;
	char* errmsg;
	int i;

	sqlite3 *pdb = NULL;
	nResult = sqlite3_open_v2(dbPathName,&pdb,SQLITE_OPEN_READWRITE,NULL);

    char * strSql = "select * from devHardCfg";
    nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK)
	{
		sqlite3_close(pdb);
		sqlite3_free(errmsg);
		return ;
	}

	int nIndex = nCol;
	g_devHardCfgLen = nRow;
	//---------------------------------------------------

	printf("g_devHardCfgLen=%d\n",g_devHardCfgLen);
	if ((g_devHardCfg = (devHardCfg *)malloc((g_devHardCfgLen) * sizeof(devHardCfg))) == NULL)
	{
		printf("malloc g_devHardCfg error\n");
		exit(EXIT_FAILURE);
	}

	for(i=0;i<g_devHardCfgLen;i++)
	{
		strcpy(g_devHardCfg[i].name,pResult[nIndex]);
		strcpy(g_devHardCfg[i].option1,pResult[nIndex+1]);
		strcpy(g_devHardCfg[i].option2,pResult[nIndex+2]);
		strcpy(g_devHardCfg[i].option3,pResult[nIndex+3]);
		strcpy(g_devHardCfg[i].option4,pResult[nIndex+4]);
		strcpy(g_devHardCfg[i].option5,pResult[nIndex+5]);

		nIndex=nIndex+nCol;
	}
//	for(i=0;i<g_devHardCfgLen;i++)
//	{
//		printf("g_devHardCfg.name=%s ",g_devHardCfg[i].name);
//		printf("g_devHardCfg.option1=%s ",g_devHardCfg[i].option1);
//		printf("g_devHardCfg.option2=%s ",g_devHardCfg[i].option2);
//		printf("g_devHardCfg.option3=%s ",g_devHardCfg[i].option3);
//		printf("g_devHardCfg.option4=%s ",g_devHardCfg[i].option4);
//		printf("g_devHardCfg.option5=%s ",g_devHardCfg[i].option5);
//		printf("\n ");
//	}
	  //---------------------------------------------------

	  sqlite3_close(pdb);
	  sqlite3_free(errmsg);

}

static void getMbPollCfg(void)
{
	char** pResult;
	int nRow,nCol,nResult;
	char* errmsg;
	int i;

	sqlite3 *pdb = NULL;
	nResult = sqlite3_open_v2(dbPathName,&pdb,SQLITE_OPEN_READWRITE,NULL);

    char * strSql = "select * from mbPollCfgTab";
    nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK)
	{
		sqlite3_close(pdb);
		sqlite3_free(errmsg);
		return ;
	}

	int nIndex = nCol;
	g_mbPollTabLen = nRow;
	//---------------------------------------------------

	printf("g_mbPollTabLen=%d\n",g_mbPollTabLen);
	if ((g_mbPollTab = (MBPOLLCFG *)malloc((g_mbPollTabLen) * sizeof(MBPOLLCFG))) == NULL)
	{
		printf("malloc g_devHardCfg error\n");
		exit(EXIT_FAILURE);
	}

	for(i=0;i<g_mbPollTabLen;i++)
	{
		g_mbPollTab[i].oid =atoi(pResult[nIndex]);
		g_mbPollTab[i].len =atoi(pResult[nIndex+2]);
		g_mbPollTab[i].pollPortNameIdx=changRs485Idx(pResult[nIndex+3]);
		//strcpy(g_mbPollTab[i].pollPortNameIdx,pResult[nIndex+3]);
		g_mbPollTab[i].mbDevAddr =atoi(pResult[nIndex+4]);
		g_mbPollTab[i].stdMbReadCmd =atoi(pResult[nIndex+5]);
		g_mbPollTab[i].stdMbSetCmd =atoi(pResult[nIndex+6]);
		g_mbPollTab[i].mbStartAddr =atoi(pResult[nIndex+7]);
		strcpy(g_mbPollTab[i].nonStdCmdId,pResult[nIndex+8]);

		nIndex=nIndex+nCol;
	}
	SortByRs485();
	SortByMbDevAddr();
	for(i=0;i<g_mbPollTabLen;i++)
	{
		printf("oid=%d ",g_mbPollTab[i].oid);
		printf("len=%d ",g_mbPollTab[i].len);
		printf("mbDevAddr=%d ",g_mbPollTab[i].mbDevAddr);
		printf("stdMbReadCmd=%d ",g_mbPollTab[i].stdMbReadCmd);
		printf("stdMbSetCmd=%d ",g_mbPollTab[i].stdMbSetCmd);
		printf("mbStartAddr=%d ",g_mbPollTab[i].mbStartAddr);
		printf("nonStdCmdId=%s ",g_mbPollTab[i].nonStdCmdId);
		printf("pollPortNameIdx=%d ",g_mbPollTab[i].pollPortNameIdx);
		printf("\n ");
	}
	  //---------------------------------------------------

	  sqlite3_close(pdb);
	  sqlite3_free(errmsg);

}
int changRs485Idx(char *p)
{
	int i;
	char  *rs485Name[]={"RS485-0","RS485-1","RS485-2","RS485-3","RS485-4","RS485-5"};

	for(i=0;i<6;i++)
	{
		if(!strcmp(p,rs485Name[i]))
				return i;
	}
	return 0;
}

void SortByRs485(void)
{
	int i,j;
	MBPOLLCFG temp;


	for(i=0; i<g_mbPollTabLen; i++){
		for(j=0; j<g_mbPollTabLen-i; j++){
			if(g_mbPollTab[j].pollPortNameIdx>g_mbPollTab[j+1].pollPortNameIdx){
				temp=g_mbPollTab[j];
				g_mbPollTab[j]=g_mbPollTab[j+1];
				g_mbPollTab[j+1]=temp;
			}
		}
	}
}
void SortByMbDevAddr(void)
{
	int m,n,i,j;
	MBPOLLCFG temp;
	int startAddr=0;

	for(m=0;m<6;m++)
	{
		for(i=0; i<g_mbPollTabLen; i++)
		{
			if(g_mbPollTab[i].pollPortNameIdx ==m)
			{
				startAddr=i;
				break;
			}
		}
		n=0;
		if(i>=g_mbPollTabLen)continue;

		for(i=startAddr;i<g_mbPollTabLen; i++)
		{
			if(g_mbPollTab[i].pollPortNameIdx !=m)
			{
				break;
			}
			n++;
		}
//		for(i=startAddr; i<endAddr; i++)
//		{
//			for(j=startAddr; j<endAddr-1; j++)
//			{
//				if(g_mbPollTab[j].mbDevAddr>g_mbPollTab[j+1].mbDevAddr)
//				{
//					temp=g_mbPollTab[j];
//					g_mbPollTab[j]=g_mbPollTab[j+1];
//					g_mbPollTab[j+1]=temp;
//				}
//			}
//		}

		for(i=0; i<n-1; i++){
			for(j=startAddr; j<n-1-i+startAddr; j++){
				if(g_mbPollTab[j].mbDevAddr>g_mbPollTab[j+1].mbDevAddr){
					temp=g_mbPollTab[j];
					g_mbPollTab[j]=g_mbPollTab[j+1];
					g_mbPollTab[j+1]=temp;
				}
			}
		}
//		for(i=startAddr; i<endAddr-1; i++)
//		{
//			if(g_mbPollTab[i].mbDevAddr>g_mbPollTab[i+1].mbDevAddr)
//			{
//				temp=g_mbPollTab[i];
//				g_mbPollTab[i]=g_mbPollTab[i+1];
//				g_mbPollTab[i+1]=temp;
//			}
//		}

	}
}
