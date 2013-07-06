#ifndef _TIMER_H
#define _TIMER_H

struct TIMERCTL{
  unsigned int count;
};

void init_pit(void);
extern struct TIMERCTL timerctl;

#endif
