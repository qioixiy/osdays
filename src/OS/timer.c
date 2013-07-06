#include "timer.h"
#include "int.h"
#include "naskfunc.h"
#include "fifo.h"

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
  int e;
  struct TIMER *s, *t;
  
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  e = io_load_eflags();
  io_cli();
  timerctl.using++;
  if (timerctl.using == 1) {//ֻ��һ�����еĶ�ʱ��
    timerctl.t0 = timer;
    timer->next = 0;//û����һ��
    timerctl.next = timer->timeout;
    io_store_eflags(e);
    return;
  }
  
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
  //�������
  s->next = timer;
  timer->next = 0;
  io_store_eflags(e);
  return;
}

void inthandler20(int *esp)
{
  int i, j;
  struct TIMER *timer;
  io_out8(PIC0_OCW2, 0X60);//��IRQ-00�źŽ������˵���Ϣ֪ͨ��PIC
  timerctl.count++;
  if (timerctl.count < timerctl.next) {
    return ;//��������һʱ��
  }
  timer = timerctl.t0;//���Ȱ���ǰ��ĵ�ַ��timer
  for (i = 0; i < timerctl.using; i++) {
    //timer����USING ״̬����Ҫȷ��
    if (timer->timeout > timerctl.count) {
      break;
    }
    //��ʱ
    timer->flags = TIMER_FLAGS_ALLOC;
    fifo32_put(timer->fifo, timer->data);
    timer = timer->next;//ָ����һ����ʱ��
  }
  //��i����ʱ����ʱ
  timerctl.using -= i;
  //����λ
  timerctl.t0 = timer;
  //timer next �趨
  if (timerctl.using > 0) {
    timerctl.next = timerctl.t0->timeout;
  } else {
    timerctl.next = 0xffffffff;
  }
  return;
}

//ʱ���������
void timer_adjust(void)
{
  int t0 = timerctl.count;
  int i = 0;
  io_cli();
  
  timerctl.count -= t0;
  for (i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timers0[i].flags == TIMER_FLAGS_USING) {
      timerctl.timers0[i].timeout -= t0;
    }
  }
  io_sti();
}
