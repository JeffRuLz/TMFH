#include "main.h"
#include "menu.h"
#include <time.h>

#define VERSION "0.5.5"

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

	//Setup screens
	REG_DISPCNT = MODE_FB0;
	VRAM_A_CR = VRAM_ENABLE;

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen,    3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true,  true);
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	clearScreen(&bottomScreen);

	VRAM_A[100] = 0xFFFF;

	//Cannot use SD card
	if (!fatInitDefault())
	{
		clearScreen(&bottomScreen);

		iprintf("fatInitDefault()...Failed\n");
		iprintf("\nPress B to exit.\n");

		keyWait(KEY_B | KEY_A | KEY_START);

		return 0;
	}

	//Main menu selection
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

/*			case MAIN_MENU_RESTORE:
				restoreMenu();
				break;
*/
			case MAIN_MENU_TEST:
				testMenu();
				break;

			case MAIN_MENU_EXIT:
				programEnd = true;
				break;
		}
	}

	return 0;
}

static int mainMenu()
{
	clearScreen(&topScreen);

	iprintf("\tTitle Manager for HiyaCFW\n");
	iprintf("\nversion %s\n", VERSION);
	iprintf("\x1b[23;0HJeff - 2018-2019");

	Menu* m = (Menu*)malloc(sizeof(Menu));
	clearMenu(m);

	addMenuItem(m, "Install");
	addMenuItem(m, "Titles");
//	addMenuItem(m, "Restore");
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

void clearScreen(PrintConsole* screen)
{
	consoleSelect(screen);
	consoleClear();
}