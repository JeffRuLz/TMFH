#include "main.h"
#include "menu.h"
#include "storage.h"
#include "message.h"
#include <dirent.h>
#include <sys/stat.h>

enum {
	BACKUP_MENU_RESTORE,
	BACKUP_MENU_DELETE,
	BACKUP_MENU_BACK
};

static void generateList(Menu* m);
static int subMenu();
static void restore(Menu* m);
static bool delete(Menu* m);

void backupMenu()
{
	Menu* m = newMenu();
	generateList(m);

	//no files found
	if (m->itemCount <= 0)
	{
		messageBox("No backups found.\n");
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
			}

			if (keysDown() & KEY_B || m->itemCount <= 0)
				break;

			else if (keysDown() & KEY_A)
			{
				switch (subMenu())
				{
					case BACKUP_MENU_RESTORE:
						restore(m);
						break;

					case BACKUP_MENU_DELETE:
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

static int subMenu()
{
	int result = -1;

	Menu* m = newMenu();

	addMenuItem(m, "Restore", NULL);
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

static void generateList(Menu* m)
{
	if (!m) return;

	//reset menu
	clearMenu(m);

	m->page += sign(m->changePage);
	m->changePage = 0;

	bool done = false;	

	struct dirent* ent;
	DIR* dir = opendir(BACKUP_PATH);

	if (dir)
	{
		int count = 0;

		while ( (ent = readdir(dir)) && done == false)
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
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
						char* path = (char*)malloc(strlen(BACKUP_PATH) + strlen(ent->d_name) + 8);
						sprintf(path, "%s/%s", BACKUP_PATH, ent->d_name);

						addMenuItem(m, ent->d_name, path);

						free(path);
					}					
				}
			}
		}
	}

	closedir(dir);

	m->nextPage = done;

	if (m->cursor >= m->itemCount)
		m->cursor = m->itemCount - 1;

	printMenu(m);
}

static void restore(Menu* m)
{
	char* fpath = m->items[m->cursor];

	bool choice = NO;
	{
		char str[] = "Are you sure you want to restore\n";
		char* msg = (char*)malloc(strlen(str) + strlen(fpath) + 1);
		sprintf(msg, "%s%s", str, fpath);

		choice = choiceBox(msg);

		free(msg);
	}

	if (choice == YES)
	{
		if (!fpath)
		{
			messageBox("Failed to restore backup.\n");
		}
		else
		{
			clearScreen(&bottomScreen);

			if (!copyDir(fpath, "/title"))
			{
				messagePrint("\nFailed to restore backup.\n");
			}
			else
			{
				messagePrint("\nBackup restored.\n");
			}
		}
	}
}

static bool delete(Menu* m)
{
	if (!m) return false;

	char* fpath = m->items[m->cursor];

	bool result = false;
	bool choice = NO;
	{
		char str[] = "Are you sure you want to delete\n";
		char* msg = (char*)malloc(strlen(str) + strlen(fpath) + 1);
		sprintf(msg, "%s\n%s", str, fpath);

		choice = choiceBox(msg);

		free(msg);
	}

	if (choice == YES)
	{
		if (!fpath)
		{
			messageBox("Failed to delete backup.\n");
		}
		else
		{
			if (!dirExists(fpath))
			{
				messageBox("Failed to delete backup.\n");
			}
			else
			{
				clearScreen(&bottomScreen);

				if (deleteDir(fpath))
				{
					result = true;
					messagePrint("\nBackup deleted.\n");
				}
				else
				{
					messagePrint("\nError deleting backup.\n");
				}
			}
		}
	}

	return result;
}