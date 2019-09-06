#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#include "graph_module.h"

const char * FILENAME = "/proc/temp_process_entry";

int main()
{
	srand((unsigned int) time(NULL));

	int32_t number = 0 ;
	 

	int fd = open(FILENAME, O_RDWR);

	if(fd < 0)
	{
		perror("Unable to open file\n");
		return -1;
	}

	int len = 10;
	int ret = 0;
	
	printf("setting order and type for ioctl interface\n");
	
	char input ;
	input = DATA_TYPE_INT;

	ret = ioctl(fd, PB2_SET_TYPE, &input);
	if(ret < 0) perror("ioctl failed\n");

	input = GRAPH_IN_ORDER;
	ret = ioctl(fd, PB2_SET_ORDER, &input);
	if(ret < 0) perror("ioctl failed\n");
	printf("writing");
	while(len)
	{
		number = rand() % 1000;
		
		printf(" %d", number);
		ret = write(fd, &number, sizeof(int32_t));
		
		if(ret < 0)
		{
			perror("write failed\n");
		}
		
		len--;
	}
	printf("\n");
	while(len < 10)
	{
		ret = read(fd, &number, sizeof(int32_t));
		if(ret < 0) perror("read failed \n");

		printf("%d ", number);

		len++;
	}

	printf("\n");
	close(fd);	
	
	fd = open(FILENAME, O_RDWR);

	if(fd < 0)
	{
		perror("unable to open file\n");
		return 0;
	}

	char buffer[][100] = {"is", "This", "awesome", "as", "fuck"};

	input = DATA_TYPE_STRING;
	
	ret = ioctl(fd, PB2_SET_TYPE, &input);
	if(ret < 0) perror("ioctl failed\n");

	input = GRAPH_IN_ORDER;
	ret = ioctl(fd, PB2_SET_ORDER, &input);
	if(ret < 0) perror("ioctl failed\n");


	len = 0;


	while(len < 5)
	{
		number = rand() % 1000;
		ret = write(fd, buffer[len], sizeof(int32_t));
		
		if(ret < 0)
		{
			perror("write failed\n");
		}
		
		len++;
	}

	len = 0 ;

	char tempbuffer[100];
	while(len < 5)
	{
		len++;

		ret = read(fd, tempbuffer, sizeof(char) * 100);

		if(ret < 0)
		{
			perror("read failed\n");
		}

		printf("%s ", tempbuffer);

		memset(tempbuffer, 0, sizeof(char) * 100);
	}

	printf("\n");
	return 0;
}
