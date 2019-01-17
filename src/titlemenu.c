#include "menu.h"
#include "main.h"
#include "storage.h"
#include <dirent.h>

static void generateList(Menu* m);
static void printItem(Menu* m);

static int subMenu();
static void dump(Menu* m);
static void delete(Menu* m);
//static void backupData(Menu* m);
//static void restoreData(Menu* m);

enum {
//	TITLE_MENU_BACKUP,
	TITLE_MENU_DUMP,
	TITLE_MENU_DELETE,
	TITLE_MENU_BACK
};

void titleMenu()
{
	Menu* m = (Menu*)malloc(sizeof(Menu));

	clearScreen(&topScreen);
	clearScreen(&bottomScreen);

	generateList(m);

	//No titles error
	if (getNumberOfMenuItems(m) <= 0)
	{
		consoleClear();

		iprintf("No titles found.\n");
		iprintf("Back - B\n");

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

		else if (keysDown() & KEY_A)
		{
			switch (subMenu())
			{
/*				case TITLE_MENU_BACKUP:
					backup(m);
					break;
*/
				case TITLE_MENU_DUMP:
					dump(m);
					break;

				case TITLE_MENU_DELETE:
					delete(m);					
					break;
			}
			
			printMenu(m);
		}
	}

	free(m);
}

#define NUM_OF_DIRECTORIES 3
static const char* directories[] = {
	"00030004",
	"00030005",
	"00030015"
};

//mode = 0: add items to menu
//mode = 1: return full path of menu item
static bool _walkList(Menu* m, int mode, char* out)
{
	//Skip if no menu
	if (m == NULL)
		return false;

	//Reset menu
	if (mode == 0)
		clearMenu(m);

	bool result = false;
	int count = 0;

	//Scan choice title directories
	for (int i = 0; i < NUM_OF_DIRECTORIES && result == false; i++)
	{
		DIR* dir;
		struct dirent* ent;		

		char dirPath[256];
		sprintf(dirPath, "/title/%s", directories[i]);

		dir = opendir(dirPath);

		if (dir)
		{
			while ( (ent = readdir(dir)) != NULL && result == false)
			{
				if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
					continue;

				if (ent->d_type == DT_DIR)
				{
					//Scan content folder
					char contentPath[384];
					sprintf(contentPath, "%s/%s/content", dirPath, ent->d_name);

					DIR* subdir;
					struct dirent* subent;

					subdir = opendir(contentPath);

					if (subdir)
					{
						while ( (subent = readdir(subdir)) != NULL && result == false)
						{
							if (strcmp(".", subent->d_name) == 0 || strcmp("..", subent->d_name) == 0)
								continue;

							if (subent->d_type != DT_DIR)
							{								
								if (strstr(subent->d_name, ".app") != NULL)
								{
									//Found requested title
									char path[384];
									sprintf(path, "%s/%s", contentPath, subent->d_name);

									//Generate list
									if (mode == 0)
									{
										FILE* f = fopen(path, "rb");

										if (f)
										{
											tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));
											tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

											fread(header, sizeof(tNDSHeader), 1, f);
											fseek(f, header->bannerOffset, SEEK_SET);
											fread(banner, sizeof(tNDSBanner), 1, f);

											char tstr[128+1];
											tstr[128] = '\0';

											for (int i = 0; i < 128; i++)
											{
												char c = banner->titles[1][i];
												
												//Replace new line with space
												if (c == '\n')
													c = ' ';

												tstr[i] = c;
											}

											free(banner);
											free(header);

											addMenuItem(m, tstr);
										}

										fclose(f);
									}

									//Get item file name
									else if (mode == 1)
									{
										if (count < m->cursor)
											count++;
										else
										{
											sprintf(out, "%s", path);
											result = true;
										}	
									}
								}
							}
						}
					}

					closedir(subdir);				
				}
			}
		}

		closedir(dir);
	}

	return result;
}

void generateList(Menu* m)
{
	if (m == NULL) return;

	clearScreen(&bottomScreen);
	iprintf("Gathering files...\n"); swiWaitForVBlank();

	clearMenu(m);
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

//	addMenuItem(m, "Backup");
	addMenuItem(m, "Dump");
	addMenuItem(m, "Delete");
	addMenuItem(m, "Back");

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

static void dump(Menu* m)
{
	char fpath[256];	

	if (_walkList(m, 1, fpath) == false)
	{
		messageBox("Failed to dump title.\n");
	}
	else
	{
		FILE* f = fopen(fpath, "rb");

		if (!f)
			messageBox("Can not dump title.\n");

		else
		{
			unsigned long long fsize = getFileSize(f);

			if (fsize > getSDCardFree())
			{
				messageBox("Not enough free space on SD card.\n");
			}
			else
			{
				tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));
				tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

				fread(header, sizeof(tNDSHeader), 1, f);
				fseek(f, header->bannerOffset, SEEK_SET);
				fread(banner, sizeof(tNDSBanner), 1, f);
				
				fclose(f);

				char outpath[256];
				sprintf(outpath, "%s%.12s - %.4s.nds", ROM_PATH, header->gameTitle, header->gameCode);

				bool choice = NO;
				{
					char msg[512];
					sprintf(msg, "Dump title to\n%s\n", outpath);
					choice = choiceBox(msg);
				}

				if (choice == YES)
				{
					if (copyFile(fpath, outpath) == 1)
						messageBox("Title saved.\n");
					else
						messageBox("Title dump failed.\n");
				}

				free(banner);
				free(header);
			}
		}

		fclose(f);
	}
}

static void delete(Menu* m)
{
	char fpath[256];	

	if (_walkList(m, 1, fpath) == false)
	{
		messageBox("Failed to delete title.\n");
	}
	else
	{
		bool choice = NO;

		{
			//Get title name
			char title[128+1];
			title[128] = '\0';

			FILE* f = fopen(fpath, "rb");

			if (!f)
			{
				messageBox("Failed to delete title.\n");
			}
			else
			{
				tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));
				tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

				fseek(f, 0, SEEK_SET);
				fread(header, sizeof(tNDSHeader), 1, f);
				fseek(f, header->bannerOffset, SEEK_SET);
				fread(banner, sizeof(tNDSBanner), 1, f);					

				for (int i = 0; i < 128; i++)					
					title[i] = (char)banner->titles[1][i];

				free(banner);
				free(header);
			}

			fclose(f);		

			char msg[512];
			sprintf(msg, "Are you sure you want to delete\n%s", title);
			choice = choiceBox(msg);
		}

		if (choice == YES)
		{
			char dirpath[256];
			sprintf(dirpath, "%.25s", fpath);

			if (deleteDir(dirpath) == 1)
				messageBox("Title deleted.\n");
			else
				messageBox("Title could not be deleted.\n");

			generateList(m);
			printItem(m);
		}
	}

	printMenu(m);
}
/* Incomplete
static void backup(Menu* m)
{
	char msg[512];
	char dirPath[256];
	sprintf(dirPath, "/dsisave/%s", title);
	sprintf(msg, "Backup data to\n%s", dirPath);

	if (choiceBox(msg) == YES)
	{
		mkdir(dirPath, 0777);

		FILE* f = fopen(path, "rb");

		if (f)
		{
			tDSiHeader header;
			fread(&header, sizeof(tDSiHeader), 1, f);
			fclose(f);

			char titleID[8+1];
			sprintf(titleID, "%08x", (unsigned int)header.tid_low);

			DIR* dir;
			struct dirent* ent;

			dir = opendir(ROM_PATH);

			closedir(dir);
		}

		fclose(f);
}
*/