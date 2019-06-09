#include "fat12.h"
#include <string.h>
#include <malloc.h>

static u32 _getClusterSize(u32 sizebytes)
{
	if (sizebytes < 573440)
		return 512;
	
	else if (sizebytes < 5472256)
		return 2048;
	
	else if (sizebytes < 17301504)
		return 4096;
	
	else
		return 2048;
}

static u16 _getMaxFiles(u32 sizebytes)
{
	if (sizebytes < 573440)
		return 16;
	
	else
		return 256;
}

//wip
static u16 _getFatz(u32 sizebytes)
{
	if (sizebytes <= 0x4000)		//16 kb
		return 1;

	else if (sizebytes <= 0x200000)	//2 mb
		return 3;

	else
		return 6;
}

bool initFatHeader(FILE* f)
{
	if (!f)
		return false;
	
	//get size
	fseek(f, 0, SEEK_END);
	u32 size = ftell(f);
	
	FATHeader* h = (FATHeader*)malloc(sizeof(FATHeader));
	
	h->BS_JmpBoot[0] = 0xE9;
	h->BS_JmpBoot[1] = 0;
	h->BS_JmpBoot[2] = 0;
	
	h->BS_OEMName[0] = 'M';
	h->BS_OEMName[1] = 'S';
	h->BS_OEMName[2] = 'W';
	h->BS_OEMName[3] = 'I';
	h->BS_OEMName[4] = 'N';
	h->BS_OEMName[5] = '4';
	h->BS_OEMName[6] = '.';
	h->BS_OEMName[7] = '1';
	
	h->BPB_BytesPerSec = 512;
	h->BPB_SecPerClus = _getClusterSize(size) / h->BPB_BytesPerSec;
	h->BPB_RsvdSecCnt = 1;
	h->BPB_NumFATs = 2;
	h->BPB_RootEntCnt = _getMaxFiles(size) * 2;
	h->BPB_TotSec16 = size / h->BPB_BytesPerSec;	//
	h->BPB_Media = 0xF8;	
	h->BPB_FATSz16 = _getFatz(size);			//
	h->BPB_SecPerTrk = 0;
	h->BPB_NumHeads = 0;
	h->BPB_HiddSec = 0;
	h->BPB_TotSec32 = 0;
	h->BS_DrvNum = 2;
	h->BS_Reserved1 = 0;
	h->BS_BootSig = 0x29;
	h->BS_VolID = 305419896;
	
	h->BS_VolLab[0] = 'V';
	h->BS_VolLab[1] = 'O';
	h->BS_VolLab[2] = 'L';
	h->BS_VolLab[3] = 'U';
	h->BS_VolLab[4] = 'M';
	h->BS_VolLab[5] = 'E';
	h->BS_VolLab[6] = 'L';
	h->BS_VolLab[7] = 'A';
	h->BS_VolLab[8] = 'B';
	h->BS_VolLab[9] = 'E';
	h->BS_VolLab[10] = 'L';
	
	h->BS_FilSysType[0] = 'F';
	h->BS_FilSysType[1] = 'A';
	h->BS_FilSysType[2] = 'T';
	h->BS_FilSysType[3] = '1';
	h->BS_FilSysType[4] = '2';
	h->BS_FilSysType[5] = ' ';
	h->BS_FilSysType[6] = ' ';
	h->BS_FilSysType[7] = ' ';
	
	memset(h->BS_BootCode, 0, 448);
	
	h->BS_BootSign = 0xAA55;
	
	fseek(f, 0, SEEK_SET);
	fwrite(h, sizeof(FATHeader), 1, f);

	free(h);
	
	return true;
}