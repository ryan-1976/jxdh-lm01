#include <stdio.h>
#include "cJSON.h"
#include "sqlite3.h"
#include <stdlib.h> //exit
#include <pthread.h>
#include "circlebuff.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stddef.h>
#include <sys/wait.h>
extern void * mqtt_sub_treat(int argc, char* argv[]);
extern void * mqttPubThread(int argc, char* argv[]);
extern void * msgDisPatcherThread(void);
extern void * sampleDataThread(void);
extern void * guardMonitorThread(void);
extern void * comThread(void);
extern void * slavePollThread(void);
extern void * udpServerThread(void);

//extern void * sqlite_treat(void);
//extern void * bluetooth_treat(void);
extern void G_Buff_init(void);
extern void  json_init();
extern void  sqTable_init();
extern  void  *makeJson(int idx);
int main()
{


//	pthread_t th_a, th_b,th_c,th_d,th_e,th_f;
//	pthread_t th_1;
	pthread_t threadId[10];

	void *retval;
	//bootMonitor();
	printf("compile time is: %s %s\n",__DATE__,__TIME__);
	//json_init();
	sqTable_init();
	G_Buff_init();
	//tabProc();
	//pthread_create(&th_d, NULL, (void *(*)(void *))sqlite_treat, 0);
//	pthread_create(&th_a, NULL, (void *(*)(void *))msgDisPatcherThread, 0);
//	pthread_create(&th_b, NULL, (void *(*)(void *))comThread, 0);
//	pthread_create(&th_c, NULL, (void *(*)(void *))mqttPubThread, 0);
//	pthread_create(&th_d, NULL, (void *(*)(void *))sampleDataThread, 0);
//	pthread_create(&th_e, NULL, (void *(*)(void *))guardMonitorThread, 0);
//	pthread_create(&th_f, NULL, (void *(*)(void *))udpServerThread, 0);
//	pthread_create(&th_1, NULL, (void *(*)(void *))slavePollThread, 0);
//	//pthread_create(&th_f, NULL, (void *(*)(void *))bluetooth_treat, 0);
//
//	//pthread_join(th_d, &retval);
//	pthread_join(th_a, &retval);
//	pthread_join(th_b, &retval);
//	pthread_join(th_c, &retval);
//	pthread_join(th_d, &retval);
//	pthread_join(th_e, &retval);
//	pthread_join(th_f, &retval);
//	pthread_join(th_1, &retval);

	pthread_create(&threadId[0], NULL, (void *(*)(void *))msgDisPatcherThread, 0);
	pthread_create(&threadId[1], NULL, (void *(*)(void *))comThread, 0);
	pthread_create(&threadId[2], NULL, (void *(*)(void *))mqttPubThread, 0);
	pthread_create(&threadId[3], NULL, (void *(*)(void *))sampleDataThread, 0);
	pthread_create(&threadId[4], NULL, (void *(*)(void *))guardMonitorThread, 0);
	pthread_create(&threadId[5], NULL, (void *(*)(void *))udpServerThread, 0);
	pthread_create(&threadId[6], NULL, (void *(*)(void *))slavePollThread, 0);

	pthread_join(threadId[0], &retval);
	pthread_join(threadId[1], &retval);
	pthread_join(threadId[2], &retval);
	pthread_join(threadId[3], &retval);
	pthread_join(threadId[4], &retval);
	pthread_join(threadId[5], &retval);
	pthread_join(threadId[6], &retval);
    return 0;
}

