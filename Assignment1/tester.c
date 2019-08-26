#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>

const char * FILENAME = "/proc/temp_process_entry";

int main(){
	printf("Opening file\n");

	int fd = open(FILENAME, O_RDWR);

	if(fd < 0)
	{
		perror("Unable to open file\n"); 
	}
	
	printf("Closing file\n");

	close(fd);

	return 0;
}
