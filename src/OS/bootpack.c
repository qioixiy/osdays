void io_hlt(void);//��ͣϵͳ
void io_cli(void);//�ر��ж�
void io_out8(int port, int data);
int  io_load_eflags(void);//���ر�־λ
void io_store_eflags(int eflags);//��ԭ��־λ

void init_palette(void);//�趨��ɫ��
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
  int i;
  char *p;
  
  init_palette();
  p = (char *)0xa0000;//ָ����ַ
  for (i = 0; i <= 0xffff; i++) {
    *(p+i) = i & 0x0f;
  }

  for (;;) {
    io_hlt();
  }
}

void init_palette(void)
{
  static unsigned char table_rgb[16*3]= {
    0x00, 0x00, 0x00,//0:��ɫ
    0xff, 0x00, 0x00,//1:����ɫ
    0x00, 0xff, 0x00,//2:����ɫ
    0xff, 0xff, 0x00,//3:����ɫ
    0x00, 0x00, 0xff,//4:����ɫ
    0xff, 0x00, 0xff,//5:����ɫ
    0x00, 0xff, 0xff,//6:ǳ��ɫ
    0xff, 0xff, 0xff,//7:��ɫ
    0xc6, 0xc6, 0xc6,//8:����ɫ
    0x84, 0x00, 0x00,//9:����ɫ
    0x00, 0x84, 0x00,//10:����ɫ
    0x84, 0x84, 0x00,//11:����ɫ
    0x00, 0x00, 0x84,//12:����ɫ
    0x84, 0x00, 0x84,//13:����ɫ
    0x00, 0x84, 0x84,//14:ǳ��ɫ
    0x84, 0x84, 0x84,//15:����ɫ
  };
  set_palette(0, 15, table_rgb);
  return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
  int i, eflags;
  eflags = io_load_eflags();//��¼�ж���ɱ�־��ֵ
  io_cli();//���ж���ɱ�־��Ϊ0�� ��ֹ�ж�
  io_out8(0x03c8, start);
  
  for ( i = start; i <= end; i ++ ) {
    io_out8(0x03c9,rgb[0] / 4);
    io_out8(0x03c9,rgb[1] / 4);
    io_out8(0x03c9,rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);//��ԭ�ж���ɱ�־
}
