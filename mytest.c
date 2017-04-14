/*************************************************************************
	> File Name: mytest.c
	> Author: 
	> Mail: 
	> Created Time: Tue Apr 11 19:52:23 2017
 ************************************************************************/

#include"libefs.h"

void testOpen();
void testWrite();
void testRead();
void testWriteMulBlock();
void testReadMulBlock();

int main(){
 //   testOpen();
 
    testWrite();
 //   testRead();
  /*  printf("new");
    testWriteMulBlock();
    testReadMulBlock();
    printf("after testOpen\n");
    */
    
    return 0;
}


void testWriteMulBlock(){
    initFS("part.dsk","1122");
    FILE *fp = fopen("../testfile.txt","r");
    char buffer[500];
    unsigned int fpp = openFile("testfile8.txt",MODE_CREATE);
    int res = 0;
    do{
        res = fread(buffer,1,500,fp);
        //printf("%s",buffer);
        writeFile(fpp,buffer,500,0);
    }while(res==500);
    fclose(fp);
    closeFile(fpp);
    closeFS();
}

void testReadMulBlock(){
    initFS("part.dsk","1122");
    unsigned int fpp = openFile("testfile8.txt",MODE_READ_ONLY);
    char buffer[500];
    readFile(fpp,buffer,500,0);
    FILE *fp = fopen("testReadMulBlock10.txt","w+");
    fwrite(buffer,1,500,fp);
    fclose(fp);
    closeFile(fpp);
    closeFS();
    
}

void testWrite(){
    initFS("part.dsk","1122");
    unsigned int fp;

    //test write in read only mode
    //pass
    char *filename = "testfile.txt";
    fp = openFile("testfile.txt",MODE_READ_ONLY);
    char * buffer = "testWrite";
    writeFile(fp,buffer,5,2);
    closeFile(fp);

    printf("sizeof(char): %lu\n\n", sizeof(char));

    fp = openFile("testfile.txt", MODE_NORMAL);
    char * buffer2 = "test write mode normal";
    printf("buffer2 size %lu\n",sizeof(buffer2));
    writeFile(fp, buffer2, sizeof(char), 22);
    closeFile(fp);

    fp = openFile("testfile.txt", MODE_READ_APPEND);
    char * buffer3 = "test write read append";

    printf("buffer3 size %lu\n",sizeof(buffer3));
    writeFile(fp, buffer3, sizeof(char), 22);
    closeFile(fp);

    unsigned int len = getFileLength(filename);
    printf("\n\n len: %d \n\n", len);
    char *buffer4 = (char*)malloc(sizeof(char)*len);
    fp = openFile(filename, MODE_READ_ONLY);
    readFile(fp, buffer4, sizeof(char), len);
    closeFile(fp);

    printf("\n\nContent:\n%s\n", buffer4);
    //test create
    //create newfile.txt write to it, then use testRead can read it, but file length is wrong
    fp = openFile("newFile.txt",MODE_CREATE);
    buffer = "testWrite to new file in create mode";
    writeFile(fp,buffer,100,0);
    closeFile(fp);

    closeFS();
}

void testRead(){
    initFS("part.dsk","112233");
    unsigned int fp;

    fp = openFile("newFile.txt",MODE_READ_ONLY);
    char* buffer = (char*)malloc(100);
    readFile(fp, buffer, 100, 0);
    printf("testRead:\n%s",buffer);
    closeFile(fp);
    closeFS();
}

//ok
void testOpen(){
    initFS("part.dsk","1122");
    unsigned int fp = openFile("unexistfile.txt",MODE_NORMAL);
    if(fp != -1 || _result!=FS_FILE_NOT_FOUND){
        printf("testOPen test1 failed\n");
    }
    closeFile(fp);
    printf("fp is %d\n",fp);
    fp = openFile("test_open_file2.txt",MODE_CREATE);
    if(fp==-1){
        printf("testOpen test2 failed\n");
    }
    printf("fp is %d\n",fp);
    closeFile(fp);
}

