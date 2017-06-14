#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#define FREE 0
#define ALLOCATED 1
#define NOT_IMPLEMENTED -1
#define ERROR -2
#define BOOT_SECTOR 0
#define MFT_FIRST_SECTOR 4 
#define MFT_ROOT_DIRECTORY_FIRST_SECTOR 6
#define MTF_TUPLES_PER_RECORD 32
#define MTF_RECORD_SIZE 512 //each MFT record consists of 32 tuples of 16 bytes
#define MFT_LAST_SECTOR 8196 //2048 blocks * 4 sector per block + 4
//16 bytes per tuple
//32 tuples per sector (256/8)
//128 tuples per block (32*4 sectors per block)
//blocks of 1KiB
struct  t2fs_4tupla** rootMTFRecord; //array of 32 tuples
struct t2fs_bootBlock* block;
BYTE buffer[SECTOR_SIZE] = {0};   

//prototypes
void printMFTTuple(struct t2fs_4tupla* record);

//------------Auxiliary functions--------------------------
WORD getWord(char lsb, char msb) {
    return (WORD) lsb | (msb << 8);
}
DWORD getDWord(char b1, char b2, char b3, char b4) {
    return (DWORD) b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}
BYTE* wordToBytes(WORD word) {
    BYTE *bytes = malloc(2*sizeof(*bytes));
    bytes[0] =  word & 0x00FF;
    bytes[1] = (word & 0xFF00) >> 8;
    return bytes;
}

BYTE* dwordToBytes(DWORD dword) {
    BYTE *bytes = malloc(4*sizeof(*bytes));
    bytes[0] =  dword & 0x00FF;
    bytes[1] = (dword & 0xFF00) >> 8;
    bytes[2] = (dword & 0xFF0000) >> 16;
    bytes[3] = (dword & 0xFF000000) >> 24;
    return bytes;
}
//----------------------------------------------------------
/**
* Assumes buffer already has the necessary content. 
* REMEMBER to read_sector first!
*/
struct t2fs_4tupla* readMFTtuple(int tupleNumber) {
    struct t2fs_4tupla* tuple = malloc(sizeof(*tuple));
    int index = tupleNumber*sizeof(*tuple);
    tuple->atributeType = getDWord(buffer[index],buffer[index+1],buffer[index+2],buffer[index+3]);
    tuple->virtualBlockNumber = getDWord(buffer[index+4],buffer[index+5],buffer[index+6],buffer[index+7]);
    tuple->logicalBlockNumber = getDWord(buffer[index+8],buffer[index+9],buffer[index+10],buffer[index+11]);
    tuple->numberOfContiguosBlocks = getDWord(buffer[index+12],buffer[index+13],buffer[index+14],buffer[index+15]);
    return tuple;
}
struct t2fs_4tupla** readMFTRecord(int sector) {    
    if(read_sector(sector, buffer) != 0) {
        printf("Error reading sector for MFT record\n");
    }
    //A record is formed by two sectors, so they must be processed separately
    struct t2fs_4tupla** record = malloc(sizeof(struct t2fs_4tupla)*MTF_TUPLES_PER_RECORD);
    int i,j;
    for(i = 0; i < MTF_TUPLES_PER_RECORD/2; i++) {
        record[i] = readMFTtuple(i);
    }
    if(read_sector(sector+1, buffer) != 0) {
        printf("Error reading sector for MFT record\n");
    }
    for(j = 0; i < MTF_TUPLES_PER_RECORD; i++,j++) {
        printf("i %d j %d\n", i,j);
        record[i] = readMFTtuple(j);
    }
    return record;
}
struct t2fs_record* readFileRecord(int recordNumber) {
    struct t2fs_record* record = malloc(sizeof(struct t2fs_record*));
    int index = recordNumber*sizeof(*record);
    record->TypeVal = buffer[index];
    memcpy(record->name, buffer+index+1, MAX_FILE_NAME_SIZE);
    index += MAX_FILE_NAME_SIZE + 1;
    record->blocksFileSize = getDWord(buffer[index],buffer[index+1],buffer[index+2],buffer[index+3]);
    record->bytesFileSize = getDWord(buffer[index+4],buffer[index+5],buffer[index+6],buffer[index+7]);
    record->MFTNumber = getDWord(buffer[index+8],buffer[index+9],buffer[index+10],buffer[index+11]);
    return record;
}

void writeMTFRecord(struct t2fs_4tupla* record, int recordNumber, unsigned int sector) {
    int index = sizeof(*record) * recordNumber;
    memcpy(buffer+index, dwordToBytes(record->atributeType), 4);
    index+=4;
    memcpy(buffer+index, dwordToBytes(record->virtualBlockNumber), 4);
    index+=4;
    memcpy(buffer+index, dwordToBytes(record->logicalBlockNumber), 4);
    index+=4;
    memcpy(buffer+index, dwordToBytes(record->numberOfContiguosBlocks), 4);
    write_sector(sector, buffer);
}
void readRootDirectoryRecord() {    
    //root directory descriptor
    rootMTFRecord = readMFTRecord(MFT_ROOT_DIRECTORY_FIRST_SECTOR); 
    /*
    if(reserved[1]->atributeType != 1) { //root directory wasn't created, so we must create it
        reserved[1]->atributeType = 1;
        reserved[1]->virtualBlockNumber = 0;
        int freeBlock = searchBitmap2(FREE);
        setBitmap2(freeBlock, ALLOCATED);
        reserved[1]->logicalBlockNumber = freeBlock;
        reserved[1]->numberOfContiguosBlocks = 1;
        writeMTFRecord(reserved[1], 1, MFT_FIRST_SECTOR);
    } else {
        printf("Root was already created\n");
    }*/

}


void readBootBlock() {
    block = malloc(sizeof(*block));     
    if(read_sector(BOOT_SECTOR, buffer) != 0) {
        printf("Error reading boot sector\n");
        return;
    }
    if(!block) {
        printf("Malloc error\n");
    }
    memcpy(block->id,buffer,4);
    block->version = getWord(buffer[4],buffer[5]);
    block->blockSize = getWord(buffer[6],buffer[7]);
    block->MFTBlocksSize = getWord(buffer[8],buffer[9]);
    block->diskSectorSize = getDWord(buffer[10],buffer[11],buffer[12],buffer[13]);
}
void printMFTTuple(struct t2fs_4tupla* tuple) {
    printf("Tuple MTF: \n");
    printf("Attribute type:              %04x hex %d dec\n", tuple->atributeType, tuple->atributeType);
    printf("Virtual block number:        %04x hex %d dec\n", tuple->virtualBlockNumber, tuple->virtualBlockNumber);
    printf("Logical block number:        %04x hex %d dec\n", tuple->logicalBlockNumber, tuple->logicalBlockNumber);
    printf("Number of contiguous blocks: %04x hex %d dec\n", tuple->numberOfContiguosBlocks, tuple->numberOfContiguosBlocks);
}
int main() {
    readBootBlock();
    readRootDirectoryRecord();    
    printf("Root directory MTF: \n");
    int i;
    for (i = 0; i < 32; ++i) {
        printMFTTuple(rootMTFRecord[i]);
    }
    read_sector(rootMTFRecord[0]->logicalBlockNumber, buffer);
    
    return 0;
}

int identify2 (char *name, int size) {
    char* names = "Christian Schmitz 242258\nIgor Ferreira 242267\n";
    if(strlen(names) > size) {
        return ERROR;
    }
    strncat(name,names, size);
    return 0;
}
FILE2 create2 (char *filename) {return NOT_IMPLEMENTED;}
int delete2 (char *filename) {return NOT_IMPLEMENTED;}
FILE2 open2 (char *filename) {return NOT_IMPLEMENTED;}
int close2 (FILE2 handle){return NOT_IMPLEMENTED;}
int read2 (FILE2 handle, char *buffer, int size){return NOT_IMPLEMENTED;}
int write2 (FILE2 handle, char *buffer, int size){return NOT_IMPLEMENTED;}
int truncate2 (FILE2 handle){return NOT_IMPLEMENTED;}
int seek2 (FILE2 handle, DWORD offset){return NOT_IMPLEMENTED;}
int mkdir2 (char *pathname){return NOT_IMPLEMENTED;}
int rmdir2 (char *pathname){return NOT_IMPLEMENTED;}
DIR2 opendir2 (char *pathname){return NOT_IMPLEMENTED;}    
 int readdir2 (DIR2 handle, DIRENT2 *dentry){return NOT_IMPLEMENTED;}
int closedir2 (DIR2 handle){return NOT_IMPLEMENTED;}
