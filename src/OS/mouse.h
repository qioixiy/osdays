#ifndef _MOUSE_H
#define _MOUSE_H
 
//鼠标设备描述结构
struct MOUSE_DEC {
  unsigned char buf[3];//鼠标的3字节buffer
  unsigned char phase;//鼠标接收的状态

  int x, y;//x,y方向
  int btn;//按键状态
};
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

#endif
