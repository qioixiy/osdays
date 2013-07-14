#ifndef _CONSOLE_H
#define _CONSOLE_H
#include "sheet.h"

struct CONSOLE{
  struct SHEET *sht;
  int cur_x, cur_y,cur_c;
};

int cons_newline(struct CONSOLE *cons);
void console_task(struct SHEET *sheet, unsigned int memtotal);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);

void cons_putstr0(struct CONSOLE *cons, char *s);
void cons_putstr1(struct CONSOLE *cons, char *s, int l);

#endif 









