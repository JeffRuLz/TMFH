#include "message.h"
#include "main.h"

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

bool choiceBox(char* message)
{	
	const int choiceRow = 10;
	int cursor = 0;

	clearScreen(&bottomScreen);

	iprintf("\x1B[33m");	//yellow
	iprintf("%s\n", message);
	iprintf("\x1B[47m");	//white
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

bool choicePrint(char* message)
{
	bool choice = NO;

	iprintf("\x1B[33m");	//yellow
	iprintf("\n%s\n", message);
	iprintf("\x1B[47m");	//white
	iprintf("Yes - [A]\nNo  - [B]\n");

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (keysDown() & KEY_A)
		{
			choice = YES;
			break;
		}

		else if (keysDown() & KEY_B)
		{
			choice = NO;
			break;
		}
	}

	scanKeys();
	return choice;
}

void messageBox(char* message)
{
	clearScreen(&bottomScreen);
	messagePrint(message);
}

void messagePrint(char* message)
{
	iprintf("%s\n", message);
	iprintf("\nOkay - [A]\n");

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (keysDown() & (KEY_A | KEY_B | KEY_START))
			break;
	}

	scanKeys();
}