#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 2)
	{
		printf("\nUsage: %s <file to check>\n", av[0]);
		printf("Prints: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
    }

    initFS("part.dsk","password");

    unsigned int attr = getattr(av[1]);

    if (attr==FS_FILE_NOT_FOUND){
        printf("File not found\n");
        exit(-1);
    }

    unsigned readonly = (attr>>2)&1;
    if(readonly==1){
        printf("R\n");
    }else{
        printf("W\n");
    }
    closeFS();
	return 0;
}
