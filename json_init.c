 #include <stdio.h>
#include "sqlite3.h"
#include <stdlib.h>
#include "cJSON.h"

cJSON *g_devCfgJsonRoot,*g_devCfgJsonSerial[6];
char *g_mqComVer;
char *g_mqComId;
cJSON *readJsonFile(char *filename)
{  
    FILE *f;  
    long len;  
    char *data;  
    cJSON *json,*ret;  
      
    f=fopen(filename,"rb");  
    fseek(f,0,SEEK_END);  
    len=ftell(f);  
    fseek(f,0,SEEK_SET);  
    data=(char*)malloc(len+1);  
    fread(data,1,len,f);  
    data[len]='\0';  
    json=cJSON_Parse(data);  
    if (!json)   
    {  
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
        ret = NULL;  
        goto EXIT;  
    }  
    else  
    {  
        ret = json;  
    }  
  
EXIT:  
    free(data);  
    fclose(f);  
    return ret;  
}  
  
int writeJsonFile(char *filename,char *out)
{  
    FILE *fp = NULL;  
  
    fp = fopen(filename,"w+");
    if(fp == NULL)  
    {  
        fprintf(stderr,"open file failed\n");  
        return -1;
       // exit(-1);
    }  
    fprintf(fp,"%s",out);  
  
    if(fp != NULL)  
        fclose(fp);
    return 1;
}  
  
int json_init()
{  

	g_devCfgJsonRoot = readJsonFile("devCfg.json");
	//printf("before modify:%s\n",cJSON_Print(devCfgJsonRoot));
	g_devCfgJsonSerial[0]= cJSON_GetObjectItem(g_devCfgJsonRoot,"serialPort1");
	g_devCfgJsonSerial[1]= cJSON_GetObjectItem(g_devCfgJsonRoot,"serialPort2");
	g_devCfgJsonSerial[2]= cJSON_GetObjectItem(g_devCfgJsonRoot,"serialPort3");
	g_devCfgJsonSerial[3]= cJSON_GetObjectItem(g_devCfgJsonRoot,"serialPort4");
	g_devCfgJsonSerial[4]= cJSON_GetObjectItem(g_devCfgJsonRoot,"serialPort5");
	g_devCfgJsonSerial[5]= cJSON_GetObjectItem(g_devCfgJsonRoot,"serialPort6");
	g_mqComVer = cJSON_GetObjectItem(g_devCfgJsonRoot,"mqComVersion")->valuestring;
	g_mqComId = cJSON_GetObjectItem(g_devCfgJsonRoot,"mqComId")->valuestring;


    return 0;  
}  


