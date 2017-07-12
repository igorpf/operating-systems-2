#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

int main() {
    
    
    char dir[] = "/file13", file[]="/file13/test2.txt", file2[]="/file13/test3.txt";
    

    printf("------- TESTING CREATING DIR\n");

    printf("mkdir %d\n",mkdir2(dir));
    DIR2 dirHandl = opendir2(dir);
    printf("open dir %d\n", dirHandl);

    // printf("close dir (handle: %d) %d\n", dirHandl, closedir2(dirHandl));

    
    printf("------- TESTING CREATING FILE\n");

    printf("Trying to delete file before creating, should return ERROR (-2): %d\n",delete2(file));

    FILE2 fhandle = create2(file);
    printf("\nFile Handler:%i\n", fhandle);
    
    printf("------- TESTING CREATING SECOND FILE\n");

    FILE2 f2handle = create2(file2);
    printf("\nFile 2 Handler:%i\n", f2handle);

    DIRENT2 *dirent = malloc(sizeof(*dirent));
    printf("readdir2: should be 0: %d, direntryName: %s\n", readdir2(dirHandl,dirent), dirent->name);
    printf("readdir2: should be 0: %d, direntryName: %s\n", readdir2(dirHandl,dirent), dirent->name);
    printf("readdir2: should be end of dir(%d): %d \n", END_OF_DIR,readdir2(dirHandl,dirent));

    printf("close file (handle: %d) %d\n", fhandle, close2(fhandle));    
    printf("Trying to delete file, should return 0: %d\n",delete2(file));
    printf("Removing dir %s, should return 0: %d\n", dir, rmdir2(dir));    
    return 0;
}