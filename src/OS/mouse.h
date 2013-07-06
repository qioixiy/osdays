#ifndef _MOUSE_H
#define _MOUSE_H
 #include "fifo.h"
 
//����豸�����ṹ
struct MOUSE_DEC {
  unsigned char buf[3];//����3�ֽ�buffer
  unsigned char phase;//�����յ�״̬

  int x, y;//x,y����
  int btn;//����״̬
};
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned int data);

#endif
