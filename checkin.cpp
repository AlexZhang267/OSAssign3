#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check in> <password>\n\n", av[0]);
		return -1;
	}
	
    FILE *fp = fopen(av[1],"r");
    
    if(fp==NULL){
        printf("\nUnable to open source file%s\n\n",av[1]);
        exit(-1);
    }

    initFS("part.dsk",av[2]);

    unsigned int fp2 = openFile(av[1],MODE_CREATE);

    if (fp2==-1){
        exit(-1);
    }

    fseek(fp,0,SEEK_END);
    unsigned int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    //can malloc any size buffer
    char * buffer = (char*)malloc(sizeof(char)*len);
    
    printf("before while\n");
    fread(buffer, sizeof(char), len, fp);
    /*int size;
    while((size=fread(buffer, 1, 500, fp))==500){
        printf("in whilie\n");
        writeFile(fp2, buffer, sizeof(char), 500);
    }
    printf("after while size is %d\n", size);
    */
    printf("\n\n%s\n\n",buffer);
    writeFile(fp2,buffer,sizeof(char),len);
    
    fclose(fp);
    closeFile(fp2);
    closeFS();
    return 0;
}
