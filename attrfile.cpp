#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check set attrbute> <attribute>\n", av[0]);
		printf("Attribute: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
	}

    initFS("part.dsk","password");
    
    unsigned int attr = getattr(av[1]);
    if (attr==FS_FILE_NOT_FOUND){
        printf("FILE NOT FOUND\n");
        exit(-1);
    }
    printf("%d\n",attr);
    char *R = "R";
    char *W = "W";
    if (strcmp(av[2],R)==0){

        attr = attr | 0b100;
        printf("R: %d\n",attr);
        setattr(av[1], attr);
    }else if (strcmp(av[2],W)==0){
        attr = attr & 0b011;
        printf("W: %d\n", attr);
        setattr(av[1],attr);
    }else{
		printf("\nUsage: %s <file to check set attrbute> <attribute>\n", av[0]);
		printf("Attribute: 'R' = Read only, 'W' = Read/Write\n\n");
    }
    
    closeFS();
	return 0;
}
