#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/utils.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

void clearMemory() {
    int i;
    for(i = 0; i < MFT_TUPLES_PER_RECORD; i++) {
        free(rootMFTRecord[i]);
        rootMFTRecord[i] = NULL;
    }
    free(rootMFTRecord);
    rootMFTRecord = NULL;
}
inline struct t2fs_record* searchSector(int sector, char* filename) {
    read_sector(sector,buffer);
    int j;
    for(j = 0; j < FILE_RECORDS_PER_SECTOR; j++) {
        struct t2fs_record* fileRecord = readFileRecord(j);
        if(strcmp(fileRecord->name,filename)==0) {
            printFileRecord(fileRecord);
            return fileRecord;
        }
        free(fileRecord);
    }
    return NULL;
}
inline struct t2fs_record* searchBlock(int block, char* filename) {    
    int j, baseSector = blockToSector(block);
    for(j = 0; j < SECTORS_PER_BLOCK; j++) {
        struct t2fs_record* fileRecord = searchSector(baseSector+j, filename);
        if(fileRecord) {
            // printFileRecord(fileRecord);
            return fileRecord;
        }
    }
    return NULL;
}
inline struct t2fs_record* createFileRecord(char* filename, int MFTNumber) {
    struct t2fs_record* record = malloc(sizeof(*record));
    record->TypeVal = TYPEVAL_REGULAR;
    strncpy(record->file, filename, MAX_FILE_NAME_SIZE);
    record->blocksFileSize = 1;
    record->bytesFileSize = 0;
    record->MFTNumber = MFTNumber;
}
int main() {
    readBootBlock();
    readRootDirectoryRecord();    
    printf("Root directory MFT: \n");
    read_sector(blockToSector(rootMFTRecord[0]->logicalBlockNumber), buffer);
    // printf("%d",rootMFTRecord[0]->logicalBlockNumber);
    // printFileRecord(readFileRecord(0));
    char file[] = "/file1";
    create2(file);

    clearMemory();
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
FILE2 create2 (char *filename) {
    char *token, *name = strdup(filename); //copy the filename because strtok destroys the input]
    struct t2fs_4tupla ** currentMFTRecord = rootMFTRecord;
    int levels = strCount(name, '/'), 
        currentLevel = 0, 
        currentBlock,
        i;

    for(token = strtok(name, "/");token != NULL; token=strtok(NULL, "/")) {
        currentLevel++;
        for(i=0,currentBlock = currentMFTRecord[i]->logicalBlockNumber;
            currentMFTRecord[i]->atributeType == 1;i++, currentBlock = currentMFTRecord[i]->logicalBlockNumber) {
            //TODO: check for the possibility of having multiple mft records for one file
            struct t2fs_record* file = searchBlock(currentBlock, token);
            if(file) { 
                if(file->TypeVal == TYPEVAL_REGULAR) {
                    //Trying to create an existing file
                    return ERROR;
                }
                else if(file->TypeVal == TYPEVAL_DIRETORIO) {
                    //search recursively inside
                    currentMFTRecord = readMFTRecord(MFTRecordToSector(file->MFTNumber));
                    free(file);
                    break;
                }
                else { //something is wrong with the file, return error
                    printf("Trying to open something that's not a file or directory");
                    return ERROR;
                } 
            } else if(currentLevel == levels) { // everything ok, the file can be created
                /**
                *   1 - Create MFT record
                    2 - Allocate space
                    3 - mark bitmap
                    4 - create an open file record
                */
                int block = searchBitmap(FREE);
                //setBitmap(block, ALLOCATED);
                //
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }    
    /*Didn't find or tried to open directory as file*/
    return ERROR;
}
int delete2 (char *filename) {return NOT_IMPLEMENTED;}

FILE2 open2 (char *filename) {
    /**
        1 - Split filename into tokens
        2 - take the first token and search inside root directory
        3 - if there the current token is found
        4   - if it has next token, is a directory (check if the found record actually is)
        5        - goto 2 assuming current token = next token and search from the found directory
        6    - else if it's a file, open it
        7 - else return error
    */
    char *token, *name = strdup(filename); //copy the filename because strtok destroys the input
    int levels = strCount(name, '/'), 
        currentLevel = 0, 
        currentSector = rootMFTRecord[0]->logicalBlockNumber,//start com the first sector from root directory
        i;
    for(token = strtok(name, "/");token != NULL; token=strtok(NULL, "/")) {
        currentLevel++;
        for(i = 0; i < SECTORS_PER_BLOCK;i++) {
            struct t2fs_record* file = searchSector(blockToSector(currentSector+i), token);
            if(file) {
                if(file->TypeVal == TYPEVAL_REGULAR) {
                    // struct t2fs_4tupla **mftRecord = readMFTRecord(MFTRecordToSector(file->MFTNumber));
                    // printMFTTuple(mftRecord[0]);
                    struct openFileRegister* reg = getNewFileRegister(file);
                    return reg? reg->handle : ERROR;
                }
                else if(file->TypeVal == TYPEVAL_DIRETORIO) {
                    //search recursively inside
                    //
                    struct t2fs_4tupla **mftRecord = readMFTRecord(MFTRecordToSector(file->MFTNumber));
                    currentSector = blockToSector(mftRecord[0]->logicalBlockNumber);
                    //TODO: search recursively mft tuples/records
                    // printMFTTuple(mftRecord[0]);
                    break;
                }
                else { //something is wrong with the file, return error
                    printf("Trying to open something that's not a file or directory");
                    return ERROR;
                } 
            } else
                break;
        }
        
    }    
    /*Didn't find or tried to open directory as file*/
    return ERROR;
}

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
