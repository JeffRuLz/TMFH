#ifndef FAT12_H
#define FAT12_H

#include <nds/ndstypes.h>
#include <stdio.h>
//http://elm-chan.org/docs/fat_e.html

#pragma pack(push, 1)
typedef struct
{
	u8 BS_JmpBoot[3];	//0x0000
	u8 BS_OEMName[8];	//0x0003
	u16 BPB_BytesPerSec;	//0x000B
	u8 BPB_SecPerClus;	//0x000D
	u16 BPB_RsvdSecCnt;	//0x000E
	u8 BPB_NumFATs;
	u16 BPB_RootEntCnt;
	u16 BPB_TotSec16;
	u8 BPB_Media;
	u16 BPB_FATSz16;
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32 BPB_HiddSec;
	u32 BPB_TotSec32;
	u8 BS_DrvNum;
	u8 BS_Reserved1;
	u8 BS_BootSig;
	u32 BS_VolID;
	u8 BS_VolLab[11];
	u8 BS_FilSysType[8];
	u8 BS_BootCode[448];
	u16 BS_BootSign;
} FATHeader;
#pragma pack(push, 0)

bool initFatHeader(FILE* f);

/*
typedef struct
{
	u16 bytesPerSector;	//0x0B - Bytes per Sector. The size of a hardware sector. For most disks in use in the United States, the value of this field is 512.
	u8 sectorsPerCluster;	//0x0D - Sectors Per Cluster. The number of sectors in a cluster. The default cluster size for a volume depends on the volume size and the file system.
	u16 reservedSectors;	//0x0E - Reserved Sectors. The number of sectors from the Partition Boot Sector to the start of the first file allocation table, including the Partition Boot Sector. The minimum value is 1. If the value is greater than 1, it means that the bootstrap code is too long to fit completely in the Partition Boot Sector.
	u8 numberOfTables;	//0x10 - Number of file allocation tables (FATs). The number of copies of the file allocation table on the volume. Typically, the value of this field is 2.
	u16 rootEntries;		//0x11 - Root Entries. The total number of file name entries that can be stored in the root folder of the volume. One entry is always used as a Volume Label. Files with long filenames use up multiple entries per file. Therefore, the largest number of files in the root folder is typically 511, but you will run out of entries sooner if you use long filenames.
	u16 smallSectors;		//0x13 - Small Sectors. The number of sectors on the volume if the number fits in 16 bits (65535). For volumes larger than 65536 sectors, this field has a value of 0 and the Large Sectors field is used instead.
	u8 mediaType;			//0x15 - Media Type. Provides information about the media being used. A value of 0xF8 indicates a hard disk.
	u16 sectorsPerTable;	//0x16 - Sectors per file allocation table (FAT). Number of sectors occupied by each of the file allocation tables on the volume. By using this information, together with the Number of FATs and Reserved Sectors, you can compute where the root folder begins. By using the number of entries in the root folder, you can also compute where the user data area of the volume begins.
	u16 sectorsPerTrack;	//0x18 - Sectors per Track. The apparent disk geometry in use when the disk was low-level formatted.
	u16 numberOfHeads;		//0x1A - Number of Heads. The apparent disk geometry in use when the disk was low-level formatted.
	u32 hiddenSectors;	//0x1C - Hidden Sectors. Same as the Relative Sector field in the Partition Table.
	u32 largeSectors;		//0x20 - Large Sectors. If the Small Sectors field is zero, this field contains the total number of sectors in the volume. If Small Sectors is nonzero, this field contains zero.
} FATBiosBlock;

typedef struct
{
	FATBiosBlock fatBiosBlock;
	u8 physicalDiskNumber;	//0x24 - Physical Disk Number. This is related to the BIOS physical disk number. Floppy drives are numbered starting with 0x00 for the A disk. Physical hard disks are numbered starting with 0x80. The value is typically 0x80 for hard disks, regardless of how many physical disk drives exist, because the value is only relevant if the device is the startup disk.
	u8 currentHead;			//0x25 - Current Head. Not used by the FAT file system.
	u8 signature;				//0x26 - Signature. Must be either 0x28 or 0x29 in order to be recognized by Windows NT.
	u8 volumeSerialNumber[4];	//0x27 - Volume Serial Number. A unique number that is created when you format the volume.
	u8 volumeLabel[11];		//0x2B - Volume Label. This field was used to store the volume label, but the volume label is now stored as special file in the root directory.
	u8 systemID[8];			//0x36 - System ID. Either FAT12 or FAT16, depending on the format of the disk.
} FATBiosBlockExtended;

typedef struct
{
	u8 jump[3];						//0x0000
	u8 OEMName[8];					//0x0003
	FATBiosBlockExtended biosBlock;	//0x000B
	u8 bootstrapCode[448];			//0x003E
	u16 endOfSector;				//0x01FE
} FATBootSector;


*/
#endif