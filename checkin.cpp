#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check in> <password>\n\n", av[0]);
		return -1;
	}
    initFS("part.dsk",av[2]);
    unsigned int fp = openFile("test.txt",0);
    char * buffer = "check in ";
    writeFile(fp, buffer,5,0);
    closeFile(fp);
	return 0;
}
