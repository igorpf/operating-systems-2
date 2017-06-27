#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils.h"
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

struct  t2fs_4tupla** rootMFTRecord; //array of 32 tuples
struct t2fs_bootBlock* block;
BYTE buffer[SECTOR_SIZE] = {0};   
struct openFileRegister **openFiles;

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
int blockToSector(int block){
    return block*4;
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
struct t2fs_4tupla** readMFTRecord(int recordNumber) {
    int sector = MFTRecordToSector(recordNumber);
    if(read_sector(sector, buffer) != 0) {
        printf("Error reading sector for MFT record\n");
    }
    //A record is formed by two sectors, so they must be processed separately
    struct t2fs_4tupla** record = malloc(sizeof(struct t2fs_4tupla)*MFT_TUPLES_PER_RECORD);
    int i,j;
    for(i = 0; i < MFT_TUPLES_PER_RECORD/2; i++) {
        record[i] = readMFTtuple(i);
    }
    if(read_sector(sector+1, buffer) != 0) {
        printf("Error reading sector for MFT record\n");
    }
    for(j = 0; i < MFT_TUPLES_PER_RECORD; i++,j++) {
        record[i] = readMFTtuple(j);
    }
    return record;
}
struct t2fs_record* readFileRecord(int recordNumber) {
    struct t2fs_record* record = malloc(sizeof(*record));
    int index = recordNumber*sizeof(*record);
    record->TypeVal = buffer[index];
    memcpy(record->name, buffer+index+1, MAX_FILE_NAME_SIZE);
    index += MAX_FILE_NAME_SIZE + 1;
    record->blocksFileSize = getDWord(buffer[index],buffer[index+1],buffer[index+2],buffer[index+3]);
    record->bytesFileSize = getDWord(buffer[index+4],buffer[index+5],buffer[index+6],buffer[index+7]);
    record->MFTNumber = getDWord(buffer[index+8],buffer[index+9],buffer[index+10],buffer[index+11]);
    return record;
}
void writeMFTRecord(struct t2fs_4tupla* record, int recordNumber, unsigned int sector) {
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
void printFileRecord(struct t2fs_record* fileRecord) {
    printf("TypeVal:        %d\n", fileRecord->TypeVal);
    printf("Name:           %s\n", fileRecord->name);
    printf("blocksFileSize: %d\n", fileRecord->blocksFileSize);
    printf("bytesFileSize:  %d\n", fileRecord->bytesFileSize);
    printf("MFTNumber:      %d\n", fileRecord->MFTNumber);
}
void readRootDirectoryRecord() {    
    //root directory descriptor
    rootMFTRecord = readMFTRecord(1); 
    printMFTTuple(rootMFTRecord[0]);
   printMFTTuple(rootMFTRecord[1]); 
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
    printf("Tuple MFT: \n");
    printf("Attribute type:              %04x hex %d dec\n", tuple->atributeType, tuple->atributeType);
    printf("Virtual block number:        %04x hex %d dec\n", tuple->virtualBlockNumber, tuple->virtualBlockNumber);
    printf("Logical block number:        %04x hex %d dec\n", tuple->logicalBlockNumber, tuple->logicalBlockNumber);
    printf("Number of contiguous blocks: %04x hex %d dec\n", tuple->numberOfContiguosBlocks, tuple->numberOfContiguosBlocks);
}
int strCount(const char* str, const char c) {
    int count = 0;
    while(*str != '\0') {
        if(*str++==c)
            count++;
    }
    return count;
}
/**
*    Checks if there is an empty entry in the openFiles array.
*    returns a new register if there is, NULL otherwise
*/
struct openFileRegister* getNewFileRegister(struct t2fs_record* fileRecord) {
    if(!openFiles) {
        openFiles = malloc(sizeof(*openFiles) * MAX_OPEN_FILES);
    }
    int i;
    for(i = 0; i < MAX_OPEN_FILES; i++) {
        if(!openFiles[i]) { //free slot
            openFiles[i] = malloc(sizeof(*openFiles[i]));
            openFiles[i]->fileRecord = fileRecord;
            openFiles[i]->currentPointer = 0;
            openFiles[i]->handle = i;
            return openFiles[i];
        }
    }
    //open files limit exceeded
    return NULL;
}
int MFTRecordToSector(int recordNumber) {
    // return recordNumber * 2 + MFT_FIRST_VALID_SECTOR;
    return recordNumber * 2 + MFT_FIRST_SECTOR;
}