#include "menu.h"
#include "main.h"

Menu* newMenu()
{
	Menu* m = (Menu*)malloc(sizeof(Menu));
	
	m->cursor = 0;
	m->page = 0;
	m->itemCount = 0;
	m->nextPage = false;
	m->changePage = 0;

	for (int i = 0; i < ITEMS_PER_PAGE; i++)
	{
		m->labels[i] = NULL;
		m->items[i] = NULL;
	}

	return m;
}

void freeMenu(Menu* m)
{
	if (!m) return;

	clearMenu(m);
	
	free(m);
	m = NULL;
}

void addMenuItem(Menu* m, char const* label, char const* value)
{
	if (!m) return;

	int i = m->itemCount;
	if (i >= ITEMS_PER_PAGE) return;

	if (label)
	{
		m->labels[i] = (char*)malloc(32);
		sprintf(m->labels[i], "%.31s", label);
	}

	if (value)
	{
		m->items[i] = (char*)malloc(strlen(value)+1);
		sprintf(m->items[i], "%s", value);
	}
	
	m->itemCount += 1;
}

void resetMenu(Menu* m)
{
	m->cursor = 0;
	m->page = 0;
	m->changePage = 0;
	m->nextPage = 0;
}

void clearMenu(Menu* m)
{
	if (!m) return;

	for (int i = 0; i < ITEMS_PER_PAGE; i++)
	{
		if (m->labels[i])
		{
			free(m->labels[i]);
			m->labels[i] = NULL;
		}

		if (m->items[i])
		{
			free(m->items[i]);
			m->items[i] = NULL;
		}
	}

	m->itemCount = 0;
}

void printMenu(Menu* m)
{
	clearScreen(&bottomScreen);

	if (!m) return;

	for (int i = 0; i < m->itemCount; i++)
	{
		if (m->labels[i])
			iprintf(" %.30s\n", m->labels[i]);
		else
			iprintf(" \n");
	}

	//cursor
	iprintf("\x1b[%d;0H>", m->cursor);

	//scroll arrows
	if (m->page > 0)
		iprintf("\x1b[0;31H^");

	if (m->nextPage)
		iprintf("\x1b[22;31Hv");
}

static void _moveCursor(Menu* m, int dir)
{
	if (m->changePage != 0)
		return;

	m->cursor += sign(dir);

	if (m->cursor < 0)
	{
		if (m->page <= 0)
			m->cursor = 0;
		else
		{
			m->changePage = -1;
			m->cursor = ITEMS_PER_PAGE - 1;
		}
	}

	else if (m->cursor > m->itemCount-1)
	{
		if (m->cursor >= ITEMS_PER_PAGE)
		{
			m->changePage = 1;
			m->cursor = 0;
		}
		else
		{
			m->cursor = m->itemCount-1;
		}
	}		
}

bool moveCursor(Menu* m)
{
	if (!m) return false;

	m->changePage = 0;
	int lastCursor = m->cursor;

	if (keysDown() & KEY_DOWN)
		_moveCursor(m, 1);

	else if (keysDown() & KEY_UP)
		_moveCursor(m, -1);

	if (keysDown() & KEY_RIGHT)
	{
		repeat(10)
			_moveCursor(m, 1);
	}

	else if (keysDown() & KEY_LEFT)
	{
		repeat(10)
			_moveCursor(m, -1);
	}

	return !(lastCursor == m->cursor);
}