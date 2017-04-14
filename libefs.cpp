#include "libefs.h"

#define MAX_OFT_ENTRY_NUM 1024
// FS Descriptor
TFileSystemStruct *_fs;

// Open File Table
TOpenFile *_oft;

// Open file table counter
int _oftCount=0;

// Mounts a paritition given in fsPartitionName. Must be called before all
// other functions
void initFS(const char *fsPartitionName, const char *fsPassword)
{
    mountFS(fsPartitionName, fsPassword);
    _oft = (TOpenFile *) calloc(sizeof(TOpenFile),MAX_OFT_ENTRY_NUM);
    _fs = getFSInfo();
}

// Opens a file in the partition. Depending on mode, a new file may be created
// if it doesn't exist, or we may get FS_FILE_NOT_FOUND in _result. See the enum above for valid modes.
// Return -1 if file open fails for some reason. E.g. file not found when mode is MODE_NORMAL, or
// disk is full when mode is MODE_CREATE, etc.

int openFile(const char *filename, unsigned char mode)
{
    unsigned int fileNdx = findFile(filename);
   // printf("openFile file index %d filename %s\n",fileNdx,filename);
    if (fileNdx == FS_FILE_NOT_FOUND){
        if (mode == MODE_NORMAL || mode == MODE_READ_ONLY){
            _result = FS_FILE_NOT_FOUND;
            printf("FILE NOT FOUND!\n");
            return -1;
        }else if(mode == MODE_CREATE || MODE_READ_APPEND){
            //create the file.
            fileNdx = makeDirectoryEntry(filename, 0x0, 0);
            //printf("file not found, mode is create\n");
            if (fileNdx == FS_DIR_FULL){
                //disk is full
                _result = FS_DIR_FULL;
                return -1;
            }
        }
    }else{
        if (mode==MODE_NORMAL || mode == MODE_READ_APPEND){
            unsigned int attr = getattr(filename);
            //attr can be found
            unsigned int readonly = (attr >> 2) & 0b1;
            if (readonly){
                printf("This file is read only file!\n");
                _result = FS_ERROR;
                return -1;;
            }
        }else if(mode == MODE_CREATE){
            printf("FILE DUPLICATE\n");
            _result = FS_DUPLICATE_FILE;
            return -1;
        }
    }


    //operate to OFT
    unsigned int ndx = getFreeOFTEntry();
    _oft[ndx].openMode = mode;
    _oft[ndx].blockSize = _fs->blockSize;     //why do we need the block size in OFT?
    _oft[ndx].inode = fileNdx;
    _oft[ndx].inodeBuffer = makeInodeBuffer();
    loadInode(_oft[ndx].inodeBuffer,fileNdx);
    _oft[ndx].buffer = makeDataBuffer();
    _oft[ndx].writePtr= 0;
    _oft[ndx].readPtr = 0;
    _oft[ndx].filePtr = 0;


    _oft[ndx].currentBlockNum = 0;
    //initial buffer when open
    unsigned fileLength = getFileLength(filename);
   
    _oft[ndx].fileLength = fileLength;
    
	return ndx;
}

// Write data to the file. File must be opened in MODE_NORMAL or MODE_CREATE modes. Does nothing
// if file is opened in MODE_READ_ONLY mode.
void writeFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{

    if(fp==-1){
        exit(-1);
    }


    dataSize = dataSize * dataCount;


    _oft[fp].filePtr = 0;

    if(_oft[fp].fileLength==0){
        //new create file
        unsigned long freeBlock = findFreeBlock();
        markBlockBusy(freeBlock);
        setBlockNumInInode(_oft[fp].inodeBuffer,0,freeBlock);
        _oft[fp].currentBlockNum = freeBlock;
        //memset(_oft[fp].buffer, blockNum);
        memset(_oft[fp].buffer,0,_oft[fp].blockSize);
        readBlock(_oft[fp].buffer, freeBlock);
    }else{
        unsigned long blockNum = 0;
        if (MODE_READ_APPEND){
            blockNum = returnBlockNumFromInode(_oft[fp].inodeBuffer, _oft[fp].fileLength);
        }else{
            blockNum = returnBlockNumFromInode(_oft[fp].inodeBuffer, 0);
        }
         _oft[fp].currentBlockNum = blockNum;
        memset(_oft[fp].buffer,0,_oft[fp].blockSize);
        readBlock(_oft[fp].buffer, blockNum);
    }

    //check access
    if(_oft[fp].openMode == MODE_READ_ONLY){
        printf("CAN NOT WRITE IN READ ONLY MODE\n");
        return;
    }else if(_oft[fp].openMode == MODE_READ_APPEND){ 
        _oft[fp].filePtr = _oft[fp].fileLength;
    }
  
    //update writePtr, fileptr maybe modified by read
    _oft[fp].writePtr = _oft[fp].filePtr % _oft[fp].blockSize;

    //printf("file ptr%d,  writeptr:%d\n",_oft[fp].filePtr, _oft[fp].writePtr);
    unsigned int bufferOffset = 0;
    while(dataSize % _oft[fp].blockSize != 0){
        if (dataSize + _oft[fp].writePtr > _oft[fp].blockSize){
            //printf("while > if buffer offset is %d dataSize is %d, writePtr is %d\n", bufferOffset, dataSize,_oft[fp].writePtr );
            memcpy(_oft[fp].buffer + _oft[fp].writePtr, (char*) buffer + bufferOffset, _oft[fp].blockSize - _oft[fp].writePtr);
            //printf("after memcpy");
            bufferOffset += _oft[fp].blockSize - _oft[fp].writePtr;
            _oft[fp].filePtr += _oft[fp].blockSize - _oft[fp].writePtr;

            if(_oft[fp].filePtr > _oft[fp].fileLength){
                _oft[fp].fileLength = _oft[fp].filePtr;
            }

            dataSize -= _oft[fp].blockSize - _oft[fp].writePtr;
            _oft[fp].writePtr = 0;

            //Flush
            flushFile(fp);

            //load another block
            unsigned long freeBlock = findFreeBlock();
            markBlockBusy(freeBlock);
            printf("load another block, freeblock is %lu, fileptr is %d\n",freeBlock,_oft[fp].filePtr);
            setBlockNumInInode(_oft[fp].inodeBuffer,_oft[fp].filePtr, freeBlock);
            memset(_oft[fp].buffer,0,_oft[fp].blockSize);
            readBlock(_oft[fp].buffer, freeBlock);
            _oft[fp].currentBlockNum = freeBlock;
       

        }else{
            printf("while >  else\n");
            memcpy(_oft[fp].buffer + _oft[fp].writePtr, (char*) buffer + bufferOffset, dataSize);
            bufferOffset += dataSize;

            _oft[fp].filePtr += dataSize;
            _oft[fp].writePtr += dataSize;
            if(_oft[fp].filePtr  > _oft[fp].fileLength){
                _oft[fp].fileLength = _oft[fp].filePtr;
            }

            dataSize = 0;
            flushFile(fp);
        }

    }
    printf("file length is %d\n", _oft[fp].fileLength);

}

// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp)
{
    //flush to disk
   
    /*
    if(_oft[fp].currentBlockNum==0){
        //this is a new create file, have not allocate block to it
       // printf("flushFile allocate new block");
        unsigned long freeBlock = findFreeBlock();
        markBlockBusy(freeBlock);
        setBlockNumInInode(_oft[fp].inodeBuffer,0,freeBlock);
        _oft[fp].currentBlockNum = freeBlock;

    }*/
    //unsigned long bckNum = returnBlockNumFromInode(_oft[fp].inodeBuffer,_oft[fp].filePtr);
    writeBlock(_oft[fp].buffer, _oft[fp].currentBlockNum);
    printf("flushFile: fileptr: %d,blockNum:%lu\n",_oft[fp].filePtr, _oft[fp].currentBlockNum);
    
   // printf("flushFile updateDirectory\n");
   //freeOFTEntry(fp);
    printf("FLUSH: %s\n\n",_oft[fp].buffer);
}

void readFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
    //Todo: dont consider this situation: dataSize + readptr > file length
    

    //TOpenFile fileEntry = _oft[fp];

    if(fp==-1){
        exit(-1);
    }



    _oft[fp].filePtr = 0;
    _oft[fp].readPtr = 0;
    if(_oft[fp].fileLength>0){
        unsigned long blockNum = returnBlockNumFromInode(_oft[fp].inodeBuffer, 0);
         _oft[fp].currentBlockNum = blockNum;
        memset(_oft[fp].buffer,0,_oft[fp].blockSize);
        readBlock(_oft[fp].buffer, blockNum);
    }


    dataSize = dataSize * dataCount;
    


    //fix, file length is not given in OFT
    if(dataSize > _oft[fp].fileLength){
        dataSize = _oft[fp].fileLength;
    }
    
    //dont need to check access, all four mode can read

    printf("readFile: dataSize is %d, readPtr is %d\n",dataSize, _oft[fp].readPtr);
    unsigned int bufferOffset = 0;
    while(dataSize%_oft[fp].blockSize != 0){
  
        printf("datasize %d, readptr%d, blocksize %d",dataSize, _oft[fp].readPtr, _oft[fp].blockSize);
        if(dataSize + _oft[fp].readPtr  > _oft[fp].blockSize){
            printf("while > if\n");
            memcpy((char *)buffer+bufferOffset, _oft[fp].buffer +_oft[fp].readPtr, _oft[fp].blockSize - _oft[fp].readPtr);
                    
            bufferOffset += _oft[fp].blockSize-_oft[fp].readPtr;
            _oft[fp].filePtr += _oft[fp].blockSize - _oft[fp].readPtr;

            dataSize -= _oft[fp].blockSize - _oft[fp].readPtr;
            _oft[fp].readPtr = 0;
            
            //todo: load another block
            //todo: test it
            memset(_oft[fp].buffer,0,_oft[fp].blockSize);
            unsigned int blockNum = returnBlockNumFromInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);
            printf("readFile: blockNum: %lu fileptr: %d\n",blockNum, _oft[fp].filePtr);
            readBlock(_oft[fp].buffer, blockNum);
        }else{
            printf("while > else\n");
            memcpy((char *)buffer+bufferOffset, _oft[fp].buffer + _oft[fp].readPtr, dataSize);
            
            bufferOffset += dataSize;
            _oft[fp].filePtr += dataSize;
            _oft[fp].readPtr += dataSize;
            dataSize = 0;
        }
      //  printf("after read, fileEntry.readPtr is %d\n", _oft[fp].readPtr);
    }

      //  printf("%s",buffer);
}

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set. 
// See TDirectory structure.
void delFile(const char *filename){
    unsigned int ndx = findFile(filename);
    printf("delFile file index %d file name %s\n",ndx, filename);
    if(ndx==FS_FILE_NOT_FOUND){
        printf("FILE NOT FOUND\n");
        exit(-1);
    }
   
    printf("before load inode\n");
    unsigned long *inode = makeInodeBuffer();
    loadInode(inode, ndx);
    printf("after load inode\n");
    int blockSize = _fs-> blockSize;
    printf("after get blocksize\n");
    int num = _directory[ndx].length / (_fs->blockSize);

    printf("delFile num %d",num);
    for(int i=0; i<= num;i++){
        printf("del file, block num is :%lu\n",inode[i]);
        markBlockFree(inode[i]);
    }

    delDirectoryEntry(filename);
}

// Close a file. Flushes all data buffers, updates inode, directory, etc.
void closeFile(int fp){
    if(fp > -1){

        if(_oft[fp].openMode!=MODE_READ_ONLY){
            flushFile(fp);
        }
        //release buffer
        free(_oft[fp].buffer);

        saveInode(_oft[fp].inodeBuffer,_oft[fp].inode);
       
        //update file length
        unsigned int fileLength = _directory[_oft[fp].inode].length;
        if (_oft[fp].fileLength > fileLength){
            _directory[_oft[fp].inode].length = _oft[fp].fileLength;
            updateDirectory(); 
        }
        
        freeOFTEntry(fp);
    }
}

// Unmount file system.
void closeFS(){
    updateDirectory();
    unmountFS();
    free(_oft);
}


//return index to next free oft entry
unsigned int getFreeOFTEntry(){
    unsigned int i;
    for(i=0; i < MAX_OFT_ENTRY_NUM;i++){
        if (_oft[i].inode==0){
            return i;
        }
    }
    // if return this value, then there is no storage in oft
    return MAX_OFT_ENTRY_NUM;
}

void freeOFTEntry(unsigned int index){
    _oft[index].inode = 0;
}
