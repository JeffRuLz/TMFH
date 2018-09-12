#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>

#define BACKUP_PATH "/titlebackup/"
#define ROM_PATH "/dsi/"

#define BYTES_PER_BLOCK (1024*128)

//Printing
void printBytes(int bytes);
void printFileInfo(const char* path);

//Progress bar
void printProgressBar(int progress, int total);
void clearProgressBar();

//Files
int copyFile(const char* in, char* out);
int getFileSize(FILE* f);
int getFileSizePath(const char* path);

//Directories
int dirExists(const char* path);
//int copyDir(const char* in, char* out);
int deleteDir(const char* path);
int getDirSize(const char* path);

//Home menu
int getMenuSlots();
int getMenuSlotsFree();
#define getMenuSlotsUsed() (getMenuSlots() - getMenuSlotsFree())

//SD Card
int sdIsInserted();

int getSDCardSize();
int getSDCardFree();
#define getSDCardUsedSpace() (getSDCardSize() - getSDCardFree())

//Internal storage
int getDsiSize();
int getDsiFree();
#define getDsiUsed() (getDSIStorageSize() - getDSIStorageFree())

#endif