#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>

#define BACKUP_PATH "/titlebackup/"
#define ROM_PATH "/dsi/"

#define BYTES_PER_BLOCK (1024*128)

//Printing
void printBytes(unsigned long long bytes);
void printFileInfo(const char* path);

//Progress bar
void printProgressBar(int progress, int total);
void clearProgressBar();

//Files
int copyFile(const char* in, char* out);
unsigned long long getFileSize(FILE* f);
unsigned long long getFileSizePath(const char* path);
int padFile(const char* path, int size);

//Directories
int dirExists(const char* path);
//int copyDir(const char* in, char* out);
int deleteDir(const char* path);
unsigned long long getDirSize(const char* path);

//Home menu
int getMenuSlots();
int getMenuSlotsFree();
#define getMenuSlotsUsed() (getMenuSlots() - getMenuSlotsFree())

//SD Card
int sdIsInserted();

unsigned long long getSDCardSize();
unsigned long long getSDCardFree();
#define getSDCardUsedSpace() (getSDCardSize() - getSDCardFree())

//Internal storage
int getDsiSize();
int getDsiFree();
#define getDsiUsed() (getDSIStorageSize() - getDSIStorageFree())

#endif