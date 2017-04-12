#include "libefs.h"

int main(int ac, char **av)
{
    printf("main\n");
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check out> <password>\n\n", av[0]);
		return -1;
	}

    printf("before init FS\n");
    initFS("part.dsk",av[2]);
    printf("after init fs\n");
    unsigned int fp = openFile("test.txt",0);
    printf("after open file\n");
    char* buffer1;
    buffer1 = (char*)malloc(30);
    readFile(fp, buffer1, 10, 0);
    printf("checkout main\n");
    printf("buffer1: %s\n",buffer1);
    char* buffer2;
    buffer2 = (char*)malloc(10);
    readFile(fp,buffer2,10,0);
    printf("buffer2: %s\n",buffer2);
	return 0;
}
