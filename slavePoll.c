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
char dest[256];
void threadcreate(void);
void * func(void *arg);

const int idx[6]={0,1,2,3,4,5};
const char serialPath[6][15]={"/dev/ttymxc1","/dev/ttymxc2","/dev/ttymxc3","/dev/ttymxc4","/dev/ttymxc5","/dev/ttymxc7"};

void *slavePollThread(void)
{
//
	threadcreate();
    sleep(2);
    return 0;
	while(1)
	{
		DebugPrint("--------enter slavePollThread---------------\n");
		sleep(3);

		modbus_t * m_modbus;
		int i, addr, act;
		int ret = 0;

		m_modbus = modbus_new_rtu("/dev/ttymxc6", 9600, 'N', 8, 1); //波特率9600,比特位8，校验位1，无等价位
		modbus_set_debug(m_modbus, 1); //设置为1将可以看到调试信息
		struct timeval response_timeout;
		response_timeout.tv_sec = 1;
		response_timeout.tv_usec = 0;
		modbus_set_response_timeout(m_modbus,&response_timeout); //设置等待延时时间
		if(m_modbus && modbus_connect(m_modbus) == -1) {
			printf("\nSorry,fail to connect ttymxc6,please check your device's state.\n");
		}
		else {
			printf("Connected to dev!\n");
			//connected = 1;
		}

		if(m_modbus == NULL)
		{
			printf("No connection,check your device!\n");
			//return;
		}
		/* //注释中的代码作用是读取串口的4个端口数据
		uint8_t dest[4]; //setup memory for data
		uint16_t * dest16 = (uint16_t *) dest;
		memset(dest, 0, 4);
		*/
		modbus_set_slave(m_modbus, 1);
		ret = modbus_read_bits(m_modbus, 300, 4, dest);
		printf("modbus_read_bits ret=%d\n", ret);
		/* //本段注释代码给指定端口赋值
		ret = modbus_read_bits(m_modbus, 0, 4, dest);
		printf("%d\n", ret);
		for(i = 0; i < 4 ; i++)
		{
		printf("\n%d:%4d",i,dest[i]);
		}
		for(i = 0; i < 4 ; i++)
		{
		ret = modbus_write_bit(m_modbus, i , 1);
		if(ret)
		{
		printf("成功写入数据！返回值：%d\n",ret);
		}
		else
		{
		printf("写入数据失败，返回值：%d",ret);
		}
		sleep(2);
		}
		*/

		//断开连接
		if(m_modbus) {
			modbus_close(m_modbus);
			modbus_free(m_modbus);
			m_modbus = NULL;
		}
		printf("Disconnect succeed!\n");
		//return 0;
	}
}

void * func(void *arg)
{
    int num=*(int *)arg;
    /*
      此时程序会报错，无法获取num的值
      这是因为num在线程的栈内存中，arg指针本来是threadcreate()函数中a和b的指针，
      但是a,b是个临时变量，在控制线程的栈内存中，当执行完threadcreate()函数之后，变量a和b就会被系统释放
      此时我们在另外一个线程中取a的值就变得不可预期，因为此时a有可能已经被释放了，
      解决方案：可以在进程的堆内存上创建变量a和b，这样在另一个线程中释放，就没有问题了
     */
    while(1)
    {
    	printf("num is %d\n",num);
    	printf("serialPath is %s\n",&serialPath[num][0]);

       sleep(2);
    }
    //return NULL;
}
void threadcreate(void)
{
    pthread_t thr1,thr2,thr3,thr4,thr5,thr6;

    if(pthread_create(&thr1,NULL,func,&idx[0])!=0)
    {
        printf("create thread 0 failed!\n");
        return;
    }
    if(pthread_create(&thr2,NULL,func,&idx[1])!=0)
    {
        printf("create thread 1 failed!\n");
        return;
    }
    if(pthread_create(&thr3,NULL,func,&idx[2])!=0)
    {
        printf("create thread 2 failed!\n");
        return;
    }
    if(pthread_create(&thr4,NULL,func,&idx[3])!=0)
    {
        printf("create thread 3 failed!\n");
        return;
    }
    if(pthread_create(&thr5,NULL,func,&idx[4])!=0)
    {
        printf("create thread 4 failed!\n");
        return;
    }
    if(pthread_create(&thr6,NULL,func,&idx[5])!=0)
    {
        printf("create thread 5 failed!\n");
        return;
    }
}

