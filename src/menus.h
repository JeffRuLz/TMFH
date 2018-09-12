#ifndef MENUS_H
#define MENUS_H

#include "main.h"

//Text box choices
#define YES 1
#define NO  0

void titleMenu();
void installMenu();
void testMenu();

void keyWait(u32 key);

int choiceBox(char* message);
void messageBox(char* message);

#endif