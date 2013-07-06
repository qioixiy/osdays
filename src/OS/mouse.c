#include "mouse.h"
#include "keyboard.h"
#include "naskfunc.h"
#include "int.h"

//�������
#define KEYCMD_SENDTO_MOUSE 0Xd4
#define MOUSECMD_ENABLE 0Xf4

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
  mousefifo = fifo;
  mousedata0 = data0;
  
  //�������
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

  mdec->phase = 0;//��������¼��̿������᷵��0xfa
  return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned int data)
{
  int res = 0;

  if (0 == mdec->phase ) {
    //�ȴ�����0xfa״̬
    if (data == 0xfa) {
      mdec->phase = 1;
    }
    res = 0;
  }else if(1 == mdec->phase){
    //�ȴ����ĵ�һ���ֽ�
    if (0x08 == (data & 0xC8)) {//��һ���ֽ���ȷ��
      mdec->buf[0] = data;
      mdec->phase = 2;
    }
    res = 0;
  } else if (2 == mdec->phase) {
    //�ȴ����ĵ�2���ֽ�
    mdec->buf[1] = data;
    mdec->phase = 3;
    res = 0;
  } else if (mdec->phase == 3) {
    //�ȴ����ĵ�3���ֽ�
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

    mdec->y = - mdec->y;//����Y�����뻭������෴
    
    res = 1;
  }
  
  return res;
}
