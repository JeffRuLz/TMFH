#ifndef STORAGE_H
#define STORAGE_H

#include <nds/ndstypes.h>
#include <stdio.h>

#define BACKUP_PATH "/titlebackup"
#define BYTES_PER_BLOCK (1024*128)

//printing
void printBytes(unsigned long long bytes);

//progress bar
void printProgressBar(float percent);
void clearProgressBar();

//Files
bool fileExists(char const* path);
int copyFile(char const* src, char const* dst);
int copyFilePart(char const* src, u32 offset, u32 size, char const* dst);
unsigned long long getFileSize(FILE* f);
unsigned long long getFileSizePath(char const* path);
bool padFile(char const* path, int size);

//Directories
bool dirExists(char const* path);
bool copyDir(char const* src, char const* dst);
bool deleteDir(char const* path);
unsigned long long getDirSize(char const* path);

//home menu
int getMenuSlots();
int getMenuSlotsFree();
#define getMenuSlotsUsed() (getMenuSlots() - getMenuSlotsFree())

//SD card
bool sdIsInserted();
unsigned long long getSDCardSize();
unsigned long long getSDCardFree();
#define getSDCardUsedSpace() (getSDCardSize() - getSDCardFree())

//internal storage
unsigned long long getDsiSize();
unsigned long long getDsiFree();
#define getDsiUsed() (getDSIStorageSize() - getDSIStorageFree())

#endif