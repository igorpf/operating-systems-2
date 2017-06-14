#ifndef _utils_h_
#define _utils_h_

#include "t2fs.h"

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

extern struct  t2fs_4tupla** rootMTFRecord; //array of 32 tuples
extern struct t2fs_bootBlock* block;
extern BYTE buffer[SECTOR_SIZE];

void printMFTTuple(struct t2fs_4tupla* record);
WORD getWord(char lsb, char msb);
DWORD getDWord(char b1, char b2, char b3, char b4);
BYTE* wordToBytes(WORD word);
BYTE* dwordToBytes(DWORD dword);
int blockToSector(int block);
struct t2fs_4tupla* readMFTtuple(int tupleNumber);
struct t2fs_4tupla** readMFTRecord(int sector);
struct t2fs_record* readFileRecord(int recordNumber);
void writeMTFRecord(struct t2fs_4tupla* record, int recordNumber, unsigned int sector);
void printFileRecord(struct t2fs_record* fileRecord);
void readRootDirectoryRecord();
void readBootBlock();
void printMFTTuple(struct t2fs_4tupla* tuple);
#endif