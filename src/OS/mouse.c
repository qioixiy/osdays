#include "mouse.h"
#include "keyboard.h"

//鼠标设置
#define KEYCMD_SENDTO_MOUSE 0Xd4
#define MOUSECMD_ENABLE 0Xf4

void enable_mouse(struct MOUSE_DEC *mdec)
{
  //激活鼠标
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

  mdec->phase = 0;//正常情况下键盘控制器会返回0xfa
  return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data)
{
  int res = 0;

  if (0 == mdec->phase ) {
    //等待鼠标的0xfa状态
    if (data == 0xfa) {
      mdec->phase = 1;
    }
    res = 0;
  }else if(1 == mdec->phase){
    //等待鼠标的第一个字节
    if (0x08 == (data & 0xC8)) {//第一个字节正确？
      mdec->buf[0] = data;
      mdec->phase = 2;
    }
    res = 0;
  } else if (2 == mdec->phase) {
    //等待鼠标的第2个字节
    mdec->buf[1] = data;
    mdec->phase = 3;
    res = 0;
  } else if (mdec->phase == 3) {
    //等待鼠标的第3个字节
    mdec->buf[2] = data;
    mdec->phase = 1;

    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];
    
    if ((mdec->buf[0] & 0x10) != 0) {
      mdec->x |= 0xffffff00;
    }
    if ((mdec->buf[0] & 0x20) != 0) {
      mdec->y |= 0xffffff00;
    }

    mdec->y = - mdec->y;//鼠标的Y方向与画面符号相反
    
    res = 1;
  }
  
  return res;
}
