#ifndef APP_H
#define APP_H

#include <nds/ndstypes.h>

bool getAppTitle(char* fpath, char* out);
bool getAppFullTitle(char* fpath, char* out);

bool getAppLabel(char* fpath, char* out);

bool getGameCode(char* fpath, char* out);

bool getTid(char* fpath, u32* low, u32* high);

void printAppInfo(char const* path);

#endif