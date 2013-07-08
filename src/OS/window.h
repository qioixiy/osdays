#ifndef _WINDOW_H
#define _WINDOW_H
#include "sheet.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

#endif
