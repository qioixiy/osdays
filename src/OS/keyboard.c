#include "keyboard.h"
#include "naskfunc.h"
#include "int.h"

void wait_KBC_sendready(void)
{
  //等待键盘控制电路准备完毕
  for (;;) {
    if (( io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
      break;
    }
  }
  return;
}

void init_keyboard(struct FIFO32 *fifo, int data0)
{
  keyfifo = fifo;
  keydata0 = data0;
  
  //初始化键盘控制电路
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
  
  return;
}
