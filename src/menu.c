#include "menu.h"
#include "main.h"
#include <nds.h>

#define SCREEN_ROWS 23

void clearMenu(Menu* m)
{
	if (m == NULL) return;

	m->numberOfItems = 0;
	m->cursor = 0;
	m->scrolly = 0;
}

static void _printMenuItem(Menu* m, int item)
{
	if (item < 0 || item > m->numberOfItems) return;

	iprintf(" %.30s", m->items[item].label);
}

void printMenu(Menu* m)
{
	if (m == NULL) return;

	swiWaitForVBlank();
	clearScreen(&bottomScreen);

	int i = m->scrolly;
	while (i < m->scrolly + SCREEN_ROWS && i < m->numberOfItems)
	{
		_printMenuItem(m, i);
		iprintf("\n");

		i++;
	}

	//Cursor
	iprintf("\x1b[%d;0H>", m->cursor - m->scrolly);

	//Scroll arrows
	if (m->scrolly > 0)
		iprintf("\x1b[0;31H^");

	if (m->scrolly < m->numberOfItems - SCREEN_ROWS)
		iprintf("\x1b[22;31Hv");
}

int getMenuCursor(Menu* m)
{
	if (m == NULL) return -1;
	return m->cursor;
}

int getNumberOfMenuItems(Menu* m)
{
	if (m == NULL) return -1;
	return m->numberOfItems;
}

static void _moveCursor(Menu* m, int dir)
{
	m->cursor += sign(dir);

	if (m->cursor < 0)
		m->cursor = 0;

	if (m->cursor >= m->numberOfItems - 1)
		m->cursor = m->numberOfItems - 1;

	if (m->cursor - m->scrolly >= SCREEN_ROWS)
		m->scrolly += 1;

	if (m->cursor - m->scrolly < 0)
		m->scrolly -= 1;
}

bool moveCursor(Menu* m)
{
	if (m == NULL)
		return false;

	int lastCursor = m->cursor;

	if (keysDown() & KEY_DOWN)
		_moveCursor(m, 1);

	if (keysDown() & KEY_UP)
		_moveCursor(m, -1);

	if (keysDown() & KEY_RIGHT)
	{
		repeat (10)
			_moveCursor(m, 1);
	}		

	if (keysDown() & KEY_LEFT)
	{
		repeat (10)
			_moveCursor(m, -1);
	}

	return !(lastCursor == m->cursor);
}

void addMenuItem(Menu* m, char* label)
{
	if (m == NULL) return;

	if (label != NULL)
		sprintf(m->items[m->numberOfItems].label, "%.32s", label);

	m->numberOfItems += 1;

	m->cursor = 0;
	m->scrolly = 0;
}

//
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

bool choicePrint(char* message)
{
	bool choice = NO;

	iprintf("\n%s\n", message);
	iprintf("Yes - A\nNo  - B\n");

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