/*
*******************************************************************************
*                    Copyright (c) 2013, TQ System
*                             All Rights Reserved
*
* Purpose           : COM file
* File Name         : PUBLIC.H

* Revision history  :
    01a,2018/05/16,Ryan  -- Creation(written)

* Description       :

*******************************************************************************
*/
#include <pthread.h>

#ifndef _AP_PUBLIC_INCLUDED_
#define _AP_PUBLIC_INCLUDED_

//#define   TZigbee_PACK_HEAD       0xA5
#define   MAX_BUFF_LEN            40960
#define   MQ_BUFF_LEN             20480
#define   RECV_BUFF_LEN           10240
#define   MAX_PACKET_BUFF_LEN     2100  
#define BUSY 1;
#define IDLE 0;

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                  
typedef signed   char  INT8S;                   
typedef unsigned short INT16U;                  
typedef signed   short INT16S;                 
typedef unsigned int   INT32U;                  
typedef signed   int   INT32S;                 

#define DTU        0X01
#define MQTR       0X02
#define MQTC       0X03
#define UDP        0X04
#define MODEM      0X05
#define SERIAL1    0X06


#define DTU2MQTPR    0X0102
#define DTU2MQTPC    0X0103
#define MQTPC2DTU    0X0301
#define MQTPD2DTU    0X0401

#define MQTP_REPORT  0X04
#define MQTP_CTRL    0X05

#define SET_0        0

#define READ_10      10
#define READ_11      11
#define READ_30      11
#define READ_100     100

#define ALM_20       20
#define SET_1        1


typedef struct
{
	volatile unsigned long int    readPos;
	volatile unsigned long int    writePos;
    unsigned char   data[MAX_BUFF_LEN];
	pthread_mutex_t lock;
	pthread_cond_t  newPacketFlag;
	
}DATAS_BUFF_T;
typedef struct
{
	volatile unsigned long int    readPos;
	volatile unsigned long int    writePos;
    unsigned char   data[MQ_BUFF_LEN];
    unsigned int   mqttTopicFlag;
    unsigned int    len;
	pthread_mutex_t lock;
	pthread_cond_t  newPacketFlag;
	unsigned int    packetSum;
}MQTT_SENT_BUFF_T;
typedef struct
{
    unsigned char   data[RECV_BUFF_LEN];
    unsigned int   protoltype;
    unsigned int   scrFlag;//data from where
    unsigned int    len;
	pthread_mutex_t lock;
	pthread_cond_t  newPacketFlag;
}RECV_BUFF_T;

typedef struct{
	int oid;
	int valueInt;
	int valueIntPre;
	int radio;
	int upSentPeriod;
	int belongToOid;
	int dataOption;
	int belongOidIdx;
	char ssDevId[5];
	char valueString[10];
	char ssType[30];
	char ssDataType[30];
	char ssTypeCnName[50];
	char oidName[50];

}devDataTable;
typedef struct{
	char name[10];
	char option1[20];
	char option2[20];
	char option3[20];
	char option4[20];
	char option5[20];

}devHardCfg;

typedef struct{
	INT16U  oid;
	INT16U  len;
	INT16U  mbDevAddr;
	INT8U   stdMbReadCmd;
	INT8U   stdMbSetCmd;
	INT16U  mbStartAddr;
	INT8U   pollPortNameIdx;
	INT16U  comPacketIndex;
	INT16U  byteIndex;
	INT16U  bitIndex;
	INT16U   nonStdCmdId;

}MBPOLLCFG;

typedef struct
{
    unsigned int   fileLen;
    unsigned int   pkMaxLen;
    unsigned int   pkSum;
    unsigned int   pkCrc;
    unsigned int   curPkIndx;
    unsigned int   curState;
    char           dlName[50];
}DOWNLOAD;

typedef struct{

	INT8U name[30];
	INT8U content[50];
}DevCfgTable;

typedef union{
	INT8U byte[4];
	INT32U  lword;
}lWord4Byte;

typedef struct{
	INT8U   devAddr;
	INT8U   cmd;
	INT16U  starMbAddr;
	INT16U  byteSum;
}CONTENT_RECORD;

typedef struct{
	INT16U          packetInex;
	INT16U          starAddrIndex;//在总表中的开始位置。方便寻址
	INT8U           portIdx;
	INT16U          spCmdId;
	INT16U          recLen;
	CONTENT_RECORD  content;
}PACKET_record;

typedef struct{
	INT16U         id;
	INT8U          len;
	INT8U          ctx[20];
}nonStdMbCmdTab;

extern int  get_oidIdx(int oid);
extern int  get_typeValue(char *ssType);
extern void testCrc32(void);
extern INT32U caculate_crc( unsigned char *string, INT32U size);
extern int base64_decode( const char * base64, unsigned char * bindata );
extern void DebugPrint(char *fmt, ...);
extern void StrToHex(INT8U *pbDest, INT8U *pbSrc, INT16U nLen);
extern void rmZfFun(INT8U *p,INT8U ch);
//extern void calc_img_crc(void);

extern devDataTable *g_devDataTab;
extern devHardCfg *g_devHardCfg;
extern MBPOLLCFG *g_mbPollTab;
extern PACKET_record *g_CommPacket;
extern INT16U  g_tabLen;
extern INT16U  g_devHardCfgLen;
extern INT16U  g_mbPollTabLen;
extern INT16U g_comPackeIdx;


extern char *g_mqComVer;
extern char g_mqComId[];

extern char g_mqTopicReport[];
extern char g_mqTopicCtrl[];
extern char g_mqServer[];
extern char g_mqClientId[];
extern  RECV_BUFF_T RecvBuff4treat;

#endif

