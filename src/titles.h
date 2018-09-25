#ifndef TITLES_H
#define TITLES_H

#include <nds/ndstypes.h>

#define gameCodeToTitleID(X) ( (X[0] << 24) | (X[1] << 16) | (X[2] << 8) | X[3] )

bool titleIsUsed(u32 tidlow, u32 tidhigh);

#endif