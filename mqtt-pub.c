//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include <unistd.h>
#include "circlebuff.h"
#include "public.h"

//#define PAYLOAD     "wgrt-data"
#define QOS         1
#define TIMEOUT     5000L
//const char *g_mqTopicDl="jianxin/baiyun_0002/control";
extern MQTT_SENT_BUFF_T   mqBuff;
extern DATAS_BUFF_T   comBuff0;
volatile MQTTClient_deliveryToken deliveredtoken;
char pubBuf[1500];
int g_test=0;
extern unsigned char mq_circleBuff_ReadData(void);
void delivered(void *context, MQTTClient_deliveryToken dt)
{
   // printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
//    int i;
    INT8U * payloadptr;
//    printf(" new mqtt Message arrived:");
//    printf(" topic: %s", topicName);
//	printf(" topicLen:%d\n ",message->payloadlen);
    //printf(" mq recv message=%s\n ",message->payload);
    payloadptr = message->payload;


	pthread_mutex_lock(&comBuff0.lock);
	AP_circleBuff_WritePacket(payloadptr,message->payloadlen,MQTPC2DTU);
	pthread_cond_signal(&comBuff0.newPacketFlag);
	pthread_mutex_unlock(&comBuff0.lock);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost cause: %s\n", cause);
}

//----------------------------------------------------------------------
void *mqttPubThread(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_deliveryToken token;
    int rc, i;


    MQTTClient_create(&client, g_mqServer, g_mqClientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;

   MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("start up Failed to connect, return code %d\n", rc);
	}
	printf("Sub topic: %s\nfor clientId: %s using QoS%d\n\n", g_mqTopicCtrl, g_mqClientId, QOS);
	MQTTClient_subscribe(client, g_mqTopicCtrl, QOS);
	//MQTTClient_subscribe(client, g_mqTopicDl, QOS);
	printf(" mqtt sub ok 1\n");

	while(1)
	{
		while(!MQTTClient_isConnected(client)){
			if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
			{
				printf(" pub retry to connect failure, return code %d\n", rc);
			}
			else
			{
				MQTTClient_subscribe(client, g_mqTopicCtrl, QOS);
				//MQTTClient_subscribe(client, g_mqTopicDl, QOS);
				printf(" mqtt sub ok 2\n");
			}
			sleep(3);
		};
		pthread_mutex_lock(&mqBuff.lock);
		if(mqBuff.packetSum ==0)
		{
			pthread_cond_wait(&mqBuff.newPacketFlag, &mqBuff.lock);
		}

		mqBuff.mqttTopicFlag=mq_circleBuff_ReadData();
		mqBuff.mqttTopicFlag =mqBuff.mqttTopicFlag *256;
		mqBuff.mqttTopicFlag +=mq_circleBuff_ReadData();

		mqBuff.len=mq_circleBuff_ReadData();
		mqBuff.len =mqBuff.len *256;
		mqBuff.len +=mq_circleBuff_ReadData();

		//char * pubBuf= (char *)malloc(mqBuff.len+1);
		for(i =0;i<mqBuff.len;i++)
		{
			pubBuf[i]= mq_circleBuff_ReadData();
		}
		if(mqBuff.packetSum >0)mqBuff.packetSum--;
		pubBuf[i] =0;
		//pthread_mutex_unlock(&mqBuff.lock);
		//printf("len=%d,%s\n",mqBuff.len,pubBuf);
		//printf("pubbuf=%s\n",pubBuf);
		
		g_test++;
	//	printf("g_test=%d\n",g_test);

		if(mqBuff.mqttTopicFlag == MQTP_REPORT)
		{
			MQTTClient_publish(client, g_mqTopicReport,mqBuff.len, pubBuf,QOS,0, &token);
			//printf("%s\n",pubBuf);
		}
		else
		{
			MQTTClient_publish(client, g_mqTopicCtrl,mqBuff.len, pubBuf,QOS,0, &token);
			//printf("%s\n",pubBuf);
		}
		pthread_mutex_unlock(&mqBuff.lock);
		//usleep(1000);

		//free(pubBuf);
//		 printf("Waiting for up to %d seconds for publication of %s\n"	 ""
//				 "on topic %s for client with ClientID: %s\n",
//		 (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
		 rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

//		 printf("Message with delivery token %d delivered\n", token);

		//--------------------------------------
		

	}

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    //return rc;
}
