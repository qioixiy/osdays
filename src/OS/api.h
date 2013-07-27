#ifndef _API_H
#define _API_H

void api_putchar(int c);
void api_end(void);
void api_putstr0(char *s);
int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
int api_putstrwin(int win, int x, int y, int col, int len, char *str);
int api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);

void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);

void api_point(int win, int x, int y, int col);//col:É«ºÅ

#endif
