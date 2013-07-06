#ifndef _TIMER_H
#define _TIMER_H

#define MAX_TIMER 500//×î¶àtimerÊý
struct TIMER{
  unsigned int timeout, flags;
  struct FIFO8 *fifo;
  unsigned char data;
};

struct TIMERCTL{
  unsigned int count;
  struct TIMER timer[MAX_TIMER];
};


void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void timer_init(struct TIMER *timer,struct FIFO8 *fifo, unsigned char data);
extern struct TIMERCTL timerctl;

#endif
