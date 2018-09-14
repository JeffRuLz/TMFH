#include "menus.h"
#include "storage.h"

void testMenu()
{
	consoleSelect(&topScreen);
	consoleClear();

	iprintf("Storage Check Test\n\n");

	consoleSelect(&bottomScreen);
	consoleClear();

	int free = -1;
	int size = -1;

	//Home menu slots
	{
		iprintf("Free Home Menu Slots:\n"); swiWaitForVBlank();

		free = getMenuSlotsFree();
		iprintf("\t%d / ", free); swiWaitForVBlank();

		size = getMenuSlots();
		iprintf("%d\n", size); swiWaitForVBlank();
	}

	//SD Card
	{
		iprintf("\nFree SD Space:\n\t"); swiWaitForVBlank();

		free = getSDCardFree();
		printBytes(free);
		iprintf(" / "); swiWaitForVBlank();

		size = getSDCardSize();
		printBytes(size);	
		iprintf("\n"); swiWaitForVBlank();

		printf("\t%.0f / %.0f blocks\n", (float)free / BYTES_PER_BLOCK, (float)size / BYTES_PER_BLOCK);
	}

	//Emunand
	{
		iprintf("\nFree DSi Space:\n\t"); swiWaitForVBlank();

		free = getDsiFree();
		printBytes(free);
		iprintf(" / "); swiWaitForVBlank();

		size = getDsiSize();
		printBytes(size);
		iprintf("\n"); swiWaitForVBlank();

		printf("\t%.0f / %.0f blocks\n", (float)free / BYTES_PER_BLOCK, (float)size / BYTES_PER_BLOCK);
	}

	//
	iprintf("\nBack - B\n");

	keyWait(KEY_B);
}