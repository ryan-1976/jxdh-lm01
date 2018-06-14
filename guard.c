#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>

#define FIFO_SERVER "/tmp/myfifo"
char g_guardString[20]="ping";
#define MAXLINE 1024

void *guardMonitorThread(void)
{
	int fd;
	int nwrite;

	sleep(1);
	return;
	fd=open(FIFO_SERVER,O_WRONLY|O_NONBLOCK,0);
	if(fd<0)exit(1);

	while(1)
	{
	    /* 向管道写入数据 */
	    if((nwrite=write(fd,g_guardString,sizeof(g_guardString)))==-1)
	    {
	         printf("The FIFO has not been read yet.Please try later\n");
	    }
	    else
	    {
	    	printf("write %s to the FIFO\n",g_guardString);
	    }
	    sleep(3);
	}
}
