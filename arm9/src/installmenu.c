#include "main.h"
#include "app.h"
#include "install.h"
#include "menu.h"
#include "storage.h"
#include "message.h"
#include <dirent.h>

enum {
	INSTALL_MENU_INSTALL,
	INSTALL_MENU_SYSTEM_TITLE,
	INSTALL_MENU_DELETE,
	INSTALL_MENU_BACK
};

static void generateList(Menu* m);
static void printItem(Menu* m);
static int subMenu();
static bool delete(Menu* m);

void installMenu()
{
	Menu* m = newMenu();
	generateList(m);

	//no files found
	if (m->itemCount <= 0)
	{
		clearScreen(&bottomScreen);

		iprintf("No files found.\n");
		iprintf("Place .nds, .app, or .dsi files in %s\n", ROM_PATH);
		iprintf("\nBack - [B]\n");

		keyWait(KEY_B | KEY_A | KEY_START);
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

			//back
			if (keysDown() & KEY_B || m->itemCount <= 0)
			{
				break;
			}

			//selection
			else if (keysDown() & KEY_A)
			{
				switch (subMenu())
				{
					case INSTALL_MENU_INSTALL:
					{
						if (install(m->items[m->cursor], false))
						{
							resetMenu(m);
							generateList(m);
						}
					}
					break;
						
					case INSTALL_MENU_SYSTEM_TITLE:
					{
						if (install(m->items[m->cursor], true))
						{
							resetMenu(m);
							generateList(m);
						}
					}
					break;

					case INSTALL_MENU_DELETE:
					{
						if (delete(m))
						{
							resetMenu(m);
							generateList(m);
						}
					}
					break;

					case INSTALL_MENU_BACK:					
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

	//reset menu
	clearMenu(m);

	m->page += sign(m->changePage);
	m->changePage = 0;

	bool done = false;

	struct dirent* ent;
	DIR* dir = opendir(ROM_PATH);	

	if (dir)
	{
		int count = 0;

		//scan /dsi/
		while ( (ent = readdir(dir)) && !done)
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type != DT_DIR)
			{
				if (strstr(ent->d_name, ".nds") != NULL ||
					strstr(ent->d_name, ".app") != NULL ||
					strstr(ent->d_name, ".dsi") != NULL ||
					strstr(ent->d_name, ".NDS") != NULL ||
					strstr(ent->d_name, ".APP") != NULL ||
					strstr(ent->d_name, ".DSI") != NULL)
				{
					if (count < m->page * ITEMS_PER_PAGE)
						count += 1;
					
					else
					{
						if (m->itemCount >= ITEMS_PER_PAGE)
							done = true;
						
						else
						{
							char* fpath = (char*)malloc(strlen(ROM_PATH) + strlen(ent->d_name) + 8);
							sprintf(fpath, "%s/%s", ROM_PATH, ent->d_name);

							addMenuItem(m, ent->d_name, fpath);

							free(fpath);
						}
					}
				}
			}
		}
	}

	closedir(dir);

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

	addMenuItem(m, "Install", NULL);
	addMenuItem(m, "Install as System Title", NULL);
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
		{
			result = -1;
			break;
		}

		else if (keysDown() & KEY_A)
		{
			result = m->cursor;
			break;
		}
	}

	freeMenu(m);
	return result;
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
		sprintf(msg, "%s%s", str, fpath);

		choice = choiceBox(msg);

		free(msg);
	}

	if (choice == YES)
	{
		if (!fpath)
		{
			messageBox("Could not delete file.");
		}
		else
		{
			if (remove(fpath) == 0)
			{
				result = true;
				messageBox("File deleted.");
			}
			else
			{
				messageBox("Could not delete file.");
			}
		}		
	}

	return result;
}