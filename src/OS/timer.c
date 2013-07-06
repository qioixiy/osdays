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
    timerctl.timer[i].flags = TIMER_FLAGS_NOUSE;//δʹ��
  }
  return;
}

struct TIMER *timer_alloc(void)
{
  int i;
  for(i = 0; i < MAX_TIMER; i++){
    if (timerctl.timer[i].flags == TIMER_FLAGS_NOUSE) {
      timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
      return &timerctl.timer[i];
    }
  }
  return 0;
}

void timer_free(struct TIMER *timer)
{
  timer->flags = TIMER_FLAGS_NOUSE;
}

void timer_init(struct TIMER *timer,struct FIFO8 *fifo, unsigned char data)
{
  timer->fifo = fifo;
  timer->data = data;
  return ;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  if (timerctl.next > timer->timeout) {
    timerctl.next = timer->timeout;
  }
  return;
}

void inthandler20(int *esp)
{
  int i;
  io_out8(PIC0_OCW2, 0X60);//��IRQ-00�źŽ������˵���Ϣ֪ͨ��PIC
  timerctl.count++;
  if (timerctl.count < timerctl.next) {
    return ;//��������һʱ��
  }
  
  timerctl.next = 0xffffffff;
  for(i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timer[i].flags == TIMER_FLAGS_USING ){//����趨�˳�ʱ
      if (timerctl.timer[i].timeout <= timerctl.count) {//��ʱ�Ѿ���
	timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
	fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
      } else {
	//û�г�ʱ����
	if (timerctl.next > timerctl.timer[i].timeout) {
	  timerctl.next = timerctl.timer[i].timeout;
	}
      }
    }
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
    if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {
      timerctl.timer[i].timeout -= t0;
    }
  }
  io_sti();
}