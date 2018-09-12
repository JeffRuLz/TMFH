#include "menus.h"
#include "storage.h"
#include "maketmd.h"
#include <dirent.h>

static int cursor = 0;
static int scrolly = 0;
static int numberOfTitles = 0;

static void moveCursor(int dir);

static void printList();
static void printFileInfoNum(int num);

static int getNumberOfTitles();
//static void getGameTitle(char* title, FILE* f);
static int getFile(char* dest, int num, int fullpath);

static void subMenu();
static void install(char* fpath);

void installMenu()
{
	cursor = 0;
	scrolly = 0;
	numberOfTitles = getNumberOfTitles();

	consoleSelect(&topScreen);
	consoleClear();

	consoleSelect(&bottomScreen);
	consoleClear();

	//No titles error
	if (numberOfTitles == 0)
	{
		iprintf("No files found.\n");
		iprintf("Place .nds(dsi) or .app files in %s\n", ROM_PATH);
		iprintf("\nBack - B\n");

		keyWait(KEY_B | KEY_A | KEY_START);
		return;
	}

	//Print data
	consoleSelect(&topScreen);
	printFileInfoNum(cursor);

	consoleSelect(&bottomScreen);
	printList();	

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		int thisCursor = cursor;
		int thisscrolly = scrolly;

		//Clear cursor
		consoleSelect(&bottomScreen);
		iprintf("\x1b[%d;0H ", cursor - scrolly);

		//Move cursor
		if (keysDown() & KEY_DOWN)
			moveCursor(1);

		if (keysDown() & KEY_UP)
			moveCursor(-1);

		if (keysDown() & KEY_RIGHT)
		{
			repeat (10)
				moveCursor(1);
		}		

		if (keysDown() & KEY_LEFT)
		{
			repeat (10)
				moveCursor(-1);
		}		

		//Refresh screens
		if (thisCursor != cursor)
		{
			consoleSelect(&topScreen);
			consoleClear();

			printFileInfoNum(cursor);			
		}

		if (thisscrolly != scrolly)
		{
			consoleSelect(&bottomScreen);
			consoleClear();

			printList();
		}

		//Print cursor
		consoleSelect(&bottomScreen);
		iprintf("\x1b[%d;0H>", cursor - scrolly);

		//
		if (keysDown() & KEY_B)
			break;
		else if (keysDown() & KEY_A)
		{
			subMenu();

			consoleSelect(&topScreen);
			printFileInfoNum(cursor);

			consoleSelect(&bottomScreen);
			printList();
		}
	}
}

void moveCursor(int dir)
{
	cursor += sign(dir);

	if (cursor < 0)
		cursor = 0;

	if (cursor >= numberOfTitles - 1)
		cursor = numberOfTitles - 1;

	if (cursor - scrolly >= 23)
		scrolly += 1;

	if (cursor - scrolly < 0)
		scrolly -= 1;
}

void printList()
{
	consoleClear();
	
	for (int i = scrolly; i < scrolly + 23; i++)
	{
		char str[256];
		if (getFile(str, i, 0) == 1)
			iprintf(" %.30s\n", str);
	}

	//Scroll arrows
	if (scrolly > 0)
		iprintf("\x1b[0;31H^");

	if (scrolly < numberOfTitles - 23)
		iprintf("\x1b[22;31Hv");
}

void printFileInfoNum(int num)
{
	consoleClear();

	char path[256];
	if (getFile(path, num, 1) == 1)
		printFileInfo(path);
}

int getNumberOfTitles()
{
	DIR* dir;
	struct dirent* ent;
	int count = 0;

	dir = opendir(ROM_PATH);

	if (dir)
	{
		while ( (ent = readdir(dir)) != NULL )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type != DT_DIR)
			{
				if (strstr(ent->d_name, ".nds") != NULL || strstr(ent->d_name, ".app") != NULL)
					count++;
			}
		}
	}

	closedir(dir);
	return count;
}
/*
void getGameTitle(char* title, FILE* f)
{
	tDSiHeader header;
	tNDSBanner banner;

	fseek(f, 0, SEEK_SET);
	fread(&header, sizeof(tDSiHeader), 1, f);
	fseek(f, header.ndshdr.bannerOffset, SEEK_SET);
	fread(&banner, sizeof(tNDSBanner), 1, f);

//	iprintf("\t%s\n", header.ndshdr.gameTitle);
//	iprintf("\t%s\n", (char*)banner.titles[0]);

	int line = 0;
	for (int i = 0; i < 64; i++)
	{
		char c = banner.titles[0][i];

		if (c == '\n')
		{
			if (line == 0)
			{
				title[i] = ' ';
				line = 1;
			}
			else
			{
				title[i] = '\0';
				break;
			}
		}
		else
			title[i] = c;
	}

	title[64] = '\0';
}
*/

int getFile(char* dest, int num, int fullpath)
{
	DIR* dir;
	struct dirent* ent;
	int result = 0;

	dir = opendir(ROM_PATH);

	if (dir)
	{
		int count = 0;

		while ( (ent = readdir(dir)) != NULL )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type != DT_DIR)
			{
				if(strstr(ent->d_name, ".nds") != NULL || strstr(ent->d_name, ".app") != NULL)
				{
					if (count < num)
					{
						count++;
						continue;
					}
					else
					{
						if (fullpath == 0)
							sprintf(dest, "%s", ent->d_name);
						else
							sprintf(dest, "%s%s", ROM_PATH, ent->d_name);

						result = 1;
						break;
					}
				}
			}
		}
	}

	closedir(dir);
	return result;
}

//
static int subCursor = 0;

enum {
	INSTALL_MENU_INSTALL,
	INSTALL_MENU_DELETE,
	INSTALL_MENU_BACK
};

void subMenu()
{
	bool printMenu = true;
	subCursor = 0;	

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (printMenu == true)
		{
			printMenu = false;

			consoleSelect(&bottomScreen);
			consoleClear();

			iprintf("\tInstall\n");
			iprintf("\tDelete\n");
			iprintf("\tBack - B\n");
		}

		//Clear cursor
		iprintf("\x1b[%d;0H ", subCursor);

		//Move cursor
		if (keysDown() & KEY_DOWN)
		{
			if (subCursor < INSTALL_MENU_BACK)
				subCursor++;
		}

		if (keysDown() & KEY_UP)
		{
			if (subCursor > 0)
				subCursor--;
		}

		//Reprint cursor
		iprintf("\x1b[%d;0H>", subCursor);

		//
		if (keysDown() & KEY_B)
			break;

		else if (keysDown() & KEY_A)
		{
			if (subCursor == INSTALL_MENU_INSTALL)
			{
				char fpath[256];
				getFile(fpath, cursor, 1);

				char msg[512+1];
				msg[512] = '\0';
				sprintf(msg, "Are you sure you want to install\n%s\n", fpath);

				if (choiceBox(msg) == YES)
					install(fpath);
				
				break;
			}

			else if (subCursor == INSTALL_MENU_DELETE)
			{
				char fpath[256];
				getFile(fpath, cursor, 1);

				char msg[512+1];
				msg[512] = '\0';
				sprintf(msg, "Are you sure you want to delete\n%s\n", fpath);

				if (choiceBox(msg) == YES)
				{
					if (remove(fpath) != 0)
						messageBox("File could not be deleted.");

					else
						messageBox("File deleted.");

					//Reset
					cursor = 0;
					scrolly = 0;
					numberOfTitles = getNumberOfTitles();
					
					break;
				}				
				else
				{
					printMenu = true;
				}
			}

			else if (subCursor == INSTALL_MENU_BACK)
			{
				break;
			}
		}
	}	
}

void install(char* fpath)
{
	consoleSelect(&bottomScreen);
	consoleClear();

	iprintf("Installing %s\n", fpath); swiWaitForVBlank();
	
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		iprintf("Error: could not open file.\nPress B to exit.\n");
		keyWait(KEY_A | KEY_B);
	}
	else
	{
		//Load header		
		tDSiHeader header;
		tNDSBanner banner;	

		{
			fseek(f, 0, SEEK_SET);
			fread(&header, sizeof(tDSiHeader), 1, f);
			fseek(f, header.ndshdr.bannerOffset, SEEK_SET);
			fread(&banner, sizeof(tNDSBanner), 1, f);
		}

		//Print file size
		int fileSize = -1;

		{
			iprintf("File Size: "); swiWaitForVBlank();

			fileSize = getFileSize(f);
			
			printBytes(fileSize);
			printf("\n");
		}

		//Do not want file opened anymore
		fclose(f);

		//SD card check
		{
			iprintf("Enough room on SD card?..."); swiWaitForVBlank();

			if (getSDCardFree() < fileSize)
			{
				iprintf("No\n");
				goto error;
			}

			iprintf("Yes\n");
		}

		//DSi storage check
		{
			iprintf("Enough room on DSi?..."); swiWaitForVBlank();

			if (getDsiFree() < fileSize)
			{
				iprintf("No\n"); swiWaitForVBlank();
				goto error;				
			}
			
			iprintf("Yes\n"); swiWaitForVBlank();
		}

		//Menu slot check
		{
			iprintf("Open DSi menu slot?..."); swiWaitForVBlank();
			
			if (getMenuSlotsFree() <= 0)
			{
				iprintf("No\n"); swiWaitForVBlank();
				goto error;	
			}

			iprintf("Yes\n"); swiWaitForVBlank();
		}

		//Create title directory
		char titleID[8+1];
		char dirPath[256];

		{
			sprintf(titleID, "%08x", (unsigned int)header.tid_low);			
			sprintf(dirPath, "/title/%08x/%s", (unsigned int)header.tid_high, titleID);

			//iprintf("Creating dir\n%s\n", dirPath); swiWaitForVBlank();
		}

		//Check if title is already installed
		{
			DIR* dir = opendir(dirPath);

			if (dir)
			{
				closedir(dir);

				iprintf("Title %s is already used.\nInstall anyway?\n", titleID);
				iprintf("Yes - A\nNo  - B\n");

				while (1)
				{
					swiWaitForVBlank();
					scanKeys();

					if (keysDown() & KEY_A)
						break;

					if (keysDown() & KEY_B)
						goto complete;
				}
			}
		}

		//
		mkdir(dirPath, 0777);

		//Content folder
		{
			char contentPath[256];
			sprintf(contentPath, "%s/content", dirPath);

			//iprintf("Creating dir\n%s\n", contentPath); swiWaitForVBlank();
			mkdir(contentPath, 0777);

			//Create 0000000.app
			//Does 00000000 always work?
			{
				char appPath[256];
				sprintf(appPath, "%s/00000000.app", contentPath);

				iprintf("Creating 00000000.app..."); swiWaitForVBlank();
				
				if (copyFile(fpath, appPath) == 0)
				{
					iprintf("Failed\n");
					goto error;
				}

				iprintf("Done\n");

				//Make TMD
				{
					char tmdPath[256];
					sprintf(tmdPath, "%s/title.tmd", contentPath);

					if (maketmd(appPath, tmdPath) != 0)
						goto error;
				}
			}
		}

		//Data folder
		{
			char dataPath[256];
			sprintf(dataPath, "%s/data", dirPath);

			//iprintf("Creating dir\n%s\n", dataPath); swiWaitForVBlank();
			mkdir(dataPath, 0777);

			//If needed, create public.sav
			if (header.public_sav_size > 0)
			{
				char publicPath[512];
				sprintf(publicPath, "%s/public.sav", dataPath);

				iprintf("Creating public.sav..."); swiWaitForVBlank();

				FILE* file = fopen(publicPath, "wb");

				if (!file)
					iprintf("Failed\n");

				else
				{
					char num = 0;

					repeat (header.public_sav_size)
						fwrite(&num, 1, 1, f);

					iprintf("Done\n");
				}

				fclose(file);		
			}

			//If needed, create private.sav
			if (header.private_sav_size > 0)
			{
				char privatePath[512];
				sprintf(privatePath, "%s/private.sav", dataPath);

				iprintf("Creating private.sav..."); swiWaitForVBlank();

				FILE* file = fopen(privatePath, "wb");

				if (!file)
					iprintf("Failed\n");

				else
				{
					char num = 0;

					repeat (header.private_sav_size)
						fwrite(&num, 1, 1, f);

					iprintf("Done\n");
				}

				fclose(file);	
			}

			//If needed, create banner.sav
			if (header.appflags & 0x4)
			{
				char bannerPath[512];
				sprintf(bannerPath, "%s/banner.sav", dataPath);

				iprintf("Creating banner.sav..."); swiWaitForVBlank();

				FILE* file = fopen(bannerPath, "wb");

				if (!file)
					iprintf("Failed\n");

				else
				{
					char num = 0;

					repeat (1024*16) //Is banner.sav always 16kb?
						fwrite(&num, 1, 1, f); 

					iprintf("Done\n");
				}

				fclose(file);	
			}
		}

		iprintf("\nInstallation complete.\nPress B to exit.\n");
		keyWait(KEY_A | KEY_B);
	}

complete:
	fclose(f);
	return;

error:
	fclose(f);

	iprintf("\nInstallation failed.\nPress B to exit.\n");
	keyWait(KEY_A | KEY_B);

	return;
}