#ifndef _MOUSE_H
#define _MOUSE_H
 
//����豸�����ṹ
struct MOUSE_DEC {
  unsigned char buf[3];//����3�ֽ�buffer
  unsigned char phase;//�����յ�״̬

  int x, y;//x,y����
  int btn;//����״̬
};
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

#endif
