#include "main.h"
#include "app.h"
#include "menu.h"
#include "message.h"
#include "storage.h"
#include <dirent.h>

enum {
	TITLE_MENU_BACKUP,
	TITLE_MENU_DELETE,
	TITLE_MENU_BACK
};

static void generateList(Menu* m);
static void printItem(Menu* m);
static int subMenu();
static void backup(Menu* m);
static bool delete(Menu* m);

void titleMenu()
{
	Menu* m = newMenu();
	generateList(m);

	//no titles
	if (m->itemCount <= 0)
	{
		messageBox("No titles found.");
	}
	else
	{
		while (1)
		{
			swiWaitForVBlank();
			scanKeys();

			if (moveCursor(m))
			{
				if (m->changePage != 0)
					generateList(m);

				printMenu(m);
				printItem(m);
			}

			if (keysDown() & KEY_B || m->itemCount <= 0)
				break;

			else if (keysDown() & KEY_A)
			{
				switch (subMenu())
				{
					case TITLE_MENU_BACKUP:
						backup(m);
						break;

					case TITLE_MENU_DELETE:
					{
						if (delete(m))
						{
							resetMenu(m);
							generateList(m);
						}					
					}
					break;
				}
				
				printMenu(m);
			}
		}
	}

	freeMenu(m);
}

static void generateList(Menu* m)
{
	if (!m) return;

	const int NUM_OF_DIRS = 3;
	const char* dirs[] = {
		"00030004",
		"00030005",
		"00030015"
	};

	//Reset menu
	clearMenu(m);

	m->page += sign(m->changePage);
	m->changePage = 0;

	bool done = false;
	int count = 0;	//used to skip to the right page

	//search each category directory /title/XXXXXXXX
	for (int i = 0; i < NUM_OF_DIRS && done == false; i++)
	{
		char* dirPath = (char*)malloc(strlen(dirs[i])+10);
		sprintf(dirPath, "/title/%s", dirs[i]);

		struct dirent* ent;
		DIR* dir = opendir(dirPath);

		if (dir)
		{
			while ( (ent = readdir(dir)) && done == false)
			{
				if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
					continue;

				if (ent->d_type == DT_DIR)
				{
					//scan content folder /title/XXXXXXXX/content
					char* contentPath = (char*)malloc(strlen(dirPath) + strlen(ent->d_name) + 20);
					sprintf(contentPath, "%s/%s/content", dirPath, ent->d_name);

					struct dirent* subent;
					DIR* subdir = opendir(contentPath);

					if (subdir)
					{
						while ( (subent = readdir(subdir)) && done == false)
						{
							if (strcmp(".", subent->d_name) == 0 || strcmp("..", subent->d_name) == 0)
								continue;

							if (subent->d_type != DT_DIR)
							{
								//found .app file
								if (strstr(subent->d_name, ".app") != NULL)
								{
									//current item is not on page
									if (count < m->page * ITEMS_PER_PAGE)
										count += 1;
									
									else
									{
										if (m->itemCount >= ITEMS_PER_PAGE)
											done = true;
										
										else
										{
											//found requested title
											char* path = (char*)malloc(strlen(contentPath) + strlen(subent->d_name) + 10);
											sprintf(path, "%s/%s", contentPath, subent->d_name);

											char title[128];
											getAppTitle(path, title);

											addMenuItem(m, title, path);

											free(path);
										}																			
									}
								}
							}
						}
					}

					closedir(subdir);
					free(contentPath);			
				}
			}
		}

		closedir(dir);
		free(dirPath);
	}

	m->nextPage = done;

	if (m->cursor >= m->itemCount)
		m->cursor = m->itemCount - 1;

	printItem(m);
	printMenu(m);
}

static void printItem(Menu* m)
{
	if (!m) return;
	printAppInfo(m->items[m->cursor]);
}

static int subMenu()
{
	int result = -1;

	Menu* m = newMenu();

	addMenuItem(m, "Backup", NULL);
	addMenuItem(m, "Delete", NULL);
	addMenuItem(m, "Back - [B]", NULL);

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

	freeMenu(m);
	return result;
}

static void backup(Menu* m)
{
	char* fpath = m->items[m->cursor];
	char* backname = NULL;

	{
		//make backup folder name
		char label[16];
		getAppLabel(fpath, label);

		char gamecode[5];
		getGameCode(fpath, gamecode);

		backname = (char*)malloc(strlen(label) + strlen(gamecode) + 16);
		sprintf(backname, "%s-%s", label, gamecode);

		//make sure dir is unused
		char* dstpath = (char*)malloc(strlen(BACKUP_PATH) + strlen(backname) + 32);
		sprintf(dstpath, "%s/%s", BACKUP_PATH, backname);

		int try = 1;
		while (dirExists(dstpath))
		{
			try += 1;
			sprintf(backname, "%s-%s(%d)", label, gamecode, try);
			sprintf(dstpath, "%s/%s", BACKUP_PATH, backname);
		}

		free(dstpath);
	}

	bool choice = NO;
	{
		char str[] = "Are you sure you want to backup\n";
		char* msg = (char*)malloc(strlen(str) + strlen(backname) + 1);
		sprintf(msg, "%s%s", str, backname);
		
		choice = choiceBox(msg);

		free(msg);
	}

	if (choice == YES)
	{
		u32 tid_low = 1;
		u32 tid_high = 1;
		getTid(fpath, &tid_low, &tid_high);

		char* srcpath = (char*)malloc(strlen("/title/") + 32);
		sprintf(srcpath, "/title/%08x/%08x", (unsigned int)tid_high, (unsigned int)tid_low);

		if (getSDCardFree() < getDirSize(srcpath))
		{
			messageBox("Not enough space on SD card.");
		}
		else
		{
			char* dstpath = (char*)malloc(strlen(BACKUP_PATH) + strlen(backname) + 8);
			sprintf(dstpath, "%s/%s", BACKUP_PATH, backname);

			//create dirs
			mkdir(BACKUP_PATH, 0777); 	// /titlebackup
			mkdir(dstpath, 0777);		// /titlebackup/App Name - XXXX
			free(dstpath);

			dstpath = (char*)malloc(strlen(BACKUP_PATH) + strlen(backname) + 16);
			sprintf(dstpath, "%s/%s/%08x", BACKUP_PATH, backname, (unsigned int)tid_high);

			mkdir(dstpath, 0777);		// /titlebackup/App Name - XXXX/tid_high
			free(dstpath);

			dstpath = (char*)malloc(strlen(BACKUP_PATH) + strlen(backname) + 32);
			sprintf(dstpath, "%s/%s/%08x/%08x", BACKUP_PATH, backname, (unsigned int)tid_high, (unsigned int)tid_low);

			mkdir(dstpath, 0777);		// /titlebackup/App Name - XXXX/tid_high/tid_low

//			iprintf("dst %s\nsrc %s", dstpath, srcpath);
//			keyWait(KEY_A);

			clearScreen(&bottomScreen);

			if (!copyDir(srcpath, dstpath))
				messagePrint("\nBackup error.");
			else
				messagePrint("\nBackup finished.");

			free(dstpath);
		}

		free(srcpath);
	}

	free(backname);
}

static bool delete(Menu* m)
{
	if (!m) return false;

	char* fpath = m->items[m->cursor];
	
	bool result = false;
	bool choice = NO;
	{
		//get app title
		char title[128];
		getAppTitle(m->items[m->cursor], title);

		char str[] = "Are you sure you want to delete\n";
		char* msg = (char*)malloc(strlen(str) + strlen(title) + 8);
		sprintf(msg, "%s%s", str, title);
		
		choice = choiceBox(msg);

		free(msg);
	}

	if (choice == YES)
	{
		if (!fpath)
		{
			messageBox("Failed to delete title.\n");
		}
		else
		{
			char dirPath[64];
			sprintf(dirPath, "%.25s", fpath);

			if (!dirExists(dirPath))
			{
				messageBox("Failed to delete title.\n");
			}
			else
			{
				clearScreen(&bottomScreen);

				if (deleteDir(dirPath))
				{
					result = true;
					messagePrint("\nTitle deleted.\n");
				}
				else
				{
					messagePrint("\nTitle could not be deleted.\n");
				}
			}
		}
	}

	return result;
}