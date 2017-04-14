#include "libefs.h"

int main(int ac, char **av)
{
    printf("main\n");
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check out> <password>\n\n", av[0]);
		return -1;
	}

    initFS("part.dsk",av[2]);

    char *buffer;

    FILE *fp = fopen(av[1],"w");
    
    int fp2 = openFile(av[1],MODE_NORMAL);

    if (fp2==-1){
        exit(-1);
    }

    unsigned int len = getFileLength(av[1]);

    buffer = (char*)malloc(sizeof(char) * len);
    
    readFile(fp2, buffer, sizeof(char), len);
    fwrite(buffer, sizeof(char), len, fp);
    
    //we should not allocate a buffer with len, because maybe len is very large
    //so there use a loop to read
    /*int readSize = 0;
    for(int i = 0; i<len; i += bufferSize){
        if (len - i < bufferSize){
            readSize = len - i;
        }else{
            readSize = bufferSize;
        }
        readFile(fp2, buffer, sizeof(char), bufferSize);
        fwrite(buffer, sizeof(char), readSize, fp);
    }*/
    
    closeFile(fp2);
    fclose(fp);
    closeFS();
	return 0;
}
