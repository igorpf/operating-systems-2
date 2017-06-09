#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#define BOOT_SECTOR 0
#define MFT_FIRST_SECTOR 4 //
#define MFT_LAST_SECTOR 8196 //2048 blocks * 4 sector per block + 4
//16 bytes per tuple
//32 tuples per sector (256/8)
//64 tuples per block (16*4 sectors per block)
//blocks of 1KiB
struct  t2fs_4tupla* reserved[4] ;
struct t2fs_bootBlock* block;
BYTE buffer[SECTOR_SIZE] = {0};    

//assuming little-endian information
WORD getWord(char lsb, char msb) {
    return (WORD) lsb | (msb << 8);
}
DWORD getDWord(char b1, char b2, char b3, char b4) {
    return (DWORD) b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}
/**
* Assumes buffer already has the necessary content. 
* REMEMBER to read_sector first!
*/
struct t2fs_4tupla* readMFTrecord(int recordNumber) {
    struct t2fs_4tupla* record = malloc(sizeof(*record));
    int index = recordNumber*16;
    record->atributeType = getDWord(buffer[index],buffer[index+1],buffer[index+2],buffer[index+3]);
    record->virtualBlockNumber = getDWord(buffer[index+4],buffer[index+5],buffer[index+6],buffer[index+7]);
    record->logicalBlockNumber = getDWord(buffer[index+8],buffer[index+9],buffer[index+10],buffer[index+11]);
    record->numberOfContiguosBlocks = getDWord(buffer[index+12],buffer[index+13],buffer[index+14],buffer[index+15]);
    return record;
}
void readReservedMFT() {
    if(read_sector(MFT_FIRST_SECTOR, buffer) != 0) {
        printf("Error reading MFT sector\n");
        return;
    }
    //bitmap descriptor
    reserved[0] = readMFTrecord(0); 
    //root directory descriptor
    reserved[1] = readMFTrecord(1); 
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
void printMFTRecord(struct t2fs_4tupla* record) {
    printf("Record MTF: \n");
    printf("Attribute type:              %04x hex %d dec\n", record->atributeType, record->atributeType);
    printf("Virtual block number:        %04x hex %d dec\n", record->virtualBlockNumber, record->virtualBlockNumber);
    printf("Logical block number:        %04x hex %d dec\n", record->logicalBlockNumber, record->logicalBlockNumber);
    printf("Number of contiguous blocks: %04x hex %d dec\n", record->numberOfContiguosBlocks, record->numberOfContiguosBlocks);
}
int main() {
    readBootBlock();
    readReservedMFT();
    // printf("%s\n", block->id);
    // printf("%04x\n", block->version);
    // printf("%04x\n", block->blockSize);
    // printf("%04x\n", block->MFTBlocksSize);
    // printf("%04x\n", block->diskSectorSize);
    printf("Bitmap descriptor MTF: \n");
    printMFTRecord(reserved[0]);
    printf("Root directory MTF: \n");
    printMFTRecord(reserved[1]);
    return 0;
}