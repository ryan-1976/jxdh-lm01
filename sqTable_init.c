#include <stdio.h>
#include "sqlite3.h"
#include <stdlib.h>
#include "public.h"
#include <string.h>
#include <arpa/inet.h>
#define POLLPACKETMAXLEN 100
devDataTable *g_devDataTab;
devHardCfg *g_devHardCfg;
MBPOLLCFG *g_mbPollTab;
char *g_mqComVer =NULL;
char g_mqComId[50]={0};
char g_mqTopicReport[50]={0};
char g_mqTopicCtrl[50]={0};
char g_mqServer[30]={0};
char g_mqClientId[50]={0};

INT16U  g_tabLen=0;
INT16U  g_devHardCfgLen=0;
INT16U  g_mbPollTabLen=0;
INT16U  g_nonStdMbCmdTabLen=0;
INT8U  g_temp[255]={0};
const char *dbPathName="./db-dhlm01.db";
static void getHardCfg(void);
static void getMbPollCfg(void);
static void getNonStdCmd(void);
PACKET_record *g_CommPacket;
nonStdMbCmdTab *g_nonStdMbCmdPacket;
INT16U g_comPackeIdx;

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
  getNonStdCmd();
  getMbPollCfg();


}

static void getNonStdCmd(void)
{
	char** pResult;
	int nRow,nCol,nResult;
	char* errmsg;
	int i,j;

	sqlite3 *pdb = NULL;
	nResult = sqlite3_open_v2(dbPathName,&pdb,SQLITE_OPEN_READWRITE,NULL);

    char * strSql = "select * from nonStdMbCmdTab";
    nResult = sqlite3_get_table(pdb,strSql,&pResult,&nRow,&nCol,&errmsg);
	if (nResult != SQLITE_OK)
	{
		sqlite3_close(pdb);
		sqlite3_free(errmsg);
		return ;
	}

	int nIndex = nCol;
	g_nonStdMbCmdTabLen = nRow;
	//---------------------------------------------------
	//nonStdMbCmdTab *g_nonStdMbCmdPacket;
	//printf("g_nonStdMbCmdTabLen=%d\n",g_nonStdMbCmdTabLen);
	if ((g_nonStdMbCmdPacket = (nonStdMbCmdTab *)malloc((g_nonStdMbCmdTabLen) * sizeof(nonStdMbCmdTab))) == NULL)
	{
		printf("malloc g_nonStdMbCmdPacket error\n");
		exit(EXIT_FAILURE);
	}

	for(i=0;i<g_nonStdMbCmdTabLen;i++)
	{
		g_nonStdMbCmdPacket[i].id = atoi(pResult[nIndex]);

		strcpy(g_temp,pResult[nIndex+1]);
		rmZfFun(g_temp,0x20);
		g_nonStdMbCmdPacket[i].sendLen = strlen(g_temp)/2;
        if(strlen(g_temp)%2 ==0 && g_nonStdMbCmdPacket[i].sendLen<20)
        {
        	StrToHex(&g_nonStdMbCmdPacket[i].ctx[0], g_temp,strlen(g_temp));
        }
        else
        {
        	g_nonStdMbCmdPacket[i].sendLen = 0;
        }
        g_nonStdMbCmdPacket[i].recLen = atoi(pResult[nIndex+2]);
        g_nonStdMbCmdPacket[i].paraGetStartAddr = atoi(pResult[nIndex+3]);
        nIndex=nIndex+nCol;
	}


    //-------------------------------------------------
	for(i=0;i<g_nonStdMbCmdTabLen;i++)
	{
		printf("---------------------------------\n");
		printf("nonCmdId=%d; sendLen=%d;recLen=%d;data=",g_nonStdMbCmdPacket[i].id,g_nonStdMbCmdPacket[i].sendLen,g_nonStdMbCmdPacket[i].recLen);
		for(j=0;j<g_nonStdMbCmdPacket[i].sendLen;j++)
		{
			printf("%d ",g_nonStdMbCmdPacket[i].ctx[j]);

		}

		printf("\n");
	}
    //-------------------------------------------------


	  sqlite3_close(pdb);
	  sqlite3_free(errmsg);

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

	if ((g_mbPollTab = (MBPOLLCFG *)malloc(g_mbPollTabLen * sizeof(MBPOLLCFG))) == NULL)
	{
		printf("malloc g_devHardCfg error\n");
		sleep(1);
		exit(EXIT_FAILURE);
	}

	for(i=0;i<g_mbPollTabLen;i++)
	{
		g_mbPollTab[i].oid =atoi(pResult[nIndex]);
		g_mbPollTab[i].len =atoi(pResult[nIndex+2]);
		g_mbPollTab[i].pollPortNameIdx=changRs485Idx(pResult[nIndex+3]);
		g_mbPollTab[i].mbDevAddr =atoi(pResult[nIndex+4]);
		g_mbPollTab[i].stdMbReadCmd =atoi(pResult[nIndex+5]);
		g_mbPollTab[i].stdMbSetCmd =atoi(pResult[nIndex+6]);
		g_mbPollTab[i].mbStartAddr =atoi(pResult[nIndex+7]);
		g_mbPollTab[i].nonStdCmdId =atoi(pResult[nIndex+8]);
		g_mbPollTab[i].option =atoi(pResult[nIndex+9]);
		nIndex=nIndex+nCol;
	}
	SortByNonStdCmd();
	SortByRs485();
	SortByMbDevAddr();
	SortByMbDevReadCmd();
	SortByMbStartAddr();
	g_commPacketForm();
	g_commPacketSortByRs485();
	for(i=0;i<g_mbPollTabLen;i++)
	{

		printf("pollPortNameIdx=%d ",g_mbPollTab[i].pollPortNameIdx);
		printf("mbDevAddr=%d ",g_mbPollTab[i].mbDevAddr);
		printf("comPacketIndex=%d ",g_mbPollTab[i].comPacketIndex);
		printf("byteIndex=%d ",g_mbPollTab[i].byteIndex);
		printf("bitIndex=%d ",g_mbPollTab[i].bitIndex);
		printf("stdMbReadCmd=%d ",g_mbPollTab[i].stdMbReadCmd);
		printf("mbStartAddr=%d ",g_mbPollTab[i].mbStartAddr);
		printf("oid=%d ",g_mbPollTab[i].oid);
		printf("len=%d ",g_mbPollTab[i].len);
		printf("stdMbSetCmd=%d ",g_mbPollTab[i].stdMbSetCmd);
		printf("nonStdCmdId=%d ",g_mbPollTab[i].nonStdCmdId);
		printf("\n ");
	}

	//exit(1);
	  //---------------------------------------------------

	  sqlite3_close(pdb);
	  sqlite3_free(errmsg);

}
int changRs485Idx(char *p)
{
	INT16U i;
	char  *rs485Name[]={"RS485-0","RS485-1","RS485-2","RS485-3","RS485-4","RS485-5"};

	for(i=0;i<6;i++)
	{
		if(!strcmp(p,rs485Name[i]))
				return i;
	}
	return 0;
}
void g_commPacketSortByRs485(void)
{
	INT16U i,j;
	PACKET_record temp;

	for(i=0; i<g_comPackeIdx; i++){
		for(j=0; j<g_comPackeIdx-i-1; j++){
			if(g_CommPacket[j].portIdx>g_CommPacket[j+1].portIdx){
				temp=g_CommPacket[j];
				g_CommPacket[j]=g_CommPacket[j+1];
				g_CommPacket[j+1]=temp;
			}
		}
	}
	printf("3--------g_comPackeIdx=%d-----------------------------------------\n",g_comPackeIdx);
	for(i=0;i<g_mbPollTabLen;i++){
		if(g_CommPacket[i].packetInex ==0xff)break;
		printf("g_index=%d ",g_CommPacket[i].packetInex);
		printf("portIdx=%d ",g_CommPacket[i].portIdx);
		printf("starAddrIndex=%d ",g_CommPacket[i].starAddrIndex);
		printf("spCmdId=%d ",g_CommPacket[i].spCmdId);
		printf("recLen=%d ",g_CommPacket[i].recLen);
		printf("sendLen=%d ",g_CommPacket[i].sendLen);
		printf("devAddr=%x ",g_CommPacket[i].content.devAddr);
		printf("cmd=%x ",g_CommPacket[i].content.cmd);
		printf("starMbAddr=%x ",g_CommPacket[i].content.starMbAddr);
		printf("byteSum=%x ",g_CommPacket[i].content.byteSum);
		printf("crc=%x \n",g_CommPacket[i].content.crc);
	}
	printf("4-------------------------------------------------\n");
}
void SortByNonStdCmd(void)
{
	INT16U i,j;
	MBPOLLCFG temp;

	for(i=0; i<g_mbPollTabLen; i++){
		for(j=0; j<g_mbPollTabLen-i-1; j++){
			if(g_mbPollTab[j].nonStdCmdId>g_mbPollTab[j+1].nonStdCmdId){
				temp=g_mbPollTab[j];
				g_mbPollTab[j]=g_mbPollTab[j+1];
				g_mbPollTab[j+1]=temp;
			}
		}
	}
}
void SortByRs485(void)
{
	INT16U m,i,j;
	MBPOLLCFG temp;
	INT16U curStartAddr=0;
	INT16U nxStartAddr=0;
	INT16U cnt=0,sortFlag;
	lWord4Byte curType,nxType;

	for(i=0; i<g_mbPollTabLen; i++)
	{
		sortFlag=0;
		curType.lword=0;
		nxType.lword =0;

		curType.byte[0]=g_mbPollTab[i].nonStdCmdId;
		if(i<=g_mbPollTabLen-1)
		{
			nxType.byte[0]=g_mbPollTab[i+1].nonStdCmdId;
		}
		if(curType.lword !=nxType.lword)
		{
			nxStartAddr =i+1;
			cnt =i-curStartAddr+1;
			sortFlag =1;
		}

		if(sortFlag){
			for(m=0; m<cnt-1; m++){
				for(j=curStartAddr; j<cnt-1-m+curStartAddr; j++){
					if(g_mbPollTab[j].pollPortNameIdx>g_mbPollTab[j+1].pollPortNameIdx){
						temp=g_mbPollTab[j];
						g_mbPollTab[j]=g_mbPollTab[j+1];
						g_mbPollTab[j+1]=temp;
					}
				}
			}
		}
		curStartAddr=nxStartAddr;
	}

	for(m=0; m<g_mbPollTabLen; m++)
	{
		if(g_mbPollTab[m].nonStdCmdId !=0)break;
	}
	for(i=m; i<g_mbPollTabLen; i++){
		for(j=0; j<g_mbPollTabLen-i-1; j++){
			if(g_mbPollTab[j+m].pollPortNameIdx>g_mbPollTab[j+1+m].pollPortNameIdx){
				temp=g_mbPollTab[j+m];
				g_mbPollTab[j+m]=g_mbPollTab[j+1+m];
				g_mbPollTab[j+1+m]=temp;
			}
		}
	}

}
void SortByMbDevReadCmd(void)
{
	INT16U m,i,j;
	MBPOLLCFG temp;
	INT16U curStartAddr=0;
	INT16U nxStartAddr=0;
	INT16U cnt=0,sortFlag;

	lWord4Byte curType,nxType;


	for(i=0; i<g_mbPollTabLen; i++)
	{
		sortFlag=0;

		curType.lword=0;
		nxType.lword =0;
		curType.byte[0]=g_mbPollTab[i].nonStdCmdId;
		curType.byte[1]=g_mbPollTab[i].pollPortNameIdx;
		curType.byte[2]=g_mbPollTab[i].mbDevAddr;


		if(i<=g_mbPollTabLen-1)
		{
			nxType.byte[0]=g_mbPollTab[i+1].nonStdCmdId;
			nxType.byte[1]=g_mbPollTab[i+1].pollPortNameIdx;
			nxType.byte[2]=g_mbPollTab[i+1].mbDevAddr;

		}

		if(curType.lword !=nxType.lword)
		{
			nxStartAddr =i+1;
			cnt =i-curStartAddr+1;
			sortFlag =1;
		}

		if(sortFlag){
			for(m=0; m<cnt-1; m++){
				for(j=curStartAddr; j<cnt-1-m+curStartAddr; j++){
					if(g_mbPollTab[j].stdMbReadCmd>g_mbPollTab[j+1].stdMbReadCmd){
						temp=g_mbPollTab[j];
						g_mbPollTab[j]=g_mbPollTab[j+1];
						g_mbPollTab[j+1]=temp;
					}
				}
			}
		}
		curStartAddr=nxStartAddr;
		}
	}
void SortByMbDevAddr(void)
{
	INT16U m,i,j;
	MBPOLLCFG temp;
	INT16U curStartAddr=0;
	INT16U nxStartAddr=0;
	INT16U cnt=0,sortFlag;

	lWord4Byte curType,nxType;


	for(i=0; i<g_mbPollTabLen; i++)
	{
		sortFlag=0;
		curType.lword=0;
		nxType.lword =0;
		curType.byte[0]=g_mbPollTab[i].nonStdCmdId;
		curType.byte[1]=g_mbPollTab[i].pollPortNameIdx;
		if(i<=g_mbPollTabLen-1)
		{
			nxType.byte[0]=g_mbPollTab[i+1].nonStdCmdId;
			nxType.byte[1]=g_mbPollTab[i+1].pollPortNameIdx;
		}
		if(curType.lword !=nxType.lword)
		{
			nxStartAddr =i+1;
			cnt =i-curStartAddr+1;
			sortFlag =1;
		}
		if(sortFlag){
			for(m=0; m<cnt-1; m++){
				for(j=curStartAddr; j<cnt-1-m+curStartAddr; j++){
					if(g_mbPollTab[j].mbDevAddr>g_mbPollTab[j+1].mbDevAddr){
						temp=g_mbPollTab[j];
						g_mbPollTab[j]=g_mbPollTab[j+1];
						g_mbPollTab[j+1]=temp;
					}
				}
			}
		}
		curStartAddr=nxStartAddr;
		}
	}
void SortByMbStartAddr(void)
{
	INT16U m,i,j;
	MBPOLLCFG temp;
	INT16U curStartAddr=0;
	INT16U nxStartAddr=0;
	INT16U cnt=0,sortFlag;
	lWord4Byte curType,nxType;

	for(i=0; i<g_mbPollTabLen; i++)
	{
		sortFlag=0;
		curType.lword=0;
		nxType.lword =0;
		curType.byte[0]=g_mbPollTab[i].nonStdCmdId;
		curType.byte[1]=g_mbPollTab[i].pollPortNameIdx;
		curType.byte[2]=g_mbPollTab[i].mbDevAddr;
		curType.byte[3]=g_mbPollTab[i].stdMbReadCmd;
		if(i<=g_mbPollTabLen-1)
		{
			nxType.byte[0]=g_mbPollTab[i+1].nonStdCmdId;
			nxType.byte[1]=g_mbPollTab[i+1].pollPortNameIdx;
			nxType.byte[2]=g_mbPollTab[i+1].mbDevAddr;
			nxType.byte[3]=g_mbPollTab[i+1].stdMbReadCmd;
		}
		if(curType.lword !=nxType.lword)
		{
			nxStartAddr =i+1;
			cnt =i-curStartAddr+1;
			sortFlag =1;
		}
		if(sortFlag){
			for(m=0; m<cnt-1; m++){
				for(j=curStartAddr; j<cnt-1-m+curStartAddr; j++){
					if(g_mbPollTab[j].mbStartAddr>g_mbPollTab[j+1].mbStartAddr){
						temp=g_mbPollTab[j];
						g_mbPollTab[j]=g_mbPollTab[j+1];
						g_mbPollTab[j+1]=temp;
					}
				}
			}
		}
		curStartAddr=nxStartAddr;
		}
	}

void g_commPacketForm(void)
{
	INT16U m,i,j;
	MBPOLLCFG temp;
	INT16U curStartAddr=0;
	INT16U nxStartAddr=0;
	INT16U cnt=0;
	INT8U sortFlag;
	lWord4Byte curType,nxType;

	if ((g_CommPacket = (PACKET_record *)malloc((g_mbPollTabLen) * sizeof(PACKET_record))) == NULL)
	{
		printf("malloc g_CommPacket error\n");
		exit(EXIT_FAILURE);
	}
	for(i=0;i<g_mbPollTabLen;i++)
	{
		g_CommPacket[i].packetInex =0xff;
		g_CommPacket[i].spCmdId=0;
	}
	g_comPackeIdx=0;
	for(i=0; i<g_mbPollTabLen; i++)
	{
		sortFlag=0;
		curType.lword=0;
		nxType.lword =0;
		curType.byte[0]=g_mbPollTab[i].nonStdCmdId;
		curType.byte[1]=g_mbPollTab[i].pollPortNameIdx;
		curType.byte[2]=g_mbPollTab[i].mbDevAddr;
		curType.byte[3]=g_mbPollTab[i].stdMbReadCmd;
		if(i<=g_mbPollTabLen-1)
		{
			nxType.byte[0]=g_mbPollTab[i+1].nonStdCmdId;
			nxType.byte[1]=g_mbPollTab[i+1].pollPortNameIdx;
			nxType.byte[2]=g_mbPollTab[i+1].mbDevAddr;
			nxType.byte[3]=g_mbPollTab[i+1].stdMbReadCmd;
		}
		if(curType.lword !=nxType.lword)
		{
			nxStartAddr =i+1;
			cnt =i-curStartAddr+1;
			sortFlag =1;
		}
		if(sortFlag)
		{
			if(g_mbPollTab[curStartAddr].len<2&&g_mbPollTab[curStartAddr].nonStdCmdId==0)
			{
				PollBits_PacketTreat(curStartAddr,cnt);
			}
			else if(g_mbPollTab[curStartAddr].len==2&&g_mbPollTab[curStartAddr].nonStdCmdId==0)
			{
				PollWords_PacketTreat(curStartAddr,cnt);
			}
			else if(0!=g_mbPollTab[curStartAddr].nonStdCmdId)
			{
				PollSpecial_PacketTreat(curStartAddr,cnt);
			}
		}
		curStartAddr=nxStartAddr;
	}
	printf("1-------------------------------------------------\n");
	for(i=0;i<g_mbPollTabLen;i++){
		if(g_CommPacket[i].packetInex ==0xff)break;
		printf("g_index=%d ",g_CommPacket[i].packetInex);
		printf("portIdx=%d ",g_CommPacket[i].portIdx);
		printf("starAddrIndex=%d ",g_CommPacket[i].starAddrIndex);
		printf("spCmdId=%d ",g_CommPacket[i].spCmdId);
		printf("recLen=%d ",g_CommPacket[i].recLen);
		printf("sendLen=%d ",g_CommPacket[i].sendLen);
		printf("devAddr=%x ",g_CommPacket[i].content.devAddr);
		printf("cmd=%x ",g_CommPacket[i].content.cmd);
		printf("starMbAddr=%x ",g_CommPacket[i].content.starMbAddr);
		printf("byteSum=%x ",g_CommPacket[i].content.byteSum);
		printf("crc=%x \n",g_CommPacket[i].content.crc);
	}
	printf("2-------------------------------------------------\n");
}

//只读BIT组包处理

void PollBits_PacketTreat(INT16U starAddrIdx,INT16U len)
{
	INT16U i,j,temp,starAddr;
	MBPOLLCFG *pPtr;
	INT16U nxStarAddrIdx;
	const unsigned char mask[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

	pPtr =&g_mbPollTab[starAddrIdx];

	nxStarAddrIdx=0;
	for(i=0;i<len;i++){
		g_CommPacket[g_comPackeIdx].packetInex=g_comPackeIdx;
		g_CommPacket[g_comPackeIdx].starAddrIndex=starAddrIdx+nxStarAddrIdx;
		g_CommPacket[g_comPackeIdx].content.cmd=pPtr[nxStarAddrIdx].stdMbReadCmd;
		g_CommPacket[g_comPackeIdx].content.starMbAddr=pPtr[nxStarAddrIdx].mbStartAddr;
		g_CommPacket[g_comPackeIdx].portIdx=pPtr[nxStarAddrIdx].pollPortNameIdx;
		g_CommPacket[g_comPackeIdx].content.devAddr=pPtr[nxStarAddrIdx].mbDevAddr;


		for(j=1;j<(POLLPACKETMAXLEN+1)*8;j++)
		{
			pPtr[nxStarAddrIdx+j-1].comPacketIndex=g_comPackeIdx;
			temp = pPtr[nxStarAddrIdx+j-1].mbStartAddr-pPtr[nxStarAddrIdx].mbStartAddr;
			pPtr[nxStarAddrIdx+j-1].byteIndex=temp/8+4;
			pPtr[nxStarAddrIdx+j-1].bitIndex =mask[temp%8];

			if((nxStarAddrIdx+j)>=len)break;
			if((pPtr[nxStarAddrIdx+j].mbStartAddr-pPtr[nxStarAddrIdx].mbStartAddr)>=(POLLPACKETMAXLEN*8))break;
			//if((pPtr[nxStarAddrIdx+j].mbStartAddr-pPtr[nxStarAddrIdx].mbStartAddr)>=pPtr[len-1].mbStartAddr)break;
			else{};
		}
		temp=pPtr[nxStarAddrIdx+j-1].mbStartAddr -pPtr[nxStarAddrIdx].mbStartAddr+1;
    	g_CommPacket[g_comPackeIdx].recLen =(int)ceil((float)temp/ 8)+5;
    	g_CommPacket[g_comPackeIdx].content.byteSum=temp;
    	g_CommPacket[g_comPackeIdx].sendLen =8;
		//--------------------------------------------------------------------------------------------------
		g_CommPacket[g_comPackeIdx].content.byteSum=htons(g_CommPacket[g_comPackeIdx].content.byteSum);
		g_CommPacket[g_comPackeIdx].content.starMbAddr=htons(g_CommPacket[g_comPackeIdx].content.starMbAddr);
		//g_CommPacket[g_comPackeIdx].content.crc=modbusCrc16(&g_CommPacket[g_comPackeIdx].content,6);
		temp =modbusCrc16(&g_CommPacket[g_comPackeIdx].content,6);
		g_CommPacket[g_comPackeIdx].content.crc=htons(temp);
		//---------------------------------------------------------------------------------------------------
		nxStarAddrIdx +=j;
		g_comPackeIdx++;
		if(nxStarAddrIdx>=len)break;
	}
}

void PollWords_PacketTreat(INT16U starAddrIdx,INT16U len)
{
	INT16U i,j,temp,starAddr;
	MBPOLLCFG *pPtr;
	INT16U nxStarAddrIdx;

	pPtr =&g_mbPollTab[starAddrIdx];

	nxStarAddrIdx=0;
	for(i=0;i<len;i++){
		g_CommPacket[g_comPackeIdx].packetInex=g_comPackeIdx;
		g_CommPacket[g_comPackeIdx].starAddrIndex=starAddrIdx+nxStarAddrIdx;
		g_CommPacket[g_comPackeIdx].content.cmd=pPtr[nxStarAddrIdx].stdMbReadCmd;
		g_CommPacket[g_comPackeIdx].content.starMbAddr=pPtr[nxStarAddrIdx].mbStartAddr;
		g_CommPacket[g_comPackeIdx].portIdx=pPtr[nxStarAddrIdx].pollPortNameIdx;
		g_CommPacket[g_comPackeIdx].content.devAddr=pPtr[nxStarAddrIdx].mbDevAddr;

		for(j=1;j<(POLLPACKETMAXLEN+1);j++)
		{
			pPtr[nxStarAddrIdx+j-1].comPacketIndex=g_comPackeIdx;
			temp = pPtr[nxStarAddrIdx+j-1].mbStartAddr-pPtr[nxStarAddrIdx].mbStartAddr;
			pPtr[nxStarAddrIdx+j-1].byteIndex=temp*2+4;
			pPtr[nxStarAddrIdx+j-1].bitIndex =0xff;

			if((nxStarAddrIdx+j)>=len)break;
			if((pPtr[nxStarAddrIdx+j].mbStartAddr-pPtr[nxStarAddrIdx].mbStartAddr)>=POLLPACKETMAXLEN)break;
			if((pPtr[nxStarAddrIdx+j].mbStartAddr-pPtr[nxStarAddrIdx].mbStartAddr)>pPtr[len-1].mbStartAddr)break;
			else{};
		}
		g_CommPacket[g_comPackeIdx].content.byteSum=pPtr[nxStarAddrIdx+j-1].mbStartAddr -pPtr[nxStarAddrIdx].mbStartAddr+1;
		g_CommPacket[g_comPackeIdx].recLen =5+g_CommPacket[g_comPackeIdx].content.byteSum*2;
		g_CommPacket[g_comPackeIdx].sendLen =8;
		//--------------------------------------------------------------------------------------------------
		g_CommPacket[g_comPackeIdx].content.byteSum=htons(g_CommPacket[g_comPackeIdx].content.byteSum);
		g_CommPacket[g_comPackeIdx].content.starMbAddr=htons(g_CommPacket[g_comPackeIdx].content.starMbAddr);
		temp =modbusCrc16(&g_CommPacket[g_comPackeIdx].content,6);
		g_CommPacket[g_comPackeIdx].content.crc=htons(temp);
		//---------------------------------------------------------------------------------------------------
		nxStarAddrIdx +=j;
		g_comPackeIdx++;
		if(nxStarAddrIdx>=len)break;
	}
}

void PollSpecial_PacketTreat(INT16U starAddrIdx,INT16U len)
{
	INT16U i,j,temp,starAddr;
	MBPOLLCFG *pPtr;

	pPtr =&g_mbPollTab[starAddrIdx];
	g_CommPacket[g_comPackeIdx].packetInex=g_comPackeIdx;
	g_CommPacket[g_comPackeIdx].portIdx=pPtr[0].pollPortNameIdx;
	g_CommPacket[g_comPackeIdx].starAddrIndex=starAddrIdx;
	g_CommPacket[g_comPackeIdx].spCmdId=pPtr[0].nonStdCmdId;

	g_CommPacket[g_comPackeIdx].sendLen =0;
	for(j=0;j<g_nonStdMbCmdTabLen;j++)
	{
		if(pPtr[0].nonStdCmdId==g_nonStdMbCmdPacket[j].id)
		{
			g_CommPacket[g_comPackeIdx].sendLen =g_nonStdMbCmdPacket[j].sendLen;
			g_CommPacket[g_comPackeIdx].recLen =g_nonStdMbCmdPacket[j].recLen;
			break;
		}
	}
    for(i=0;i<len;i++)
	{
		pPtr[i].comPacketIndex=g_comPackeIdx;
		pPtr[i].byteIndex=pPtr[i].mbStartAddr+g_nonStdMbCmdPacket[j].paraGetStartAddr;
		pPtr[i].bitIndex =0xff;
	}
	g_comPackeIdx++;
}


