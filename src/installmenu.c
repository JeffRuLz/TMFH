#include "menu.h"
#include "main.h"
#include "storage.h"
#include "titles.h"
#include "maketmd.h"
#include <dirent.h>


enum {
	INSTALL_MENU_INSTALL,
	INSTALL_MENU_DELETE,
	INSTALL_MENU_BACK
};

static void generateList(Menu* m);
static void printItem(Menu* m);

static int subMenu();
static void install(Menu* m);
static void delete(Menu* m);


void installMenu()
{
	Menu* m = (Menu*)malloc(sizeof(Menu));
	
	clearMenu(m);
	generateList(m);

	//No files found
	if (getNumberOfMenuItems(m) <= 0)
	{
		consoleSelect(&bottomScreen);
		consoleClear();

		iprintf("No files found.\n");
		iprintf("Place .nds, .app, or .dsi files in %s\n", ROM_PATH);
		iprintf("\nBack - B\n");

		keyWait(KEY_B | KEY_A | KEY_START);
		return;
	}

	//Print data
	printItem(m);
	printMenu(m);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
		{
			printItem(m);
			printMenu(m);
		}		

		if (keysDown() & KEY_B)
			break;

		//Selection
		else if (keysDown() & KEY_A)
		{			
			switch (subMenu())
			{
				case INSTALL_MENU_INSTALL:
					install(m);
					break;

				case INSTALL_MENU_DELETE:
					delete(m);
					break;

				case INSTALL_MENU_BACK:					
					break;
			}

			printMenu(m);
		}
	}

	free(m);
}

//mode = 0: add items to menu
//mode = 1: return full path of item num to out
static bool _walkList(Menu* m, int mode, char* out)
{
	//Skip if no menu
	if (m == NULL)
		return false;

	//Reset menu
	if (mode == 0)
		clearMenu(m);

	//
	DIR* dir;
	struct dirent* ent;
	bool result = false;

	dir = opendir(ROM_PATH);

	if (dir)
	{
		int count = 0;

		//Scan /dsi/
		while ( (ent = readdir(dir)) != NULL )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type != DT_DIR)
			{
				if (strstr(ent->d_name, ".nds") != NULL || strstr(ent->d_name, ".app") != NULL || strstr(ent->d_name, ".dsi") != NULL ||
					strstr(ent->d_name, ".NDS") != NULL || strstr(ent->d_name, ".APP") != NULL || strstr(ent->d_name, ".DSI") != NULL)
				{
					//Generate list
					if (mode == 0)
					{
						char name[128];
						sprintf(name, "%s", ent->d_name);

						addMenuItem(m, name);
					}

					//Get item file name
					else if (mode == 1)
					{
						if (count < m->cursor)
							count++;
						else
						{
							sprintf(out, "%s%s", ROM_PATH, ent->d_name);

							result = true;
							goto endloop;
						}						
					}
				}
			}
		}
	}

endloop:
	closedir(dir);
	return result;
}

void generateList(Menu* m)
{
	if (m == NULL) return;

	consoleSelect(&bottomScreen);
	consoleClear();

	iprintf("Gathering files...\n");

	_walkList(m, 0, NULL);
}

static void printItem(Menu* m)
{
	if (m == NULL) return;

	char path[256];
	if (_walkList(m, 1, path) == true)
		printFileInfo(path);
}

//
int subMenu()
{
	int result = -1;

	Menu* m = (Menu*)malloc(sizeof(Menu));
	clearMenu(m);

	addMenuItem(m, "Install");
	addMenuItem(m, "Delete");
	addMenuItem(m, "Back - B");

	printMenu(m);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
			printMenu(m);

		if (keysDown() & KEY_B)
			break;

		else if (keysDown() & KEY_A)
		{
			result = m->cursor;
			break;
		}
	}

	free(m);

	return result;
}

void install(Menu* m)
{
	char fpath[256];	

	//Confirmation message
	{
		int choice = NO;	

		if (_walkList(m, 1, fpath) == true)
		{
			char msg[512];
			sprintf(msg, "Are you sure you want to install\n%s\n", fpath);
			choice = choiceBox(msg);
		}

		if (choice == NO)
			return;
	}

	//Start installation
	consoleSelect(&bottomScreen);
	consoleClear();

	iprintf("Installing %s\n", fpath); swiWaitForVBlank();
	
	tDSiHeader* header = (tDSiHeader*)malloc(sizeof(tDSiHeader));
	tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));
	
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		iprintf("Error: could not open file.\n");
		goto error;
	}
	else
	{
		bool patchHeader = false;

		//Read header and banner
		{
			fseek(f, 0, SEEK_SET);
			fread(header, sizeof(tDSiHeader), 1, f);

			fseek(f, header->ndshdr.bannerOffset, SEEK_SET);
			fread(banner, sizeof(tNDSBanner), 1, f);
		}

		//Patch homebrew roms if gameCode is #### or null
		if ((strcmp(header->ndshdr.gameCode, "####") == 0 && header->tid_low == 0x23232323) ||
			(!*header->ndshdr.gameCode && header->tid_low == 0))
		{
			iprintf("Patching header...");

			patchHeader = true;

			//Set as standard app
			header->tid_high = 0x00030004;

			//Give it a random game code
			do
			{				
				//First letter shouldn't be A
				do
				{
					for (int i = 0; i < 4; i++)
						header->ndshdr.gameCode[i] = 0x41 + (rand() % (26));
				}
				while (header->ndshdr.gameCode[0] == 'A');

				//Correct title id
				header->tid_low = gameCodeToTitleID(header->ndshdr.gameCode);
			}
			while (titleIsUsed(header->tid_low, header->tid_high) == true);

			//Fix header checksum
			header->ndshdr.headerCRC16 = swiCRC16(0xFFFF, header, 0x15E);
			
			//Fix RSA signature
			u8 buffer[20];
			swiSHA1Calc(&buffer, header, 0xE00);
			memcpy(&(header->rsa_signature[0x6C]), buffer, 20);

			iprintf("Done\n");
		}

		//Must be DSi rom
		//High title id must be one of three
		{
			if (header->tid_high != 0x00030004 &&
				header->tid_high != 0x00030005 &&
				header->tid_high != 0x00030015 &&
				header->tid_high != 0x00030017)
			{
				iprintf("Error: This is not a DSi rom.\n");
				goto error;	
			}
		}

		//Print file size
		unsigned long long fileSize = 0;	

		{
			iprintf("File Size: ");

			fileSize = getFileSize(f);
			
			printBytes(fileSize);
			printf("\n");
		}

		//Do not need file opened anymore
		fclose(f);

		//SD card check
		{
			iprintf("Enough room on SD card?...");

			if (getSDCardFree() < fileSize)
			{
				iprintf("No\n");
				goto error;
			}

			iprintf("Yes\n");
		}

		//DSi storage check
		{
			iprintf("Enough room on DSi?...");

			if (getDsiFree() < fileSize)
			{
				iprintf("No\n");
				if (choiceBox("Try installing anyway?") == NO)
					goto error;
			}
			else
			{
				iprintf("Yes\n");
			}
		}

		//Menu slot check
		{
			iprintf("Open DSi menu slot?...");
			
			if (getMenuSlotsFree() <= 0)
			{
				iprintf("No\n");
				if (choiceBox("Try installing anyway?") == NO)
					goto error;
			}
			else
			{
				iprintf("Yes\n");
			}			
		}

		//Create title directory
		char titleID[8+1];
		char dirPath[256];

		{
			sprintf(titleID, "%08x", (unsigned int)header->tid_low);			
			sprintf(dirPath, "/title/%08x/%s", (unsigned int)header->tid_high, titleID);
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

			mkdir(contentPath, 0777);

			//Create 0000000.app
			{
				char appPath[256];
				sprintf(appPath, "%s/00000000.app", contentPath);

				//Copy nds file to app
				{
					iprintf("Creating 00000000.app...");
					
					if (copyFile(fpath, appPath) == 0)
					{
						iprintf("Failed\n");
						goto error;
					}

					iprintf("Done\n");					
				}

				//Pad out banner if it is the last part of the file
				if (header->ndshdr.bannerOffset == fileSize - 0x1C00)
				{
					iprintf("Padding banner...");

					if (padFile(appPath, 0x7C0) == false)
						iprintf("Failed\n");
					else
						iprintf("Done\n");	
				}

				//Write new patched header
				if (patchHeader == true)
				{
					iprintf("Writing header...");
					
					FILE* f = fopen(appPath, "r+");

					if (!f)
						iprintf("Failed\n");

					else
					{
						fseek(f, 0, SEEK_SET);
						fwrite(header, sizeof(tDSiHeader), 1, f);

						iprintf("Done\n");
					}

					fclose(f);
				}				

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

			mkdir(dataPath, 0777);

			//If needed, create public.sav
			if (header->public_sav_size > 0)
			{
				char publicPath[512];
				sprintf(publicPath, "%s/public.sav", dataPath);

				{
					iprintf("Creating public.sav...");

					FILE* file = fopen(publicPath, "wb");

					if (!file)
						iprintf("Failed\n");

					else
					{
						char num = 0;

						repeat (header->public_sav_size)
							fwrite(&num, 1, 1, f);

						iprintf("Done\n");
					}

					fclose(file);
				}
			}

			//If needed, create private.sav
			if (header->private_sav_size > 0)
			{
				char privatePath[512];
				sprintf(privatePath, "%s/private.sav", dataPath);

				{
					iprintf("Creating private.sav...");

					FILE* file = fopen(privatePath, "wb");

					if (!file)
						iprintf("Failed\n");

					else
					{
						char num = 0;

						repeat (header->private_sav_size)
							fwrite(&num, 1, 1, f);

						iprintf("Done\n");
					}

					fclose(file);
				}
			}

			//If needed, create banner.sav
			if (header->appflags & 0x4)
			{
				char bannerPath[512];
				sprintf(bannerPath, "%s/banner.sav", dataPath);

				{
					iprintf("Creating banner.sav...");

					FILE* file = fopen(bannerPath, "wb");

					if (!file)
						iprintf("Failed\n");

					else
					{
						char num = 0;

						repeat (0x4000)
							fwrite(&num, 1, 1, f); 

						iprintf("Done\n");
					}

					fclose(file);
				}
			}
		}		

		//
		iprintf("\nInstallation complete.\nPress B to exit.\n");
		keyWait(KEY_A | KEY_B);		
	}

	goto complete;

error:
	iprintf("\nInstallation failed.\n\nPress B to exit.\n");
	keyWait(KEY_A | KEY_B);

complete:
	free(banner);
	free(header);

	fclose(f);
	return;
}

static void delete(Menu* m)
{
	char path[256];
	int choice = NO;

	if (_walkList(m, 1, path) == true)
	{
		{
			char msg[512];
			sprintf(msg, "Are you sure you want to delete\n%s\n", path);
			choice = choiceBox(msg);
		}

		if (choice == YES)
		{
			if (remove(path) != 0)
			{
				messageBox("File could not be deleted.");
			}
			else
			{
				messageBox("File deleted.");
				
				generateList(m);
				printItem(m);
			}
		}
	}

	printMenu(m);
}