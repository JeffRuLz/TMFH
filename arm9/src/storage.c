#include "storage.h"
#include "main.h"
#include "message.h"
#include <dirent.h>

#define TITLE_LIMIT 39

//printing
void printBytes(unsigned long long bytes)
{
	if (bytes < 1024)
		iprintf("%dB", (unsigned int)bytes);

	else if (bytes < 1024 * 1024)
		printf("%.2fKB", (float)bytes / 1024.f);

	else if (bytes < 1024 * 1024 * 1024)
		printf("%.2fMB", (float)bytes / 1024.f / 1024.f);

	else
		printf("%.2fGB", (float)bytes / 1024.f / 1024.f / 1024.f);
}

//progress bar
static int lastBars = 0;

void printProgressBar(float percent)
{
	if (percent < 0.f) percent = 0.f;
	if (percent > 1.f) percent = 1.f;

	int bars = (int)(30.f * percent);

	//skip redundant prints
	if (bars != lastBars)
	{
		consoleSelect(&topScreen);

		//Print frame
		if (lastBars <= 0)
		{
			iprintf("\x1b[23;0H[");
			iprintf("\x1b[23;31H]");
		}

		//Print bars
		if (bars > 0)
		{
			for (int i = 0; i < bars; i++)
				iprintf("\x1b[23;%dH|", 1 + i);
		}

		lastBars = bars;
	}	
}

void clearProgressBar()
{
	lastBars = 0;
	consoleSelect(&topScreen);
	iprintf("\x1b[23;0H                                ");
}

//files
bool fileExists(char const* path)
{
	if (!path) return false;

	FILE* f = fopen(path, "rb");
	if (!f)
		return false;

	fclose(f);
	return true;
}

bool copyFile(char const* in, char const* out)
{
	if (!in || !out) return false;

	FILE* fin = fopen(in, "rb");
	FILE* fout = fopen(out, "wb");

	if (!fin || !fout)
	{
		fclose(fin);
		fclose(fout);
		return false;
	}
	else
	{
		consoleSelect(&topScreen);

		int bytesRead;
		int totalBytesRead = 0;
		int fileSize = getFileSize(fin);

		#define BUFF_SIZE 128 //Arbitrary. A value too large freezes the ds.
		char* buffer = (char*)malloc(BUFF_SIZE);

		while (1)
		{
			bytesRead = fread(buffer, 1, BUFF_SIZE, fin);
			fwrite(buffer, bytesRead, 1, fout);

			totalBytesRead += bytesRead;
			printProgressBar( ((float)totalBytesRead / (float)fileSize) );

			if (bytesRead != BUFF_SIZE)
				break;
		}

		clearProgressBar();
		consoleSelect(&bottomScreen);

		free(buffer);
	}

	fclose(fin);
	fclose(fout);

	return true;
}

unsigned long long getFileSize(FILE* f)
{
	if (!f) return 0;

	fseek(f, 0, SEEK_END);
	unsigned long long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	return size;
}

unsigned long long getFileSizePath(char const* path)
{
	if (!path) return 0;

	FILE* f = fopen(path, "rb");
	unsigned long long size = getFileSize(f);
	fclose(f);

	return size;
}

bool padFile(char const* path, int size)
{
	if (!path) return false;

	FILE* f = fopen(path, "ab");
	if (!f)
	{
		return false;
	}
	else
	{
		char zero = 0;
		fwrite(&zero, size, 1, f);
	}

	fclose(f);
	return true;
}

//directories
bool dirExists(char const* path)
{
	if (!path) return false;

	DIR* dir = opendir(path);

	if (!dir)
		return false;

	closedir(dir);
	return true;
}

bool copyDir(char const* src, char const* dst)
{
	if (!src || !dst) return false;

//	iprintf("copyDir\n%s\n%s\n\n", src, dst);

	bool result = true;

	DIR* dir = opendir(src);
	struct dirent* ent;

	if (!dir)
	{
		return false;
	}
	else
	{
		while ( (ent = readdir(dir)) )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
			{
				char* dsrc = (char*)malloc(strlen(src) + strlen(ent->d_name) + 1);
				sprintf(dsrc, "%s/%s", src, ent->d_name);

				char* ddst = (char*)malloc(strlen(dst) + strlen(ent->d_name) + 1);
				sprintf(ddst, "%s/%s", dst, ent->d_name);

				mkdir(ddst, 0777);
				if (!copyDir(dsrc, ddst))
					result = false;

				free(ddst);
				free(dsrc);
			}
			else
			{
				char* fsrc = (char*)malloc(strlen(src) + strlen(ent->d_name) + 1);
				sprintf(fsrc, "%s/%s", src, ent->d_name);

				char* fdst = (char*)malloc(strlen(dst) + strlen(ent->d_name) + 1);
				sprintf(fdst, "%s/%s", dst, ent->d_name);

//				iprintf("%s\n%s\n\n", fsrc, fdst);
				iprintf("%s...", fdst);
				if(!copyFile(fsrc, fdst))
				{
					iprintf("Fail\n");
					result = false;
				}
				else
				{
					iprintf("Done\n");
				}

				free(fdst);
				free(fsrc);
			}
		}
	}

	closedir(dir);
	return result;
}

bool deleteDir(char const* path)
{
	if (!path) return false;

	if (strcmp("/", path) == 0)
	{
		//oh fuck no
		return false;
	}

	bool result = true;

	DIR* dir = opendir(path);
	struct dirent* ent;

	if (!dir)
	{
		result = false;
	}
	else
	{
		while ( (ent = readdir(dir)) )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
			{
				//Delete directory
				char subpath[512];
				sprintf(subpath, "%s/%s", path, ent->d_name);

				if (!deleteDir(subpath))
					result = false;
			}
			else
			{
				//Delete file
				char fpath[512];
				sprintf(fpath, "%s/%s", path, ent->d_name);

				iprintf("%s...", fpath);
				if (remove(fpath) != 0)
				{
					iprintf("Fail\n");
					result = false;
				}
				else
				{
					iprintf("Done\n");
				}
			}
		}
	}

	closedir(dir);

	iprintf("%s...", path);
	if (remove(path) != 0)
	{
		iprintf("Fail\n");
		result = false;
	}
	else
	{
		iprintf("Done\n");
	}

	return result;
}

unsigned long long getDirSize(const char* path)
{
	if (!path) return 0;

	unsigned long long size = 0;
	DIR* dir = opendir(path);
	struct dirent* ent;

	if (dir)
	{
		while ((ent = readdir(dir)))
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

//home menu
int getMenuSlots()
{
	//Assume the home menu has a hard limit on slots
	//Find a better way to do this
	return TITLE_LIMIT;
}

int getMenuSlotsFree()
{
	//Get number of open menu slots by subtracting the number of directories in the title folders
	//Find a better way to do this
	const int NUM_OF_DIRS = 3;
	const char* dirs[] = {
		"00030004",
		"00030005",
		"00030015"
	};

	int freeSlots = getMenuSlots();
	
	DIR* dir;
	struct dirent* ent;	
	
	for (int i = 0; i < NUM_OF_DIRS; i++)
	{
		char path[256];
		sprintf(path, "/title/%s", dirs[i]);
		
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

//SD card
bool sdIsInserted()
{
	//Find a better way to do this.
	return true;
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

//internal storage
int getDsiSize()
{
	//The DSi has 256MB of internal storage. Some is unavailable and used by other things.
	//Find a better way to do this
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