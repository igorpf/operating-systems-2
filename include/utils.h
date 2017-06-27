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
#define MFT_FIRST_VALID_SECTOR 12//first valid sector after reserved ones
#define MFT_TUPLES_PER_RECORD 32
#define MFT_RECORD_SIZE 512 //each MFT record consists of 32 tuples of 16 bytes
#define SECTORS_PER_BLOCK 4
#define FILE_RECORDS_PER_SECTOR 4
#define MFT_LAST_SECTOR 8196 //2048 blocks * 4 sector per block + 4
#define MAX_OPEN_FILES 20

struct openFileRegister {
    struct t2fs_record* fileRecord;
    DWORD currentPointer;
    FILE2 handle;    
};

extern struct  t2fs_4tupla** rootMFTRecord; //array of 32 tuples
extern struct t2fs_bootBlock* block;
extern BYTE buffer[SECTOR_SIZE];
extern struct openFileRegister **openFiles;

void printMFTTuple(struct t2fs_4tupla* record);
WORD getWord(char lsb, char msb);
DWORD getDWord(char b1, char b2, char b3, char b4);
BYTE* wordToBytes(WORD word);
BYTE* dwordToBytes(DWORD dword);
int blockToSector(int block);
struct t2fs_4tupla* readMFTtuple(int tupleNumber);
struct t2fs_4tupla** readMFTRecord(int recordNumber);
struct t2fs_record* readFileRecord(int recordNumber);
void writeMFTRecord(struct t2fs_4tupla* record, int recordNumber, unsigned int sector);
void printFileRecord(struct t2fs_record* fileRecord);
void readRootDirectoryRecord();
void readBootBlock();
void printMFTTuple(struct t2fs_4tupla* tuple);
int strCount(const char* str, const char c);
struct openFileRegister* getNewFileRegister(struct t2fs_record* fileRecord); //
int MFTRecordToSector(int recordNumber);
#endif