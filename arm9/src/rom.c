#include "rom.h"
#include "main.h"
#include "storage.h"
#include <nds.h>
#include <malloc.h>
#include <stdio.h>

tDSiHeader* getRomHeader(char const* fpath)
{
	if (!fpath) return NULL;

	tDSiHeader* h = NULL;
	FILE* f = fopen(fpath, "rb");

	if (f)
	{
		h = (tDSiHeader*)malloc(sizeof(tDSiHeader));

		if (h)
		{
			if (romIsCia(fpath))
				fseek(f, 0x3900, SEEK_SET);
			else
				fseek(f, 0, SEEK_SET);

			fread(h, sizeof(tDSiHeader), 1, f);
		}

		fclose(f);
	}

	return h;
}

tNDSBanner* getRomBanner(char const* fpath)
{
	if (!fpath) return NULL;

	tDSiHeader* h = getRomHeader(fpath);
	tNDSBanner* b = NULL;

	if (h)
	{
		FILE* f = fopen(fpath, "rb");

		if (f)
		{
			b = (tNDSBanner*)malloc(sizeof(tNDSBanner));

			if (b)
			{
				if (romIsCia(fpath))
					fseek(f, 0x3900, SEEK_SET);
				else
					fseek(f, 0, SEEK_SET);

				fseek(f, h->ndshdr.bannerOffset, SEEK_CUR);
				fread(b, sizeof(tNDSBanner), 1, f);
			}
		}

		free(h);
		fclose(f);		
	}

	return b;
}

bool getGameTitle(tNDSBanner* b, char* out, bool full)
{
	if (!b) return false;
	if (!out) return false;

	//get system language
	int lang = PersonalData->language;

	//not japanese or chinese
	if (lang == 0 || lang == 6)
		lang = 1;

	//read title
	u16 c;
	for (int i = 0; i < 128; i++)
	{
		c = b->titles[lang][i];

		//remove accents
		if (c == 0x00F3)
			c = 'o';

		if (c == 0x00E1)
			c = 'a';

		out[i] = (char)c;

		if (!full && out[i] == '\n')
		{
			out[i] = '\0';
			break;
		}
	}
	out[128] = '\0';

	return true;
}

bool getGameTitlePath(char const* fpath, char* out, bool full)
{
	if (!fpath) return false;
	if (!out) return false;

	tNDSBanner* b = getRomBanner(fpath);
	bool result = getGameTitle(b, out, full);
	
	free(b);
	return result;
}

bool getRomLabel(tDSiHeader* h, char* out)
{
	if (!h) return false;
	if (!out) return false;

	sprintf(out, "%.12s", h->ndshdr.gameTitle);

	return true;
}

bool getRomCode(tDSiHeader* h, char* out)
{
	if (!h) return false;
	if (!out) return false;

	sprintf(out, "%.4s", h->ndshdr.gameCode);

	return true;	
}

bool getTitleId(tDSiHeader* h, u32* low, u32* high)
{
	if (!h) return false;

	if (low != NULL)
		*low = h->tid_low;

	if (high != NULL)
		*high = h->tid_high;

	return true;
}

void printRomInfo(char const* fpath)
{
	clearScreen(&topScreen);

	if (!fpath) return;

	tDSiHeader* h = getRomHeader(fpath);
	tNDSBanner* b = getRomBanner(fpath);

	if (!isDsiHeader(h))
	{
		iprintf("Could not read dsi header.\n");
	}
	else
	{
		if (!b)
		{
			iprintf("Could not read banner.\n");
		}
		else
		{
			//proper title
			{
				char gameTitle[128+1];
				getGameTitle(b, gameTitle, true);

				iprintf("%s\n\n", gameTitle);
			}

			//file size
			{
				iprintf("Size: ");
				printBytes(getRomSize(fpath));
				iprintf("\n");
			}

			iprintf("Label: %.12s\n", h->ndshdr.gameTitle);
			iprintf("Game Code: %.4s\n", h->ndshdr.gameCode);

			//system type
			{
				iprintf("Unit Code: ");

				switch (h->ndshdr.unitCode)
				{
					case 0:  iprintf("NDS"); 	 break;
					case 2:  iprintf("NDS+DSi"); break;
					case 3:  iprintf("DSi"); 	 break;
					default: iprintf("unknown");
				}

				iprintf("\n");
			}

			//application type
			{
				iprintf("Program Type: ");

				switch (h->ndshdr.reserved1[7])
				{
					case 0x3: iprintf("Normal"); 	break;
					case 0xB: iprintf("Sys"); 		break;
					case 0xF: iprintf("Debug/Sys"); break;
					default:  iprintf("unknown");
				}

				iprintf("\n");
			}

			//DSi title ids
			{
				if (h->tid_high == 0x00030004 ||
					h->tid_high == 0x00030005 ||
					h->tid_high == 0x00030015 ||
					h->tid_high == 0x00030017 ||
					h->tid_high == 0x00030000)
				{
					iprintf("Title ID: %08x %08x\n", (unsigned int)h->tid_high, (unsigned int)h->tid_low);			
				}
			}

			//print full file path
			iprintf("\n%s\n", fpath);
		}
	}

	free(b);
	free(h);
}

unsigned long long getRomSize(char const* fpath)
{
	if (!fpath) return 0;

	unsigned long long size = 0;
	FILE* f = fopen(fpath, "rb");

	if (f)
	{
		//cia
		if (romIsCia(fpath))
		{
			unsigned char bytes[4] = { 0 };
			fseek(f, 0x38D0, SEEK_SET);
			fread(bytes, 4, 1, f);
			size = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
		}
		else
		{
			fseek(f, 0, SEEK_END);
			size = ftell(f);
		}
	}

	fclose(f);
	return size;
}

bool romIsCia(char const* fpath)
{
	if (!fpath) return false;
	return (strstr(fpath, ".cia") != NULL || strstr(fpath, ".CIA") != NULL);
}

bool isDsiHeader(tDSiHeader* h)
{
	if (!h) return false;

	return ( h->tid_low == (unsigned int)((h->ndshdr.gameCode[0] << 24) | (h->ndshdr.gameCode[1] << 16) | (h->ndshdr.gameCode[2] << 8) | h->ndshdr.gameCode[3]) );
}