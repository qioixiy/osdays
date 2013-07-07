#include "timer.h"
#include "int.h"
#include "naskfunc.h"
#include "fifo.h"
#include "mtask.h"

#define PIT_CTRL 0X0043
#define PIT_CNT0 0X0040

#define TIMER_FLAGS_NOUSE 0//δʹ��
#define TIMER_FLAGS_ALLOC 1//�ѷ���״̬
#define TIMER_FLAGS_USING 2//��ʱ��������

struct TIMERCTL timerctl;

void init_pit(void)
{
  int i = 0;

  io_out8(PIT_CTRL, 0x34);  
  //����Ϊ0x2e9c��ʱ������Ƶ�ʴ�ԼΪ100HZ
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);

  timerctl.count = 0;
  timerctl.next = 0xffffffff;
  for(i = 0; i<MAX_TIMER; i++) {
    timerctl.timers0[i].flags = TIMER_FLAGS_NOUSE;//δʹ��
  }
  
  struct TIMER *t;
  t = timer_alloc();
  t->timeout = 0xffffffff;
  t->flags = TIMER_FLAGS_USING;
  t->next = 0;
  timerctl.t0 = t;//��Ϊ����ֻ�д��ڱ�timer
  timerctl.next = 0xffffffff;//��һ����ʱʱ�̾���0xffffffff
  
  return;
}

struct TIMER *timer_alloc(void)
{
  int i;
  for(i = 0; i < MAX_TIMER; i++){
    if (timerctl.timers0[i].flags == TIMER_FLAGS_NOUSE) {
      timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
      return &timerctl.timers0[i];
    }
  }
  return 0;
}

void timer_free(struct TIMER *timer)
{
  timer->flags = TIMER_FLAGS_NOUSE;
}

void timer_init(struct TIMER *timer,struct FIFO32 *fifo, unsigned int data)
{
  timer->fifo = fifo;
  timer->data = data;
  return ;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
  //���ڼ����ڱ�timer������timerֻ������ǰ������м�
  int e;
  struct TIMER *s, *t;
  
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  e = io_load_eflags();
  io_cli();

  t = timerctl.t0;
  if (timer->timeout <= t->timeout) {//���������Ķ�ʱ����������ǰ��
    timerctl.t0 = timer;
    timer->next = t;
    timerctl.next = timer->timeout;
    io_store_eflags(e);
    return;
  }
  
  //��Ѱ�����λ��
  for(;;) {
    s = t;
    t = t->next;
    if (t == 0) {
      break;//��������
    }
    if (timer->timeout <= t->timeout) {
      //��t֮ǰ����, s,t֮��
      s->next = timer;
      timer->next = t;
      io_store_eflags(e);
      return;
    }
  }
}

void inthandler20(int *esp)
{
  char ts = 0;
  struct TIMER *timer;
  io_out8(PIC0_OCW2, 0X60);//��IRQ-00�źŽ������˵���Ϣ֪ͨ��PIC
  timerctl.count++;
  if (timerctl.count < timerctl.next) {
    return ;//��������һʱ��
  }

  timer = timerctl.t0;//���Ȱ���ǰ��ĵ�ַ��timer
  for (;;) {
    //timer����USING ״̬����Ҫȷ��
    if (timer->timeout > timerctl.count) {
      break;
    }
    //��ʱ
    timer->flags = TIMER_FLAGS_ALLOC;
    if (timer != task_timer) {
      fifo32_put(timer->fifo, timer->data);
    } else {
      ts = 1;//mt_timer��ʱ
    }
    timer = timer->next;//ָ����һ����ʱ��
  }
  
  timerctl.t0 = timer;
  //timer next �趨
  timerctl.next = timerctl.t0->timeout;

  if (ts != 0) {
    task_switch();
  }
  return;
}

//ʱ���������
void timer_adjust(void)
{
  int t0 = timerctl.count;
  int i = 0;
  io_cli();//��ֹ�ж�
  
  timerctl.count -= t0;
  for (i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timers0[i].flags == TIMER_FLAGS_USING) {
      if (timerctl.timers0[i].timeout != 0xffffffff)
      timerctl.timers0[i].timeout -= t0;
    }
  }
  io_sti();
}
