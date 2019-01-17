#include "main.h"
#include "menu.h"
#include "storage.h"

void testMenu()
{
	clearScreen(&topScreen);

	iprintf("Storage Check Test\n\n");

	clearScreen(&bottomScreen);

	unsigned int free = 0;
	unsigned int size = 0;

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

		unsigned long long sdfree = getSDCardFree();
		printBytes(sdfree);
		iprintf(" / "); swiWaitForVBlank();

		unsigned long long sdsize = getSDCardSize();
		printBytes(sdsize);	
		iprintf("\n"); swiWaitForVBlank();

		printf("\t%d / %d blocks\n", (unsigned int)(sdfree / BYTES_PER_BLOCK), (unsigned int)(sdsize / BYTES_PER_BLOCK));
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