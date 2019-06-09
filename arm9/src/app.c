#include "app.h"
#include "main.h"
#include "storage.h"
#include <nds.h>
#include <malloc.h>
#include <stdio.h>

static void _getTitle(tNDSBanner* b, char* out, bool full)
{
	int lang = PersonalData->language;

	//not japanese or chinese
	if (lang == 0 || lang == 6)
		lang = 1;

	for (int i = 0; i < 128; i++)
	{
		u16 c = b->titles[lang][i];

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
}

bool getAppTitle(char* fpath, char* out)
{
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		return false;
	}
	else
	{
		tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));
		tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

		fseek(f, 0, SEEK_SET);
		fread(header, sizeof(tNDSHeader), 1, f);
		fseek(f, header->bannerOffset, SEEK_SET);
		fread(banner, sizeof(tNDSBanner), 1, f);

		_getTitle(banner, out, false);

		free(banner);
		free(header);
	}

	fclose(f);
	return true;	
}

bool getAppFullTitle(char* fpath, char* out)
{
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		return false;
	}
	else
	{
		tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));
		tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

		fseek(f, 0, SEEK_SET);
		fread(header, sizeof(tNDSHeader), 1, f);
		fseek(f, header->bannerOffset, SEEK_SET);
		fread(banner, sizeof(tNDSBanner), 1, f);

		_getTitle(banner, out, true);

		free(banner);
		free(header);
	}

	fclose(f);
	return true;
}

bool getAppLabel(char* fpath, char* out)
{
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		return false;
	}
	else
	{
		tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));

		fseek(f, 0, SEEK_SET);
		fread(header, sizeof(tNDSHeader), 1, f);

		sprintf(out, "%.12s", header->gameTitle);

		free(header);
	}
	
	fclose(f);
	return true;	
}

bool getGameCode(char* fpath, char* out)
{
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		return false;
	}
	else
	{
		tNDSHeader* header = (tNDSHeader*)malloc(sizeof(tNDSHeader));

		fseek(f, 0, SEEK_SET);
		fread(header, sizeof(tNDSHeader), 1, f);

		sprintf(out, "%.4s", header->gameCode);

		free(header);
	}
	
	fclose(f);
	return true;	
}

bool getTid(char* fpath, u32* low, u32* high)
{
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		return false;
	}
	else
	{
		tDSiHeader* header = (tDSiHeader*)malloc(sizeof(tDSiHeader));

		fseek(f, 0, SEEK_SET);
		fread(header, sizeof(tDSiHeader), 1, f);

		if (low != NULL)
			*low = header->tid_low;

		if (high != NULL)
			*high = header->tid_high;

		free(header);
	}
	
	fclose(f);
	return true;	
}

void printAppInfo(char const* path)
{
	clearScreen(&topScreen);

	if (!path) return;

	tDSiHeader* header = (tDSiHeader*)malloc(sizeof(tDSiHeader));
	tNDSBanner* banner = (tNDSBanner*)malloc(sizeof(tNDSBanner));

	FILE* f = fopen(path, "rb");

	if (f)
	{
		if (fread(header, sizeof(tDSiHeader), 1, f) != 1)
		{
			iprintf("Could not read dsi header.\n");
		}
		else
		{
			fseek(f, header->ndshdr.bannerOffset, SEEK_SET);

			if (fread(banner, sizeof(tNDSBanner), 1, f) != 1)
			{
				iprintf("Could not read banner.\n");
			}
			else
			{
				//proper title
				{
					char gameTitle[128+1];
					_getTitle(banner, gameTitle, true);

					iprintf("%s\n\n", gameTitle);
				}

				//file size
				{
					iprintf("Size: ");
					printBytes(getFileSize(f));
					iprintf("\n");
				}

				iprintf("Label: %.12s\n", header->ndshdr.gameTitle);
				iprintf("Game Code: %.4s\n", header->ndshdr.gameCode);

				//system type
				{
					iprintf("Unit Code: ");

					switch (header->ndshdr.unitCode)
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

					switch (header->ndshdr.reserved1[7])
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
					if (header->tid_high == 0x00030004 ||
						header->tid_high == 0x00030005 ||
						header->tid_high == 0x00030015 ||
						header->tid_high == 0x00030017 ||
						header->tid_high == 0x00030000)
					{
						iprintf("Title ID: %08x %08x\n", (unsigned int)header->tid_high, (unsigned int)header->tid_low);			
					}
				}

				//print full file path
				iprintf("\n%s\n", path);
			}
		}
	}

	fclose(f);

	free(banner);
	free(header);
}