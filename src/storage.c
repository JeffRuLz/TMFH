#include "storage.h"
#include "main.h"
#include <nds.h>
#include <string.h>
#include <dirent.h>

#define TITLE_LIMIT 39

//Printing
void printBytes(unsigned long long bytes)
{
	if (bytes < 1024)
		iprintf("%dB", (unsigned int)bytes);

	else if (bytes < 1024 * 1024)
		printf("%.2fKB", (float)bytes / 1024);

	else if (bytes < 1024 * 1024 * 1024)
		printf("%.2fMB", (float)bytes / 1024 / 1024);

	else
		printf("%.2fGB", (float)bytes / 1024 / 1024 / 1024);
}

void printFileInfo(const char* path)
{
	if (path == NULL) return;

	clearScreen(&topScreen);

	tDSiHeader* header = (tDSiHeader*)malloc(sizeof(tDSiHeader));
	tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

	FILE* f = fopen(path, "rb");
	
	if (f)
	{
		if (fread(header, sizeof(tDSiHeader), 1, f) != 1)
			iprintf("Could not read dsi header.\n");

		else
		{
			fseek(f, header->ndshdr.bannerOffset, SEEK_SET);

			if (fread(banner, sizeof(tNDSBanner), 1, f) != 1)
				iprintf("Could not read banner.\n");

			else
			{
				//Proper title
				{
					char gameTitle[128+1];
					gameTitle[128] = '\0';
					
					//Convert 2 byte characters to 1 byte
					for (int i = 0; i < 128; i++)
						gameTitle[i] = (char)banner->titles[1][i];				

					iprintf("%s\n\n", gameTitle);
				}

				//File size
				{
					iprintf("Size: ");
					printBytes(getFileSize(f));
					iprintf("\n");
				}

				iprintf("Label: %.12s\n", header->ndshdr.gameTitle);
				iprintf("Game Code: %.4s\n", header->ndshdr.gameCode);

				//System type
				{
					iprintf("Unit Code: ");

					switch (header->ndshdr.unitCode)
					{
						case 0: iprintf("NDS"); break;
						case 2: iprintf("NDS+DSi"); break;
						case 3: iprintf("DSi"); break;
						default: iprintf("unknown");
					}

					iprintf("\n");
				}

				//Application type
				{
					iprintf("Program Type: ");

					switch (header->ndshdr.reserved1[7])
					{
						case 0x3: iprintf("Normal"); break;
						case 0xB: iprintf("Sys"); break;
						case 0xF: iprintf("Debug/Sys"); break;
						default:  iprintf("unknown");
					}

					iprintf("\n");
				}				

				//DSi data
				if (header->tid_high == 0x00030004 ||
					header->tid_high == 0x00030005 ||
					header->tid_high == 0x00030015 ||
					header->tid_high == 0x00030017 ||
					header->tid_high == 0x00030000)
				{
					iprintf("Title ID: %08x %08x\n", (unsigned int)header->tid_high,
												 	 (unsigned int)header->tid_low);			
				}

				//Print full file path
				iprintf("\n%s\n", path);
			}
		}
	}

	fclose(f);

	free(banner);
	free(header);
}

//Progress bar
void printProgressBar(int percent)
{
	//Limit 100 max
	if (percent > 100)
		percent = 100;

	//Print frame
	iprintf("\x1b[23;0H[");
	iprintf("\x1b[23;31H]");

	//Skip if there are no bars
	if (percent <= 0)
		return;

	//Print bars
	int bars = (int)(30.f * (percent / 100.f)) + 1;

	for (int i = 0; i < bars; i++)
		iprintf("\x1b[23;%dH|", 1 + i);
}

void clearProgressBar()
{
	iprintf("\x1b[23;0H                                ");
}

//Files
bool copyFile(const char* in, char* out)
{
	bool result = false;

	if (in == NULL || out == NULL)
		return false;

	FILE* fin = fopen(in, "rb");
	FILE* fout = fopen(out, "wb");

	if (fin == NULL || fout == NULL)
	{
		fclose(fin);
		fclose(fout);

		return false;
	}
	else
	{
		consoleSelect(&topScreen);

		int fileSize = getFileSize(fin);
		int totalBytesRead = 0;

		int percent = 0;
		int lastPercent = 0;

		const int buffSize = 1024*8; //Arbitrary. A value too large freezes the system.
		unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * buffSize);		

		while (1)
		{
			int bytesRead = fread(buffer, 1, buffSize, fin);	
			fwrite(buffer, 1, bytesRead, fout);

			totalBytesRead += bytesRead;

			//Re-print progress bar every so often, but not every time
			percent = (int)( ((float)totalBytesRead / (float)fileSize) * 100.f );			
			if (percent != lastPercent)
			{
				lastPercent = percent;
				printProgressBar(percent);
			}			

			if (feof(fin))
			{
				result = 1;
				break;
			}

			if (ferror(fin))
			{
				result = 0;
				break;
			}
		}

		free(buffer);

		clearProgressBar();
		consoleSelect(&bottomScreen);
	}

	fclose(fin);
	fclose(fout);

	return result;
}

unsigned long long getFileSize(FILE* f)
{
	if (!f)
		return 0;

	fseek(f, 0, SEEK_END);
	unsigned long long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	return size;
}

unsigned long long getFileSizePath(const char* path)
{
	if (path == NULL)
		return -1;

	FILE* f = fopen(path, "rb");
	unsigned long long size = getFileSize(f);
	fclose(f);

	return size;
}

bool padFile(const char* path, int size)
{
	FILE* f = fopen(path, "ab");

	if (!f)
		return 0;

	else
	{
		unsigned char zero = 0;
		fwrite(&zero, sizeof(char), size, f);
	}

	fclose(f);
	
	return 1;
}

//Directories
bool dirExists(const char* path)
{
	if (path == NULL)
		return 0;

	DIR* dir = opendir(path);

	if (dir)
	{
		closedir(dir);
		return 1;
	}

	closedir(dir);
	return 0;
}

/* Incomplete
int copyDir(char* in, char* out)
{
	if (in == NULL || out == NULL)
		return 0;

	DIR* dir;
	struct dirent *ent;

	dir = opendir(in);

	if (!dir)
	{
		closedir(dir);
		return 0;
	}
	else
	{
		mkdir(out, 0777);

		while ( (ent = readdir(dir)) != NULL )
		{
			if(strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
			{
				char inPath[512];
				char outPath[512];

				sprintf(inPath, "%s%s/", in, ent->d_name);
				sprintf(outPath, "%s%s/", out, ent->d_name);
				
				copyDir(inPath, outPath);
			}
			else
			{
				char inPath[512];
				char outPath[512];

				sprintf(inPath, "%s%s", in, ent->d_name);
				sprintf(outPath, "%s%s", out, ent->d_name);

				copyFile(inPath, outPath);
			}
		}
	}

	closedir(dir);
	return 1;
}
*/

bool deleteDir(const char* path)
{
	if (strcmp("/", path) == 0)
	{
		//oh fuck no
		return 0;
	}

	DIR* dir;
	struct dirent *ent;

	dir = opendir(path);

	if (!dir)
	{
		closedir(dir);
		return 0;
	}
	else
	{
		while ( (ent = readdir(dir)) != NULL )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
			{
				//Delete directory
				char subpath[512];
				sprintf(subpath, "%s%s/", path, ent->d_name);

				deleteDir(subpath);
			}
			else
			{
				//Delete file
				char fpath[512];
				sprintf(fpath, "%s%s", path, ent->d_name);

				remove(fpath);
			}
		}		
	}

	closedir(dir);
	rmdir(path);

	return 1;
}

unsigned long long getDirSize(const char* path)
{
	if (path == NULL)
		return 0;

	unsigned long long size = 0;

	DIR* dir;
	struct dirent *ent;

	dir = opendir(path);

	if (dir)
	{
		while ( (ent = readdir(dir)) != NULL )
		{
			if(strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
			{
				char fullpath[512];
				sprintf(fullpath, "%s%s/", path, ent->d_name);

				size += getDirSize(fullpath);
			}
			else
			{				
				char fullpath[256];
				sprintf(fullpath, "%s%s", path, ent->d_name);				

				size += getFileSizePath(fullpath);
			}
		}
	}

	closedir(dir);
	return size;
}


//Home menu
int getMenuSlots()
{
	//Assume the home menu has a hard limit on slots
	//Find a better way to do this
	return TITLE_LIMIT;
}

#define NUM_OF_DIRECTORIES 3
static const char* directories[] = {
	"00030004",
	"00030005",
	"00030015"
};

int getMenuSlotsFree()
{
	//Get number of open menu slots by subtracting the number of directories in the title folders
	//Find a better way to do this

	int freeSlots = TITLE_LIMIT;
	
	DIR* dir;
	struct dirent* ent;	
	
	for (int i = 0; i < NUM_OF_DIRECTORIES; i++)
	{
		char path[256];
		sprintf(path, "/title/%s", directories[i]);
		
		dir = opendir(path);
		
		if (dir)
		{
			while ( (ent = readdir(dir)) != NULL )
			{
				if(strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
					continue;
				
				if (ent->d_type == DT_DIR)
					freeSlots -= 1;
			}			
		}

		closedir(dir);
	}
	
	return freeSlots;
}

//SD Card
bool sdIsInserted()
{
//	return sdmmc_cardinserted();

	return 1;
}

unsigned long long getSDCardSize()
{
	if (sdIsInserted())
	{
		struct statvfs st;
		if (statvfs("/", &st) == 0)
			return st.f_bsize * st.f_blocks;
	}

	return 0;
}

unsigned long long getSDCardFree()
{
	if (sdIsInserted())
	{
		struct statvfs st;
		if (statvfs("/", &st) == 0)
			return st.f_bsize * st.f_bavail;
	}

	return 0;
}

//Internal storage
int getDsiSize()
{
	//The DSi has 256MB of internal storage. Some is unavailable and used by other things.
	//Find a better way to do this
	//return 248 * 1024 * 1024;
	return 240 * 1024 * 1024;
}

int getDsiFree()
{
	//Get free space by subtracting file sizes in emulated nand folders
	//Find a better way to do this
	int size = getDsiSize();

	size -= getDirSize("/sys/");
	size -= getDirSize("/title/");
	size -= getDirSize("/ticket/");
	size -= getDirSize("/shared1/");
	size -= getDirSize("/shared2/");
	size -= getDirSize("/import/");
	size -= getDirSize("/tmp/");
	size -= getDirSize("/progress/");

	size -= getDirSize("/photo/");
	size -= getDirSize("/private/");
	
	size += getDirSize("/title/00030015/");
	
	return size;
}