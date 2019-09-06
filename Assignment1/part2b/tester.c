#include <stdio.h>
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

	
	printf("setting order and type for ioctl interface\n");

	ioctl(fd, PB2_SET_TYPE, DATA_TYPE_INT);
	ioctl(fd, PB2_SET_ORDER, GRAPH_IN_ORDER);

	while(len)
	{
		number = rand() % 1000;
		write(fd, &number, sizeof(int32_t));
		len--;
	}


	close(fd);	

	return 0;
}
