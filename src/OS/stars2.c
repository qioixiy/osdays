#include "api.h"
int rand(void);

void HariMain(void)
{
  char *buf;
  int win;
  int i, x, y;
  
  api_initmalloc();
  buf = api_malloc(150*100);
  win = api_openwin(buf, 150, 100, -1, "stars2");
  api_boxfilwin(win, 6, 26, 143, 93, 0/*ºÚÉ«*/);

  for(i = 0; i < 50; i++) {
    x = (rand() % 137) + 6;
    y = (rand() % 67) + 26;
    api_point(win+1, x, y, 3/*»ÆÉ«*/);
  }
  api_refreshwin(win, 6, 26, 144, 94);
  api_end();
}
