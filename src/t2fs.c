#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/utils.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

int main() {
    readBootBlock();
    readRootDirectoryRecord();    
    printf("Root directory MTF: \n");
    read_sector(blockToSector(rootMTFRecord[0]->logicalBlockNumber), buffer);
    printFileRecord(readFileRecord(0));
    printFileRecord(readFileRecord(1));
    printFileRecord(readFileRecord(2));    
    printFileRecord(readFileRecord(3));
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
