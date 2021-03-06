#include "api.h"

void HariMain(void)
{
  char *buf;
  int win, i;
  int x, y;

  api_initmalloc();
  buf = api_malloc(160*100);
  win = api_openwin(buf, 160, 100, -1, "walk");
  api_boxfilwin(win, 4, 24, 155, 95, 0);//黑色
  x = 76;
  y = 56;

  api_putstrwin(win, x, y, 3/*黄色*/, 1, "*");
  
  for (;;) {
    i = api_getkey(1);
    api_putstrwin(win, x, y, 0/*黑色*/, 1, "*");//用黑色的擦除
    if (i == '4' && x > 4) 
      x -= 8;
    if (i == '6' && x < 148)
      x += 8;
    if (i == '8' && y > 24)
      y -= 8;
    if (i == '2' && y < 80)
      y += 8;
    if (i == 0x0a) 
      break;
    api_putstrwin(win, x, y, 3/*yellow*/, 1, "*");
  }

  //api_closewin(win);now, the app can close window auto.
  api_end();
}
