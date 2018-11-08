#include "main.h"
#include "menu.h"
#include <time.h>

#define VERSION "0.5.2"

enum {
	MAIN_MENU_INSTALL,
	MAIN_MENU_TITLES,
//	MAIN_MENU_RESTORE,
	MAIN_MENU_TEST,
	MAIN_MENU_EXIT
};

static int mainMenu();

int main(int argc, char **argv)
{	
	srand(time(0)); 

	//Setup top screen
	REG_DISPCNT = MODE_FB0;
	VRAM_A_CR = VRAM_ENABLE;

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen,    3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	consoleSelect(&bottomScreen);
	consoleClear();

	VRAM_A[100] = 0xFFFF;

	if (!fatInitDefault())
	{
		consoleSelect(&bottomScreen);
		consoleClear();

		//iprintf("fatInitDefault...Failed\n");
		//iprintf("\nPress B to exit.\n");

		for (int i = 0; i < 32*24; i++)
			iprintf("%c", i);

		keyWait(KEY_B | KEY_A | KEY_START);
	}
	else
	{
		bool programEnd = false;

		while (!programEnd)
		{
			switch (mainMenu())
			{
				case MAIN_MENU_INSTALL:
					installMenu();
					break;

				case MAIN_MENU_TITLES:
					titleMenu();
					break;

				case MAIN_MENU_TEST:
					testMenu();
					break;

				case MAIN_MENU_EXIT:
					programEnd = true;
					break;
			}
		}
	}

	return 0;
}

static int mainMenu()
{
	consoleSelect(&topScreen);
	consoleClear();

	iprintf("\tTitle Manager for HiyaCFW\n");
	iprintf("\nversion %s\n", VERSION);
	iprintf("\x1b[23;0HJeff - 2018");

	Menu* m = (Menu*)malloc(sizeof(Menu));
	clearMenu(m);

	addMenuItem(m, "Install");
	addMenuItem(m, "Titles");
	addMenuItem(m, "Test");
	addMenuItem(m, "Exit");

	printMenu(m);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m) == 1)
			printMenu(m);

		if (keysDown() & KEY_A)
			break;
	}

	int cursor = m->cursor;
	free(m);

	return cursor;
}