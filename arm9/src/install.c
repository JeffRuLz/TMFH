#include "install.h"
#include "fat12.h"
#include "main.h"
#include "message.h"
#include "maketmd.h"
#include "storage.h"
#include <sys/stat.h>

static bool _titleIsUsed(tDSiHeader* h)
{
	if (!h) return false;

	char path[64];
	sprintf(path, "/title/%08x/%08x/", (unsigned int)h->tid_high, (unsigned int)h->tid_low);

	return dirExists(path);
}

//patch homebrew roms if gameCode is #### or null
static bool _patchGameCode(tDSiHeader* h)
{
	if (!h) return false;

	if ((strcmp(h->ndshdr.gameCode, "####") == 0 && h->tid_low == 0x23232323) || (!*h->ndshdr.gameCode && h->tid_low == 0))
	{
		iprintf("Fixing Game Code...");
		swiWaitForVBlank();

		//set as standard app
		h->tid_high = 0x00030004;
		
		do {
			do {
				//generate a random game code
				for (int i = 0; i < 4; i++)
					h->ndshdr.gameCode[i] = 'A' + (rand() % 26);
			}
			while (h->ndshdr.gameCode[0] == 'A'); //first letter shouldn't be A
		
			//correct title id
			h->tid_low = ( (h->ndshdr.gameCode[0] << 24) | (h->ndshdr.gameCode[1] << 16) | (h->ndshdr.gameCode[2] << 8) | h->ndshdr.gameCode[3] );
		}
		while (_titleIsUsed(h));

		iprintf("Done\n");
		return true;
	}

	return false;
}

static bool _iqueHack(tDSiHeader* h)
{
	if (!h) return false;

	if (h->ndshdr.reserved1[8] == 0x80)
	{
		iprintf("iQue Hack...");	
		h->ndshdr.reserved1[8] = 0x00;
		iprintf("Done\n");
		return true;
	}

	return false;
}

static unsigned long long _getSize(FILE* f, tDSiHeader* h)
{
	iprintf("Install Size: ");
	swiWaitForVBlank();

	unsigned long long size = 0;

	//get app size
	if (f)
		size = getFileSize(f);

	//add save file size
	if (h)
	{
		size += h->public_sav_size;
		size += h->private_sav_size;

		//banner.sav
		if (h->appflags & 0x4)
			size += 0x4000;
	}

	printBytes(size);
	iprintf("\n");

	return size;
}

static bool _checkSdSpace(unsigned long long size)
{
	iprintf("Enough room on SD card?...");
	swiWaitForVBlank();

	if (getSDCardFree() < size)
	{
		iprintf("No\n");
		return false;
	}

	iprintf("Yes\n");
	return true;
}

static bool _checkDsiSpace(unsigned long long size)
{
	iprintf("Enough room on DSi?...");
	swiWaitForVBlank();

	if (getDsiFree() < size)
	{
		iprintf("No\n");
		return false;
	}

	iprintf("Yes\n");
	return true;
}

static bool _openMenuSlot()
{
	iprintf("Open DSi menu slot?...");
	swiWaitForVBlank();

	if (getMenuSlotsFree() <= 0)
	{
		iprintf("No\n");
		return choicePrint("Try installing anyway?");
	}

	iprintf("Yes\n");
	return true;
}

static bool _isDSiRom(tDSiHeader* h)
{
	//high title id must be one of four
	if (h->tid_high != 0x00030004 &&
		h->tid_high != 0x00030005 &&
		h->tid_high != 0x00030015 &&
		h->tid_high != 0x00030017)
	{
		iprintf("Error: This is not a DSi rom.\n");
		return false;
	}

	return true;
}

static void _createPublicSav(tDSiHeader* h, char* dataPath)
{
	if (!h) return;

	if (h->public_sav_size > 0)
	{
		iprintf("Creating public.sav...");
		swiWaitForVBlank();

		if (!dataPath)
		{
			iprintf("Failed\n");
		}
		else
		{
			char* publicPath = (char*)malloc(strlen(dataPath) + strlen("/public.sav") + 1);
			sprintf(publicPath, "%s/public.sav", dataPath);

			FILE* f = fopen(publicPath, "wb");

			if (!f)
			{
				iprintf("Failed\n");
			}
			else
			{
				fseek(f, h->public_sav_size-1, SEEK_SET);
				fputc(0, f);
				initFatHeader(f);

				iprintf("Done\n");
			}

			fclose(f);
			free(publicPath);
		}	
	}
}

static void _createPrivateSav(tDSiHeader* h, char* dataPath)
{
	if (!h) return;

	if (h->private_sav_size > 0)
	{
		iprintf("Creating private.sav...");
		swiWaitForVBlank();

		if (!dataPath)
		{
			iprintf("Failed\n");
		}
		else
		{
			char* privatePath = (char*)malloc(strlen(dataPath) + strlen("/private.sav") + 1);
			sprintf(privatePath, "%s/private.sav", dataPath);

			FILE* f = fopen(privatePath, "wb");

			if (!f)
			{
				iprintf("Failed\n");
			}
			else
			{
				fseek(f, h->private_sav_size-1, SEEK_SET);
				fputc(0, f);
				initFatHeader(f);

				iprintf("Done\n");
			}

			fclose(f);
			free(privatePath);
		}	
	}
}

static void _createBannerSav(tDSiHeader* h, char* dataPath)
{
	if (!h) return;

	if (h->appflags & 0x4)
	{
		iprintf("Creating banner.sav...");
		swiWaitForVBlank();

		if (!dataPath)
		{
			iprintf("Failed\n");
		}
		else
		{
			char* bannerPath = (char*)malloc(strlen(dataPath) + strlen("/banner.sav") + 1);
			sprintf(bannerPath, "%s/banner.sav", dataPath);

			FILE* f = fopen(bannerPath, "wb");

			if (!f)
			{
				iprintf("Failed\n");
			}
			else
			{
				fseek(f, 0x4000 - 1, SEEK_SET);
				fputc(0, f);

				iprintf("Done\n");
			}

			fclose(f);
			free(bannerPath);
		}	
	}
}

bool install(char* fpath, bool systemTitle)
{
	bool result = false;

	//confirmation message
	{
		char msg[512];
		sprintf(msg, "Are you sure you want to install\n%s\n", fpath);
		
		if (choiceBox(msg) == NO)
			return false;
	}

	//start installation
	clearScreen(&bottomScreen);
	iprintf("Installing %s\n\n", fpath); swiWaitForVBlank();

	tDSiHeader* header = (tDSiHeader*)malloc(sizeof(tDSiHeader));
	FILE* f = fopen(fpath, "rb");

	if (!f)
	{
		iprintf("Error: could not open file.\n");
		goto error;
	}
	else
	{
		bool fixHeader = false;

		//read header
		fseek(f, 0, SEEK_SET);
		fread(header, sizeof(tDSiHeader), 1, f);

		if (_patchGameCode(header))
			fixHeader = true;

		if (!_isDSiRom(header))
			goto error;

		unsigned long long fileSize = getFileSize(f);
		unsigned long long installSize = _getSize(f, header);

		//do not need file opened anymore
		fclose(f);

		if (!_checkSdSpace(installSize))
			goto error;

		if (!_openMenuSlot())
			goto error;

		//system title patch
		if (systemTitle)
		{
			iprintf("System Title Patch...");
			swiWaitForVBlank();
			header->tid_high = 0x00030015;
			iprintf("Done\n");

			fixHeader = true;
		}

		//skip nand check if system title
		if (header->tid_high != 0x00030015)
		{
			if (!_checkDsiSpace(installSize))
			{
				if (choicePrint("Install as system title?"))
				{
					header->tid_high = 0x00030015;
					fixHeader = true;
				}				
				else
				{
					if (choicePrint("Try installing anyway?") == NO)
						goto error;
				}
			}
		}
		
		if (_iqueHack(header))
			fixHeader = true;		

		//check if title is free
		if (_titleIsUsed(header))
		{
			char msg[512];
			sprintf(msg, "Title %08x is already used.\nInstall anyway?", (unsigned int)header->tid_low);

			if (choicePrint(msg) == NO)
				goto error;
		}

		//create title directory /title/XXXXXXXX/XXXXXXXX
		char dirPath[32];
		sprintf(dirPath, "/title/%08x/%08x", (unsigned int)header->tid_high, (unsigned int)header->tid_low);

		mkdir(dirPath, 0777);

		//content folder /title/XXXXXXXX/XXXXXXXXX/content
		{
			char contentPath[64];
			sprintf(contentPath, "%s/content", dirPath);

			mkdir(contentPath, 0777);

			//create 00000000.app
			{
				iprintf("Creating 00000000.app...");
				swiWaitForVBlank();

				char appPath[80];
				sprintf(appPath, "%s/00000000.app", contentPath);

				//copy nds file to app
				{
					if (copyFile(fpath, appPath) == 0)
					{
						iprintf("Failed\n");
						goto error;
					}

					iprintf("Done\n");
				}

				//pad out banner if it is the last part of the file
				{
					if (header->ndshdr.bannerOffset == fileSize - 0x1C00)
					{
						iprintf("Padding banner...");
						swiWaitForVBlank();

						if (padFile(appPath, 0x7C0) == false)
							iprintf("Failed\n");
						else
							iprintf("Done\n");
					}
				}

				//update header
				{
					if (fixHeader)
					{
						iprintf("Fixing header...");
						swiWaitForVBlank();

						//fix header checksum
						header->ndshdr.headerCRC16 = swiCRC16(0xFFFF, header, 0x15E);

						//fix RSA signature
						u8 buffer[20];
						swiSHA1Calc(&buffer, header, 0xE00);
						memcpy(&(header->rsa_signature[0x6C]), buffer, 20);

						f = fopen(appPath, "r+");

						if (!f)
						{
							iprintf("Failed\n");
						}
						else
						{
							fseek(f, 0, SEEK_SET);
							fwrite(header, sizeof(tDSiHeader), 1, f);

							iprintf("Done\n");
						}

						fclose(f);
					}
				}

				//make TMD
				{
					char tmdPath[80];
					sprintf(tmdPath, "%s/title.tmd", contentPath);

					if (maketmd(appPath, tmdPath) != 0)				
						goto error;
				}
			}
		}

		//data folder
		{
			char dataPath[64];
			sprintf(dataPath, "%s/data", dirPath);

			mkdir(dataPath, 0777);

			_createPublicSav(header, dataPath);
			_createPrivateSav(header, dataPath);		
			_createBannerSav(header, dataPath);
		}		

		//end
		result = true;
		iprintf("\nInstallation complete.\nBack - [B]\n");
		keyWait(KEY_A | KEY_B);	
	}

	goto complete;

error:
	messagePrint("\nInstallation failed.\n");

complete:
	fclose(f);
	free(header);

	return result;
}