#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

WORD getWord(char lsb, char msb) {
    return (WORD) lsb | (msb << 8);
}
DWORD getDWord(char b1, char b2, char b3, char b4) {
    return (DWORD) b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}
struct t2fs_bootBlock* readBlock() {
    struct t2fs_bootBlock* block = malloc(sizeof(*block));     
    BYTE buffer[SECTOR_SIZE] = {0};    

    if(read_sector(0, buffer) != 0) {
        printf("Erro ao ler o setor do bloco de boot\n");
        return NULL;
    }
    if(!block) {
        printf("Erro ao alocar memória\n");
    }
    memcpy(block->id,buffer,4);
    block->version = getWord(buffer[4],buffer[5]);
    block->blockSize = getWord(buffer[6],buffer[7]);
    block->MFTBlocksSize = getWord(buffer[8],buffer[9]);
    block->diskSectorSize = getDWord(buffer[10],buffer[11],buffer[12],buffer[13]);
    return block;
}
int main() {
    struct t2fs_bootBlock* block = readBlock();

    printf("%s\n", block->id);
    printf("%04x\n", block->version);
    printf("%04x\n", block->blockSize);
    printf("%04x\n", block->MFTBlocksSize);
    printf("%04x\n", block->diskSectorSize);
    //%04x
    return 0;
}