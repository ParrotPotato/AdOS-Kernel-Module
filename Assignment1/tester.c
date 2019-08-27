#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

const char * FILENAME = "/proc/temp_process_entry";
const char SORT_TYPE_INT = 0xff;
const char SORT_TYPE_STRING = 0xf0;

int main(){
	srand((unsigned)time(NULL));
	printf("Opening file\n");

	int fd = open(FILENAME, O_RDWR);

	if(fd < 0)
	{
		perror("Unable to open file\n");
		return -1;
	}
	
	int len = 5;

	unsigned char info[2];
	info[0] = (char)SORT_TYPE_INT;
	info[1] = len;

	printf("Writing info to file\n");
	printf("buffer[0] = %d\n", info[0]);
	printf("buffer[1] = %d\n", info[1]);

	write(fd, info, 2);

	printf("Writing info to file\n");
	int32_t temp = 0  ;

	for(int32_t i=0 ; i < 5; i++){

		temp = rand() % 1001;
		printf("%d ", temp);
		write(fd, &temp, sizeof(int32_t));
	}

	printf("\nReading from file\n");	

	for(int32_t i=0 ; i < 5; i++){

		read(fd, &temp, sizeof(int32_t));
		printf("%d ", temp);
	}
	
	printf("Closing File\n");	
	
	close(fd);

	fd = open(FILENAME, O_RDWR);

	if(fd < 0)
	{
		perror("Unable to open file \n");
		return -1;
	}
	
	info[0] = SORT_TYPE_STRING;
	info[1] = len;

	char buffer[100];

	write(fd, info, 2);

	printf("Enter strings \n");
	
	for(int i=0 ; i < len ; i++)
	{
		scanf("%s", buffer);
		write(fd, buffer, strlen(buffer) + 1);
	}
	
	printf("Reading strings\n");
	
	printf("%c",'\n');
	for(int i=0 ; i < len ; i++)
	{
		read(fd, buffer, 100);
		printf("%s ", buffer);
	}

	return 0;
}
