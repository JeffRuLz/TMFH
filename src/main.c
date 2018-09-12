#include "main.h"
#include "menus.h"

#define VERSION "0.4"

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
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen,    3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	if (!fatInitDefault())
	{
		consoleSelect(&bottomScreen);
		consoleClear();

		iprintf("fatInitDefault...Failed\n");
		iprintf("\nPress B to exit.\n");

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

static int cursor = 0;

int mainMenu()
{
	consoleSelect(&topScreen);
	consoleClear();

	iprintf("\tTitle Manager for HiyaCFW\n");
	iprintf("\nversion %s\n", VERSION);
	iprintf("\x1b[23;0HJeff - 2018");

	consoleSelect(&bottomScreen);
	consoleClear();

	iprintf("\tInstall\n");
	iprintf("\tTitles\n");
//	iprintf("\tRestore\n");
	iprintf("\tTest\n");
	iprintf("\tExit");
	
	while (1)
	{
		swiWaitForVBlank();
		scanKeys();
		
		//Clear cursor
		iprintf("\x1b[%d;0H ", cursor);
		
		if (keysDown() & KEY_DOWN)
		{
			if ( (cursor += 1) > MAIN_MENU_EXIT )
				cursor = 0;
		}
		
		if (keysDown() & KEY_RIGHT)
		{
			repeat (10)
			{
				if ( (cursor += 1) > MAIN_MENU_EXIT )
					cursor = 0;
			}
		}
		
		if (keysDown() & KEY_UP)
		{
			if ( (cursor -= 1) < 0 )
				cursor = MAIN_MENU_EXIT;
		}
		
		if (keysDown() & KEY_LEFT)
		{
			repeat (10)
			{
				if ( (cursor -= 1) < 0 )
					cursor = MAIN_MENU_EXIT;
			}
		}
		
		//print cursor
		iprintf("\x1b[%d;0H>", cursor);
		
		if (keysDown() & KEY_A)
			break;
	}
	
	return cursor;
}