#include "menus.h"

void keyWait(u32 key)
{
	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (keysDown() & key)
			break;
	}
}

int choiceBox(char* message)
{	
	const int choiceRow = 10;
	int cursor = 0;

	consoleSelect(&bottomScreen);
	consoleClear();

	iprintf("%s\n", message);
	iprintf("\x1b[%d;0H\tYes\n\tNo\n", choiceRow);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		//Clear cursor
		iprintf("\x1b[%d;0H ", choiceRow + cursor);

		if (keysDown() & (KEY_UP | KEY_DOWN))
			cursor = !cursor;

		//Print cursor
		iprintf("\x1b[%d;0H>", choiceRow + cursor);

		if (keysDown() & (KEY_A | KEY_START))
			break;

		if (keysDown() & KEY_B)
		{
			cursor = 1;
			break;
		}
	}

	scanKeys();
	return (cursor == 0)? YES: NO;
}

void messageBox(char* message)
{
	consoleSelect(&bottomScreen);
	consoleClear();

	iprintf("%s\n", message);
	iprintf("\nOkay - A\n");

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (keysDown() & (KEY_A | KEY_START))
			break;
	}

	scanKeys();
}