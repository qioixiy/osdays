#ifndef _TIMER_H
#define _TIMER_H

#define MAX_TIMER 500//最多timer数
struct TIMER{
  struct TIMER *next;
  unsigned int timeout, flags;
  struct FIFO32 *fifo;
  unsigned char data;
};

struct TIMERCTL{
  unsigned int count;
  unsigned int next;//timer下一个
  struct TIMER *t0;
  struct TIMER timers0[MAX_TIMER];
};


void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void timer_init(struct TIMER *timer,struct FIFO32 *fifo, unsigned int data);
extern struct TIMERCTL timerctl;

#endif
