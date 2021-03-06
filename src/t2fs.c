#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/utils.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

int initialized = 0;
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
            // printFileRecord(fileRecord);
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

inline int deleteFileRecord(int block, char* filename) {    
    int i, j, baseSector = blockToSector(block);
    for(i = 0; i < SECTORS_PER_BLOCK; i++) {
        read_sector(baseSector+i,buffer);
        for(j = 0; j < FILE_RECORDS_PER_SECTOR; j++) {
            struct t2fs_record* fileRecord = readFileRecord(j);
            if(strcmp(fileRecord->name,filename)==0) {
                fileRecord->TypeVal = TYPEVAL_INVALIDO;                
                strcpy(fileRecord->name,"");
                fileRecord->blocksFileSize = 0;
                fileRecord->bytesFileSize = 0;
                fileRecord->MFTNumber = 0;
                buffer[j] = fileRecord->TypeVal;
                memcpy(buffer+j+1, fileRecord->name, MAX_FILE_NAME_SIZE);
                memcpy(buffer+j+1+MAX_FILE_NAME_SIZE, dwordToBytes(fileRecord->blocksFileSize), 4);
                memcpy(buffer+j+5+MAX_FILE_NAME_SIZE, dwordToBytes(fileRecord->bytesFileSize), 4);
                memcpy(buffer+j+9+MAX_FILE_NAME_SIZE, dwordToBytes(fileRecord->MFTNumber), 4);
                write_sector(baseSector+i,buffer);
                // printFileRecord(fileRecord);
                return SUCCESS;
            }
        free(fileRecord);
        }        
    }
    return ERROR;
}

inline struct t2fs_record* createFileRecord(BYTE type, char* filename, int MFTNumber) {
    struct t2fs_record* record = malloc(sizeof(*record));
    record->TypeVal = type;
    strncpy(record->name, filename, MAX_FILE_NAME_SIZE);
    record->blocksFileSize = 1; //0??
    record->bytesFileSize = 0;
    record->MFTNumber = MFTNumber;
    return record;
}
inline int writeFileRecord(int block, struct t2fs_record* record){
    int i, j;
    for(i = 0; i < SECTORS_PER_BLOCK;i++) {
        int sector = blockToSector(block)+i; 
        read_sector(sector,buffer);
        for(j = 0; j < FILE_RECORDS_PER_SECTOR;j++) {
            struct t2fs_record* fileRecord = readFileRecord(j);
            // printFileRecord(fileRecord);
            if(fileRecord->TypeVal != TYPEVAL_REGULAR && fileRecord->TypeVal != TYPEVAL_DIRETORIO) {                
                int index = sizeof(*record)*j;
                
                buffer[index] = record->TypeVal;
                memcpy(buffer+index+1, record->name, MAX_FILE_NAME_SIZE);
                memcpy(buffer+index+1+MAX_FILE_NAME_SIZE, dwordToBytes(record->blocksFileSize), 4);
                memcpy(buffer+index+5+MAX_FILE_NAME_SIZE, dwordToBytes(record->bytesFileSize), 4);
                memcpy(buffer+index+9+MAX_FILE_NAME_SIZE, dwordToBytes(record->MFTNumber), 4);
                write_sector(sector,buffer);
                return SUCCESS;
            }
        }
    }
    return ERROR;
}
void init() {
    // printf("init\n");
    readBootBlock();
    readRootDirectoryRecord();
    initialized = 1;
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
    if(!initialized)
        init();
    if(isValidFileName(filename) == ERROR){
        return ERROR;
    }

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
                    currentMFTRecord = readMFTRecord(file->MFTNumber);
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
                    2 - create an open file record
                */
                struct t2fs_record* fileRecord = createFileRecord(TYPEVAL_REGULAR,token, getNewMFTRecord());
                // printFileRecord(fileRecord);
                writeFileRecord(currentBlock, fileRecord);
                struct openFileRegister* fr = getNewFileRegister(fileRecord);
                return fr? fr->handle:ERROR;
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }    
    /*Didn't find or tried to open directory as file*/
    return ERROR;
}
int delete2 (char *filename) {

    if(!initialized)
        init();

    if(isValidFileName(filename) == ERROR){
        return ERROR;
    }

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
                    if(currentLevel < levels) //some directory in the path is actually a file. throw an error
                        return ERROR;
                    if(deallocateBlocksFromMFT(file->MFTNumber) == SUCCESS){
                        deleteFileRecord(currentBlock, token);
                        return setAsFreeMFT(file->MFTNumber);    
                    }
                    else
                        return ERROR;
                }
                else if(file->TypeVal == TYPEVAL_DIRETORIO) {
                    //search recursively inside
                    currentMFTRecord = readMFTRecord(file->MFTNumber);
                    free(file);
                    break;
                }
                else { //something is wrong with the file, return error
                    //printf("Trying to open something that's not a file or directory");
                    return ERROR;
                } 
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }

    return ERROR;
}

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
    if(!initialized)
        init();

    if(isValidFileName(filename) == ERROR){
        return ERROR;
    }

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
                    if(currentLevel < levels) //some directory in the path is actually a file. throw an error
                        return ERROR;
                    struct openFileRegister* fr = getNewFileRegister(file);
                    return fr? fr->handle:ERROR;    
                }
                else if(file->TypeVal == TYPEVAL_DIRETORIO) {
                    //search recursively inside
                    currentMFTRecord = readMFTRecord(file->MFTNumber);
                    free(file);
                    break;
                }
                else { //something is wrong with the file, return error
                    printf("Trying to open something that's not a file or directory");
                    return ERROR;
                } 
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }    
    /*Didn't find or tried to open directory as file*/
    return ERROR;
}

int close2 (FILE2 handle){
    if(!initialized)
        init();
    struct openFileRegister *fileRegister;

    fileRegister = (struct openFileRegister *) getOpenFileRegisterByHandle(handle);

    if(fileRegister && fileRegister->fileRecord->TypeVal != TYPEVAL_REGULAR){
        printf("\n[Close2] Handler is not a file type TYPEVAL_REGULAR\n");
        return ERROR;
    }

    return removeFromOpenFiles(handle);
}

int read2 (FILE2 handle, char *buffer, int size){return NOT_IMPLEMENTED;}
int write2 (FILE2 handle, char *buffer, int size){return NOT_IMPLEMENTED;}
int truncate2 (FILE2 handle){return NOT_IMPLEMENTED;}
int seek2 (FILE2 handle, DWORD offset){return NOT_IMPLEMENTED;}
int mkdir2 (char *pathname) {
    if(!initialized)
        init();
    if(isValidFileName(pathname) == ERROR){
        return ERROR;
    }

    char *token, *name = strdup(pathname); //copy the filename because strtok destroys the input]
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
                    currentMFTRecord = readMFTRecord(file->MFTNumber);
                    free(file);
                    break;
                }
                else { //something is wrong with the file, return error
                    printf("Trying to open something that's not a file or directory");
                    return ERROR;
                } 
            } else if(currentLevel == levels) { // everything ok, the directory can be created
                /**
                *   1 - Create MFT record
                    2 - create an open file record
                    3 - write record to current directory
                */
                struct t2fs_record* fileRecord = createFileRecord(TYPEVAL_DIRETORIO,token, getNewMFTRecord());
                writeFileRecord(currentBlock, fileRecord);
                return 0;
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }    
    /*Didn't find or tried to open directory as file*/
    return ERROR;
}

int rmdir2 (char *pathname){
    if(!initialized)
        init();

    if(isValidFileName(pathname) == ERROR){
        return ERROR;
    }

    char *token, *name = strdup(pathname); //copy the filename because strtok destroys the input]
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
                        printf("\n[rmdir2] Path leads to a file, not a directory\n");                    
                        return ERROR;
                }
                else if(file->TypeVal == TYPEVAL_DIRETORIO) {
                    if(currentLevel == levels){ //its a directory and is the last level -> it's this one we should REMOVE
                        /*Before deallocate dirs data, delete it's chidlren data*/


                        if(deallocateBlocksFromMFT(file->MFTNumber) == SUCCESS){
                            deleteFileRecord(currentBlock, token);
                            return setAsFreeMFT(file->MFTNumber);    
                        }
                    }

                    //or search recursively inside
                    currentMFTRecord = readMFTRecord(file->MFTNumber);
                    free(file);
                    break;
                }
                else { //something is wrong with the file, return error
                    printf("Trying to REMOVE something that's not a file or directory");
                    return ERROR;
                } 
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }    
    return ERROR;
}

DIR2 opendir2 (char *pathname){
    if(!initialized)
        init();
    if(isValidFileName(pathname) == ERROR){
        return ERROR;
    }

    char *token, *name = strdup(pathname); //copy the filename because strtok destroys the input]
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
                		printf("\n[opendir2] Path leads to a file, not a directory\n");                    
                        return ERROR;
                }
                else if(file->TypeVal == TYPEVAL_DIRETORIO) {
                    if(currentLevel == levels){ //its a directory and is the last level -> it's this one we should open
	                    struct openFileRegister* fr = getNewFileRegister(file);
    	                return fr? fr->handle:ERROR;
                    }

                    //or search recursively inside
                    currentMFTRecord = readMFTRecord(file->MFTNumber);
                    free(file);
                    break;
                }
                else { //something is wrong with the file, return error
                    printf("Trying to open something that's not a file or directory");
                    return ERROR;
                } 
            } else { //some directory in the path does not exist
                return ERROR;
            }

        }
    }    
    return ERROR;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry) {
    if(!initialized)
        init();
    int currentBlock,
        blockLimit,
        i;

    struct openFileRegister* dir = getOpenFileRegisterByHandle(handle);
    if(!dir || dir->fileRecord->TypeVal != TYPEVAL_DIRETORIO){
        return ERROR;
    }
    struct t2fs_4tupla ** currentMFTRecord = readMFTRecord(dir->fileRecord->MFTNumber);
    // printMFTTuple(currentMFTRecord[0]);
    
    for(i=0;currentMFTRecord[i]->atributeType == 1;i++) {
        for(currentBlock = currentMFTRecord[i]->logicalBlockNumber, blockLimit = currentMFTRecord[i]->logicalBlockNumber + currentMFTRecord[i]->numberOfContiguosBlocks; 
            currentBlock < blockLimit; currentBlock++) {            
            int j, baseSector = blockToSector(currentBlock);
            for(j = 0; j < SECTORS_PER_BLOCK; j++) {
                read_sector(baseSector+j,buffer);
                int k;
                for(k = 0; k < FILE_RECORDS_PER_SECTOR; k++) {        
                    struct t2fs_record* fileRecord = readFileRecord(k);
                    // if valid entry, return                    
                    if(dir->currentEntry >=  (j*FILE_RECORDS_PER_SECTOR)+k+1)continue;
                    if(fileRecord->TypeVal == TYPEVAL_DIRETORIO || fileRecord->TypeVal == TYPEVAL_REGULAR) {
                        dentry->fileType = fileRecord->TypeVal;
                        dentry->fileSize = fileRecord->bytesFileSize;
                        strncpy(dentry->name, fileRecord->name, MAX_FILE_NAME_SIZE);
                        dir->currentEntry =  (j*FILE_RECORDS_PER_SECTOR)+k+1;
                        return 0;                        
                    }
                    // printFileRecord(fileRecord);
                    free(fileRecord);
                }
            }
        }
    }
      
    /*Didn't find or tried to open directory as file*/
    return END_OF_DIR;
 }
int closedir2 (DIR2 handle){
    if(!initialized)
        init();
    struct openFileRegister *fileRegister;

    fileRegister = (struct openFileRegister *) getOpenFileRegisterByHandle(handle);

    if(fileRegister && fileRegister->fileRecord->TypeVal != TYPEVAL_DIRETORIO){
        printf("\n[Close2] Handler is not a file type TYPEVAL_DIRETORIO\n");
        return ERROR;
    }

    return removeFromOpenFiles(handle);
}
