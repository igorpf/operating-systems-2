#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

//8 bytes per tuple
//32 tuples per sector (256/8)
//128 tuples per block (32*4 sectors per block)
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
void readReservedMTF() {
    int i;
    for (i = 0; i < 4; ++i) {
        reserved[i] = malloc(sizeof(reserved[i]));
        if(read_sector(8, buffer) != 0) {
            printf("Erro ao ler o setor do MFT \n");
            return;
        }
    }
    reserved[0]->atributeType = getWord(buffer[0],buffer[1]);
}
void readBootBlock() {
    block = malloc(sizeof(*block));     
    if(read_sector(0, buffer) != 0) {
        printf("Erro ao ler o setor do bloco de boot\n");
        return;
    }
    if(!block) {
        printf("Erro ao alocar memória\n");
    }
    memcpy(block->id,buffer,4);
    block->version = getWord(buffer[4],buffer[5]);
    block->blockSize = getWord(buffer[6],buffer[7]);
    block->MFTBlocksSize = getWord(buffer[8],buffer[9]);
    block->diskSectorSize = getDWord(buffer[10],buffer[11],buffer[12],buffer[13]);
}
int main() {
    readBootBlock();

    printf("%s\n", block->id);
    printf("%04x\n", block->version);
    printf("%04x\n", block->blockSize);
    printf("%04x\n", block->MFTBlocksSize);
    printf("%04x\n", block->diskSectorSize);
    //%04x
    return 0;
}