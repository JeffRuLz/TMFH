#ifndef MAIN_H
#define MAIN_H

#include <nds.h>
#include <fat.h>
#include <stdio.h>

PrintConsole topScreen;
PrintConsole bottomScreen;

#define abs(X) ( (X) < 0 ? -(X): (X) )
#define sign(X) ( ((X) > 0) - ((X) < 0) )
#define repeat(X) for (int _I_ = 0; _I_ < (X); _I_++)

#endif