#include "menus.h"
#include "storage.h"
#include <dirent.h>

#define NUM_OF_DIRECTORIES 3
static const char* directories[] = {
	"00030004",
	"00030005",
	"00030015"
};

static int cursor = 0;
static int scrolly = 0;
static int numberOfTitles = 0;

static void moveCursor(int dir);

static void printList();
static void printTitleInfo(int num);

static int getNumberOfTitles();
static int getTitle(int num, char* title, char* path);

static void subMenu();

void titleMenu()
{
	cursor = 0;
	scrolly = 0;
	numberOfTitles = getNumberOfTitles();

	consoleSelect(&topScreen);
	consoleClear();

	consoleSelect(&bottomScreen);
	consoleClear();

	//No titles error
	if (numberOfTitles <= 0)
	{
		iprintf("No titles found.\n");
		iprintf("Back - B\n");

		keyWait(KEY_B | KEY_A | KEY_START);
		return;
	}

	//Print data
	consoleSelect(&topScreen);
	printTitleInfo(cursor);

	consoleSelect(&bottomScreen);
	printList();

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		int thisScrolly = scrolly;
		int thisCursor = cursor;

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

		//Re-print list
		if (thisCursor != cursor)
		{
			consoleSelect(&topScreen);
			printTitleInfo(cursor);
		}

		if (thisScrolly != scrolly)
		{
			consoleSelect(&bottomScreen);
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
			printTitleInfo(cursor);

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
		char title[256];
		if (getTitle(i, title, NULL) == 1)
			iprintf(" %.30s\n", title);
	}

	//Scroll arrows
	if (scrolly > 0)
		iprintf("\x1b[0;31H^");

	if (scrolly < numberOfTitles - 23)
		iprintf("\x1b[22;31Hv");
}

void printTitleInfo(int num)
{
	consoleClear();

	char path[256];
	if (getTitle(num, NULL, path) == 1)
		printFileInfo(path);
}

int getNumberOfTitles()
{
	int count = 0;

	//Scan choice title directories
	for (int i = 0; i < NUM_OF_DIRECTORIES; i++)
	{
		DIR* dir;
		struct dirent* ent;

		char dirPath[256];
		sprintf(dirPath, "/title/%s", directories[i]);

		dir = opendir(dirPath);

		if (dir)
		{
			while ( (ent = readdir(dir)) != NULL )
			{
				if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
					continue;

				if (ent->d_type == DT_DIR)
				{
					//Search for an .app file
					char contentPath[384];
					sprintf(contentPath, "%s/%s/content", dirPath, ent->d_name);

					DIR* subdir;
					struct dirent* subent;

					subdir = opendir(contentPath);

					if (subdir)
					{
						while ( (subent = readdir(subdir)) != NULL )
						{
							if (strcmp(".", subent->d_name) == 0 || strcmp("..", subent->d_name) == 0)
								continue;

							//Found a title
							if (strstr(subent->d_name, ".app") != NULL)
								count++;
						}
					}

					closedir(subdir);
				}
			}
		}

		closedir(dir);
	}

	return count;
}

int getTitle(int num, char* title, char* path)
{
	int result = 0;
	int count = 0;

	//Scan choice title directories
	for (int i = 0; i < NUM_OF_DIRECTORIES && result == 0; i++)
	{
		DIR* dir;
		struct dirent* ent;

		char dirPath[256];
		sprintf(dirPath, "/title/%s", directories[i]);

		dir = opendir(dirPath);

		if (dir)
		{
			while ( (ent = readdir(dir)) != NULL && result == 0)
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
						while ( (subent = readdir(subdir)) != NULL && result == 0)
						{
							if (strcmp(".", subent->d_name) == 0 || strcmp("..", subent->d_name) == 0)
								continue;

							if (subent->d_type != DT_DIR)
							{								
								if (strstr(subent->d_name, ".app") != NULL)
								{
									if (count < num)
										count++;

									else
									{
										//Found requested title
										char filepath[384];
										sprintf(filepath, "%s/%s", contentPath, subent->d_name);

										//Output title
										if (title != NULL)
										{
											FILE* f = fopen(filepath, "rb");

											if (!f)
											{
												sprintf(title, " ");
											}
											else
											{
												tNDSHeader header;
												tNDSBanner banner;

												fread(&header, sizeof(tNDSHeader), 1, f);
												fseek(f, header.bannerOffset, SEEK_SET);
												fread(&banner, sizeof(tNDSBanner), 1, f);

												char tstr[128+1];
												tstr[128] = '\0';

												for (int i = 0; i < 128; i++)
												{
													char c = banner.titles[1][i];
													
													if (c == '\n')
														c = ' ';

													tstr[i] = c;
												}

												sprintf(title, "%s", tstr);
											}

											fclose(f);
										}

										//Output path
										if (path != NULL)
										{
											sprintf(path, "%s", filepath);
										}

										//Exit this mess
										result = 1;
										break;
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

//
static int subCursor = 0;

enum {
//	TITLE_MENU_BACKUP,
	TITLE_MENU_DUMP,
	TITLE_MENU_DELETE,
	TITLE_MENU_BACK
};

void subMenu()
{
	subCursor = 0;
	
	consoleSelect(&bottomScreen);
	consoleClear();

//	iprintf("\tBackup\n");
	iprintf("\tDump\n");
	iprintf("\tDelete\n");
	iprintf("\tBack\n");

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		//Clear cursor
		iprintf("\x1b[%d;0H ", subCursor);

		//Move cursor
		if (keysDown() & KEY_DOWN)
		{
			if (subCursor < TITLE_MENU_BACK)
				subCursor += 1;
		}

		if (keysDown() & KEY_UP)
		{
			if (subCursor > 0)
				subCursor -= 1;
		}

		//Print cursor
		iprintf("\x1b[%d;0H>", subCursor);

		if (keysDown() & KEY_A)
		{
			char title[256];
			char path[256];
			getTitle(cursor, title, path);

			//Only get first line of title
			for (int i = 0; i < 256; i++)
			{
				if (title[i] == '\n')
				{
					title[i] = '\0';
					break;
				}				
			}

/*			//
			if (subCursor == TITLE_MENU_BACKUP)
			{
				char dir[256];
				sprintf(dir, "%.24s", path);

				char msg[512];
				sprintf(msg, "Are you sure you want to backup\n%s", dir);

				if (choiceBox(msg) == YES)
				{
					if (getDirSize(dir) > getSDCardFree())
						messageBox("Error, not enough space on SD card.\nTitle backup failed.");

					else
					{
						if (copyDir(dir, BACKUP_PATH) == 1)
							messageBox("Title was backed up.");
						else
							messageBox("Title backup failed.");
					}
				}

				break;
			}
*/
			if (subCursor == TITLE_MENU_DUMP)
			{
				char fpath[256];
				if (getTitle(cursor, NULL, fpath) == 1)
				{
					FILE* f = fopen(fpath, "rb");

					if (!f)
						messageBox("Can not dump title.\n");

					else
					{
						int fsize = getFileSize(f);

						if (fsize > getSDCardFree())
							messageBox("Not enough free space on SD card.\n");

						else
						{
							tNDSHeader header;
							tNDSBanner banner;

							fread(&header, sizeof(tNDSHeader), 1, f);
							fseek(f, header.bannerOffset, SEEK_SET);
							fread(&banner, sizeof(tNDSBanner), 1, f);
							fclose(f);

							char outpath[256];
							sprintf(outpath, "%s%.12s - %.4s.nds", ROM_PATH, header.gameTitle, header.gameCode);

							char msg[512];
							sprintf(msg, "Dump title to\n%s\n", outpath);

							if (choiceBox(msg) == YES)
							{
								if (copyFile(fpath, outpath) == 1)
									messageBox("Title saved.\n");
								else
									messageBox("Title dump failed.\n");
							}
						}
					}

					fclose(f);
				}

				break;
			}

			else if (subCursor == TITLE_MENU_DELETE)
			{
				char msg[512];
				sprintf(msg, "Are you sure you want to delete\n%s", title);

				if (choiceBox(msg) == YES)
				{
					char dirPath[256];
					sprintf(dirPath, "%.25s", path);

					if (deleteDir(dirPath) == 1)
						messageBox("Title deleted.\n");
					else
						messageBox("Title could not be deleted.\n");

					//Reset main menu
					cursor = 0;
					scrolly = 0;
					numberOfTitles = getNumberOfTitles();
				}
				
				break;
			}

			else if (subCursor == TITLE_MENU_BACK)
			{
				break;
			}
		}
		else if (keysDown() & KEY_B)
			break;
	}
}