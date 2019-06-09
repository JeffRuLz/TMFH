#ifndef MENU_H
#define MENU_H

#include <nds/ndstypes.h>

#define ITEMS_PER_PAGE 23

typedef struct {
	int cursor;
	int page;
	int itemCount;
	bool nextPage;
	int changePage;
	char* labels[ITEMS_PER_PAGE];
	char* items[ITEMS_PER_PAGE];
} Menu;

Menu* newMenu();
void freeMenu(Menu* m);

void addMenuItem(Menu* m, char const* label, char const* value);

void resetMenu(Menu* m);
void clearMenu(Menu* m);
void printMenu(Menu* m);

bool moveCursor(Menu* m);

#endif