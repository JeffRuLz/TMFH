#include "titles.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>

bool titleIsUsed(u32 tidlow, u32 tidhigh)
{
	bool result = false;

	char path[256];
	sprintf(path, "/title/%08x/%08x/", (unsigned int)tidhigh, (unsigned int)tidlow);

	DIR* dir = opendir(path);
	struct dirent* ent;

	if (dir)
	{
		while ( (ent = readdir(dir)) != NULL )
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type != DT_DIR)
			{
				if (strstr(ent->d_name, ".app") != NULL)
				{
					result = true;
					break;
				}
			}
		}		
	}

	closedir(dir);

	return result;
}