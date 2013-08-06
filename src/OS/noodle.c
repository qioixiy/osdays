#include <stdio.h>
#include "api.h"

void HariMain(void)
{
  char *buf, s[12];
  int win, timer, sec = 0, min = 0, hour = 0;
  
  api_initmalloc();
  buf = api_malloc(150*50);
  win = api_openwin(buf, 150, 50, -1, "noodle.");
  
  timer = api_alloctimer();
  api_inittimer(timer, 128);
  
  for(;;) {
    sprintf(s, "%5d:%02d:%02d", hour, min, sec);
    
    api_boxfilwin(win, 28, 27, 115, 41, 7/*°×É«*/);
    api_putstrwin(win, 28, 27, 0/*ºÚÉ«*/, 11, s);
    
    api_settimer(timer, 100);//100ms * 10
    
    if (api_getkey(1) != 128 ) {
      break;
    }
    
    sec++;
    
    if (sec == 60) {
      min++;
      sec = 0;
    }
    
    if (min == 60) {
      hour++;
      min = 0;
    }

    if (hour == 24){
      hour = 0;
    }
  }

  api_end();
}
