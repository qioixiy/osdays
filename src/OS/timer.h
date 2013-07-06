#ifndef _TIMER_H
#define _TIMER_H

struct TIMERCTL{
  unsigned int count;
  unsigned int timeout;
  struct FIFO8 *fifo;
  unsigned char data;
};

void init_pit(void);
extern struct TIMERCTL timerctl;

#endif
